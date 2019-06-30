/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2018                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Bastian Schneider <b.schneider@badnoob.com>                 |
  | Borrowed code from php-src                                           |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_STREAMS_XP_SOCKET
#define HAVE_PTHREADS_STREAMS_XP_SOCKET

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_NETWORK_H
#	include <src/network.h> // defines php_netstream_data_t
#endif

#ifndef FILE_H
#	include <ext/standard/file.h>
#endif

#ifdef PHP_WIN32
/* send/recv family on windows expects int */
# define PTHREADS_XP_SOCK_BUF_SIZE(sz) (((sz) > INT_MAX) ? INT_MAX : (int)(sz))
#else
# define PTHREADS_XP_SOCK_BUF_SIZE(sz) (sz)
#endif

const pthreads_stream_ops pthreads_stream_generic_socket_ops;
const pthreads_stream_ops pthreads_stream_socket_ops;
const pthreads_stream_ops pthreads_stream_udp_socket_ops;
#ifdef AF_UNIX
const pthreads_stream_ops pthreads_stream_unix_socket_ops;
const pthreads_stream_ops pthreads_stream_unixdg_socket_ops;
#endif

static int pthreads_tcp_sockop_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam);

/* {{{ Generic socket stream operations */
static size_t pthreads_sockop_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
	int didwrite;
	struct timeval *ptimeout;

	if (!sock || sock->socket == -1) {
		return 0;
	}

	if (sock->timeout.tv_sec == -1)
		ptimeout = NULL;
	else
		ptimeout = &sock->timeout;

retry:
	didwrite = send(sock->socket, buf, PTHREADS_XP_SOCK_BUF_SIZE(count), (sock->is_blocked && ptimeout) ? MSG_DONTWAIT : 0);

	if (didwrite <= 0) {
		int err = php_socket_errno();
		char *estr;

		if (sock->is_blocked && (err == EWOULDBLOCK || err == EAGAIN)) {
			int retval;

			sock->timeout_event = 0;

			do {
				retval = php_pollfd_for(sock->socket, POLLOUT, ptimeout);

				if (retval == 0) {
					sock->timeout_event = 1;
					break;
				}

				if (retval > 0) {
					/* writable now; retry */
					goto retry;
				}

				err = php_socket_errno();
			} while (err == EINTR);
		}
		estr = php_socket_strerror(err, NULL, 0);
		php_error_docref(NULL, E_NOTICE, "send of " ZEND_LONG_FMT " bytes failed with errno=%d %s",
				(zend_long)count, err, estr);
		efree(estr);
	}

	if (didwrite > 0) {
		pthreads_stream_notify_progress_increment(pthreads_stream_get_context(threaded_stream), didwrite, 0);
	}

	if (didwrite < 0) {
		didwrite = 0;
	}

	return didwrite;
}

static void pthreads_sock_stream_wait_for_data(pthreads_stream_t *threaded_stream, pthreads_netstream_data_t *sock)
{
	int retval;
	struct timeval *ptimeout;

	if (!sock || sock->socket == -1) {
		return;
	}

	sock->timeout_event = 0;

	if (sock->timeout.tv_sec == -1)
		ptimeout = NULL;
	else
		ptimeout = &sock->timeout;

	while(1) {
		retval = php_pollfd_for(sock->socket, PHP_POLLREADABLE, ptimeout);

		if (retval == 0)
			sock->timeout_event = 1;

		if (retval >= 0)
			break;

		if (php_socket_errno() != EINTR)
			break;
	}
}

static size_t pthreads_sockop_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
	ssize_t nr_bytes = 0;
	int err;

	if (!sock || sock->socket == -1) {
		return 0;
	}

	if (sock->is_blocked) {
		pthreads_sock_stream_wait_for_data(threaded_stream, sock);
		if (sock->timeout_event)
			return 0;
	}

	nr_bytes = recv(sock->socket, buf, PTHREADS_XP_SOCK_BUF_SIZE(count), (sock->is_blocked && sock->timeout.tv_sec != -1) ? MSG_DONTWAIT : 0);
	err = php_socket_errno();

	stream->eof = (nr_bytes == 0 || (nr_bytes == -1 && err != EWOULDBLOCK && err != EAGAIN));

	if (nr_bytes > 0) {
		pthreads_stream_notify_progress_increment(pthreads_stream_get_context(threaded_stream), nr_bytes, 0);
	}

	if (nr_bytes < 0) {
		nr_bytes = 0;
	}

	return nr_bytes;
}


static int pthreads_sockop_close(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
#ifdef PHP_WIN32
	int n;
#endif

	if (!sock) {
		return 0;
	}

	if (close_handle) {

#ifdef PHP_WIN32
		if (sock->socket == -1)
			sock->socket = SOCK_ERR;
#endif
		if (sock->socket != SOCK_ERR) {
#ifdef PHP_WIN32
			/* prevent more data from coming in */
			shutdown(sock->socket, PTHREADS_SHUT_RD);

			/* try to make sure that the OS sends all data before we close the connection.
			 * Essentially, we are waiting for the socket to become writeable, which means
			 * that all pending data has been sent.
			 * We use a small timeout which should encourage the OS to send the data,
			 * but at the same time avoid hanging indefinitely.
			 * */
			do {
				n = php_pollfd_for_ms(sock->socket, POLLOUT, 500);
			} while (n == -1 && php_socket_errno() == EINTR);
#endif
			closesocket(sock->socket);
			sock->socket = SOCK_ERR;
		}
	}
	return 0;
}

static void pthreads_sockop_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;

	if (!sock) {
		return;
	}
	free(sock);
}

static int pthreads_sockop_flush(pthreads_stream_t *threaded_stream) {
#if 0
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
	return fsync(sock->socket);
#endif
	return 0;
}

static int pthreads_sockop_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb)
{
#if ZEND_WIN32
	return 0;
#else
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;

	return zend_fstat(sock->socket, &ssb->sb);
#endif
}

static inline int sock_sendto(pthreads_netstream_data_t *sock, const char *buf, size_t buflen, int flags,
		struct sockaddr *addr, socklen_t addrlen
		)
{
	int ret;
	if (addr) {
		ret = sendto(sock->socket, buf, PTHREADS_XP_SOCK_BUF_SIZE(buflen), flags, addr, PTHREADS_XP_SOCK_BUF_SIZE(addrlen));

		return (ret == SOCK_CONN_ERR) ? -1 : ret;
	}
#ifdef PHP_WIN32
	return ((ret = send(sock->socket, buf, buflen > INT_MAX ? INT_MAX : (int)buflen, flags)) == SOCK_CONN_ERR) ? -1 : ret;
#else
	return ((ret = send(sock->socket, buf, buflen, flags)) == SOCK_CONN_ERR) ? -1 : ret;
#endif
}

static inline int sock_recvfrom(pthreads_netstream_data_t *sock, char *buf, size_t buflen, int flags,
		zend_string **textaddr,
		struct sockaddr **addr, socklen_t *addrlen
		)
{
	int ret;
	int want_addr = textaddr || addr;

	if (want_addr) {
		php_sockaddr_storage sa;
		socklen_t sl = sizeof(sa);
		ret = recvfrom(sock->socket, buf, PTHREADS_XP_SOCK_BUF_SIZE(buflen), flags, (struct sockaddr*)&sa, &sl);
		ret = (ret == SOCK_CONN_ERR) ? -1 : ret;
		if (sl) {
			php_network_populate_name_from_sockaddr((struct sockaddr*)&sa, sl,
					textaddr, addr, addrlen);
		} else {
			if (textaddr) {
				*textaddr = ZSTR_EMPTY_ALLOC();
			}
			if (addr) {
				*addr = NULL;
				*addrlen = 0;
			}
		}
	} else {
		ret = recv(sock->socket, buf, PTHREADS_XP_SOCK_BUF_SIZE(buflen), flags);
		ret = (ret == SOCK_CONN_ERR) ? -1 : ret;
	}

	return ret;
}

static int pthreads_sockop_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam){
	int oldmode, flags;
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
	pthreads_stream_xport_param *xparam;

	if (!sock) {
		return PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
	}

	switch(option) {
		case PTHREADS_STREAM_OPTION_CHECK_LIVENESS:
			{
				struct timeval tv;
				char buf;
				int alive = 1;

				if (value == -1) {
					if (sock->timeout.tv_sec == -1) {
						tv.tv_sec = FG(default_socket_timeout);
						tv.tv_usec = 0;
					} else {
						tv = sock->timeout;
					}
				} else {
					tv.tv_sec = value;
					tv.tv_usec = 0;
				}

				if (sock->socket == -1) {
					alive = 0;
				} else if (php_pollfd_for(sock->socket, PHP_POLLREADABLE|POLLPRI, &tv) > 0) {
#ifdef PHP_WIN32
					int ret;
#else
					ssize_t ret;
#endif
					int err;

					ret = recv(sock->socket, &buf, sizeof(buf), MSG_PEEK);
					err = php_socket_errno();

					if (0 == ret || /* the counterpart did properly shutdown*/
						(0 > ret && err != EWOULDBLOCK && err != EAGAIN && err != EMSGSIZE)) { /* there was an unrecoverable error */
						alive = 0;
					}
				}
				return alive ? PTHREADS_STREAM_OPTION_RETURN_OK : PTHREADS_STREAM_OPTION_RETURN_ERR;
			}

		case PTHREADS_STREAM_OPTION_BLOCKING:
			oldmode = sock->is_blocked;
			if (SUCCESS == php_set_sock_blocking(sock->socket, value)) {
				sock->is_blocked = value;
				return oldmode;
			}
			return PTHREADS_STREAM_OPTION_RETURN_ERR;

		case PTHREADS_STREAM_OPTION_READ_TIMEOUT:
			sock->timeout = *(struct timeval*)ptrparam;
			sock->timeout_event = 0;
			return PTHREADS_STREAM_OPTION_RETURN_OK;

		case PTHREADS_STREAM_OPTION_META_DATA_API:
			add_assoc_bool((zval *)ptrparam, "timed_out", sock->timeout_event);
			add_assoc_bool((zval *)ptrparam, "blocked", sock->is_blocked);
			add_assoc_bool((zval *)ptrparam, "eof", stream->eof);
			return PTHREADS_STREAM_OPTION_RETURN_OK;

		case PTHREADS_STREAM_OPTION_XPORT_API:
			xparam = (pthreads_stream_xport_param *)ptrparam;

			switch (xparam->op) {
				case PTHREADS_STREAM_XPORT_OP_LISTEN:
					xparam->outputs.returncode = (listen(sock->socket, xparam->inputs.backlog) == 0) ?  0: -1;
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_GET_NAME:
					xparam->outputs.returncode = php_network_get_sock_name(sock->socket,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_GET_PEER_NAME:
					xparam->outputs.returncode = php_network_get_peer_name(sock->socket,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_SEND:
					flags = 0;
					if ((xparam->inputs.flags & PTHREADS_STREAM_OOB) == PTHREADS_STREAM_OOB) {
						flags |= MSG_OOB;
					}
					xparam->outputs.returncode = sock_sendto(sock,
							xparam->inputs.buf, xparam->inputs.buflen,
							flags,
							xparam->inputs.addr,
							xparam->inputs.addrlen);
					if (xparam->outputs.returncode == -1) {
						char *err = php_socket_strerror(php_socket_errno(), NULL, 0);
						php_error_docref(NULL, E_WARNING,
						   	"%s\n", err);
						efree(err);
					}
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_RECV:
					flags = 0;
					if ((xparam->inputs.flags & PTHREADS_STREAM_OOB) == PTHREADS_STREAM_OOB) {
						flags |= MSG_OOB;
					}
					if ((xparam->inputs.flags & PTHREADS_STREAM_PEEK) == PTHREADS_STREAM_PEEK) {
						flags |= MSG_PEEK;
					}
					xparam->outputs.returncode = sock_recvfrom(sock,
							xparam->inputs.buf, xparam->inputs.buflen,
							flags,
							xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
							xparam->want_addr ? &xparam->outputs.addr : NULL,
							xparam->want_addr ? &xparam->outputs.addrlen : NULL
							);
					return PTHREADS_STREAM_OPTION_RETURN_OK;


#ifdef HAVE_SHUTDOWN
# ifndef SHUT_RD
#  define SHUT_RD 0
# endif
# ifndef SHUT_WR
#  define SHUT_WR 1
# endif
# ifndef SHUT_RDWR
#  define SHUT_RDWR 2
# endif
				case PTHREADS_STREAM_XPORT_OP_SHUTDOWN: {
					static const int shutdown_how[] = {SHUT_RD, SHUT_WR, SHUT_RDWR};

					xparam->outputs.returncode = shutdown(sock->socket, shutdown_how[xparam->how]);
					return PTHREADS_STREAM_OPTION_RETURN_OK;
				}
#endif

				default:
					return PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
			}

		default:
			return PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
	}
}

static int pthreads_sockop_cast(pthreads_stream_t *threaded_stream, int castas, void **ret) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;

	if (!sock) {
		return FAILURE;
	}

	switch(castas)	{
		case PTHREADS_STREAM_AS_STDIO:
			if (ret)	{
				*(FILE**)ret = fdopen(sock->socket, stream->mode);
				if (*ret)
					return SUCCESS;
				return FAILURE;
			}
			return SUCCESS;
		case PTHREADS_STREAM_AS_FD_FOR_SELECT:
		case PTHREADS_STREAM_AS_FD:
		case PTHREADS_STREAM_AS_SOCKETD:
			if (ret)
				*(php_socket_t *)ret = sock->socket;
			return SUCCESS;
		default:
			return FAILURE;
	}
}
/* }}} */

/* These may look identical, but we need them this way so that
 * we can determine which type of socket we are dealing with
 * by inspecting stream->ops.
 * A "useful" side-effect is that the user's scripts can then
 * make similar decisions using stream_get_meta_data.
 * */
const pthreads_stream_ops pthreads_stream_generic_socket_ops = {
	pthreads_sockop_write, pthreads_sockop_read,
	pthreads_sockop_close, pthreads_sockop_free,
	pthreads_sockop_flush,
	"generic_socket",
	NULL, /* seek */
	pthreads_sockop_cast,
	pthreads_sockop_stat,
	pthreads_sockop_set_option,
};

const pthreads_stream_ops pthreads_stream_socket_ops = {
	pthreads_sockop_write, pthreads_sockop_read,
	pthreads_sockop_close, pthreads_sockop_free,
	pthreads_sockop_flush,
	"tcp_socket",
	NULL, /* seek */
	pthreads_sockop_cast,
	pthreads_sockop_stat,
	pthreads_tcp_sockop_set_option,
};

const pthreads_stream_ops pthreads_stream_udp_socket_ops = {
	pthreads_sockop_write, pthreads_sockop_read,
	pthreads_sockop_close, pthreads_sockop_free,
	pthreads_sockop_flush,
	"udp_socket",
	NULL, /* seek */
	pthreads_sockop_cast,
	pthreads_sockop_stat,
	pthreads_tcp_sockop_set_option,
};

#ifdef AF_UNIX
const pthreads_stream_ops pthreads_stream_unix_socket_ops = {
	pthreads_sockop_write, pthreads_sockop_read,
	pthreads_sockop_close, pthreads_sockop_free,
	pthreads_sockop_flush,
	"unix_socket",
	NULL, /* seek */
	pthreads_sockop_cast,
	pthreads_sockop_stat,
	pthreads_tcp_sockop_set_option,
};

const pthreads_stream_ops pthreads_stream_unixdg_socket_ops = {
	pthreads_sockop_write, pthreads_sockop_read,
	pthreads_sockop_close, pthreads_sockop_free,
	pthreads_sockop_flush,
	"udg_socket",
	NULL, /* seek */
	pthreads_sockop_cast,
	pthreads_sockop_stat,
	pthreads_tcp_sockop_set_option,
};
#endif


/* network socket operations */

#ifdef AF_UNIX
static inline int parse_unix_address(pthreads_stream_xport_param *xparam, struct sockaddr_un *unix_addr)
{
	memset(unix_addr, 0, sizeof(*unix_addr));
	unix_addr->sun_family = AF_UNIX;

	/* we need to be binary safe on systems that support an abstract
	 * namespace */
	if (xparam->inputs.namelen >= sizeof(unix_addr->sun_path)) {
		/* On linux, when the path begins with a NUL byte we are
		 * referring to an abstract namespace.  In theory we should
		 * allow an extra byte below, since we don't need the NULL.
		 * BUT, to get into this branch of code, the name is too long,
		 * so we don't care. */
		xparam->inputs.namelen = sizeof(unix_addr->sun_path) - 1;
		php_error_docref(NULL, E_NOTICE,
			"socket path exceeded the maximum allowed length of %lu bytes "
			"and was truncated", (unsigned long)sizeof(unix_addr->sun_path));
	}

	memcpy(unix_addr->sun_path, xparam->inputs.name, xparam->inputs.namelen);

	return 1;
}
#endif

static inline char *parse_ip_address_ex(const char *str, size_t str_len, int *portno, int get_err, zend_string **err)
{
	char *colon;
	char *host = NULL;

#ifdef HAVE_IPV6
	char *p;

	if (*(str) == '[' && str_len > 1) {
		/* IPV6 notation to specify raw address with port (i.e. [fe80::1]:80) */
		p = memchr(str + 1, ']', str_len - 2);
		if (!p || *(p + 1) != ':') {
			if (get_err) {
				*err = strpprintf(0, "Failed to parse IPv6 address \"%s\"", str);
			}
			return NULL;
		}
		*portno = atoi(p + 2);
		return estrndup(str + 1, p - str - 1);
	}
#endif
	if (str_len) {
		colon = memchr(str, ':', str_len - 1);
	} else {
		colon = NULL;
	}
	if (colon) {
		*portno = atoi(colon + 1);
		host = estrndup(str, colon - str);
	} else {
		if (get_err) {
			*err = strpprintf(0, "Failed to parse address \"%s\"", str);
		}
		return NULL;
	}

	return host;
}

static inline char *parse_ip_address(pthreads_stream_xport_param *xparam, int *portno) {
	return parse_ip_address_ex(xparam->inputs.name, xparam->inputs.namelen, portno, xparam->want_errortext, &xparam->outputs.error_text);
}

static inline int pthreads_tcp_sockop_bind(pthreads_stream_t *threaded_stream, pthreads_netstream_data_t *sock, pthreads_stream_xport_param *xparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	char *host = NULL;
	int portno, err;
	long sockopts = PTHREADS_STREAM_SOCKOP_NONE;
	zval *tmpzval = NULL;

#ifdef AF_UNIX
	if (stream->ops == &pthreads_stream_unix_socket_ops || stream->ops == &pthreads_stream_unixdg_socket_ops) {
		struct sockaddr_un unix_addr;

		sock->socket = socket(PF_UNIX, stream->ops == &pthreads_stream_unix_socket_ops ? SOCK_STREAM : SOCK_DGRAM, 0);

		if (sock->socket == SOCK_ERR) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "Failed to create unix%s socket %s",
						stream->ops == &pthreads_stream_unix_socket_ops ? "" : "datagram",
						strerror(errno));
			}
			return -1;
		}

		parse_unix_address(xparam, &unix_addr);

		return bind(sock->socket, (const struct sockaddr *)&unix_addr,
			(socklen_t) XtOffsetOf(struct sockaddr_un, sun_path) + xparam->inputs.namelen);
	}
#endif

	host = parse_ip_address(xparam, &portno);

	if (host == NULL) {
		return -1;
	}

#ifdef IPV6_V6ONLY
	if (pthreads_stream_get_context(threaded_stream)
		&& (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "ipv6_v6only")) != NULL
		&& Z_TYPE_P(tmpzval) != IS_NULL
	) {
		sockopts |= STREAM_SOCKOP_IPV6_V6ONLY;
		sockopts |= STREAM_SOCKOP_IPV6_V6ONLY_ENABLED * zend_is_true(tmpzval);
	}
#endif

#ifdef SO_REUSEPORT
	if (pthreads_stream_get_context(threaded_stream)
		&& (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "so_reuseport")) != NULL
		&& zend_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_REUSEPORT;
	}
#endif

#ifdef SO_BROADCAST
	if (stream->ops == &pthreads_stream_udp_socket_ops /* SO_BROADCAST is only applicable for UDP */
		&& pthreads_stream_get_context(threaded_stream)
		&& (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "so_broadcast")) != NULL
		&& zend_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_BROADCAST;
	}
#endif

	sock->socket = php_network_bind_socket_to_local_addr(host, portno,
			stream->ops == &pthreads_stream_udp_socket_ops ? SOCK_DGRAM : SOCK_STREAM,
			sockopts,
			xparam->want_errortext ? &xparam->outputs.error_text : NULL,
			&err
			);

	if (host) {
		efree(host);
	}

	return sock->socket == -1 ? -1 : 0;
}

static inline int pthreads_tcp_sockop_connect(pthreads_stream_t *threaded_stream, pthreads_netstream_data_t *sock, pthreads_stream_xport_param *xparam)
{
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	char *host = NULL, *bindto = NULL;
	int portno, bindport = 0;
	int err = 0;
	int ret;
	zval *tmpzval = NULL;
	long sockopts = STREAM_SOCKOP_NONE;

#ifdef AF_UNIX
	if (stream->ops == &pthreads_stream_unix_socket_ops || stream->ops == &pthreads_stream_unixdg_socket_ops) {
		struct sockaddr_un unix_addr;

		sock->socket = socket(PF_UNIX, stream->ops == &pthreads_stream_unix_socket_ops ? SOCK_STREAM : SOCK_DGRAM, 0);

		if (sock->socket == SOCK_ERR) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "Failed to create unix socket");
			}
			return -1;
		}

		parse_unix_address(xparam, &unix_addr);

		ret = php_network_connect_socket(sock->socket,
				(const struct sockaddr *)&unix_addr, (socklen_t) XtOffsetOf(struct sockaddr_un, sun_path) + xparam->inputs.namelen,
				xparam->op == PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC, xparam->inputs.timeout,
				xparam->want_errortext ? &xparam->outputs.error_text : NULL,
				&err);

		xparam->outputs.error_code = err;

		goto out;
	}
#endif

	host = parse_ip_address(xparam, &portno);

	if (host == NULL) {
		return -1;
	}

	if (pthreads_stream_get_context(threaded_stream) && (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "bindto")) != NULL) {
		if (Z_TYPE_P(tmpzval) != IS_STRING) {
			if (xparam->want_errortext) {
				xparam->outputs.error_text = strpprintf(0, "local_addr context option is not a string.");
			}
			efree(host);
			return -1;
		}
		bindto = parse_ip_address_ex(Z_STRVAL_P(tmpzval), Z_STRLEN_P(tmpzval), &bindport, xparam->want_errortext, &xparam->outputs.error_text);
	}

#ifdef SO_BROADCAST
	if (stream->ops == &pthreads_stream_udp_socket_ops /* SO_BROADCAST is only applicable for UDP */
		&& pthreads_stream_get_context(threaded_stream)
		&& (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "so_broadcast")) != NULL
		&& zend_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_SO_BROADCAST;
	}
#endif

	if (stream->ops != &pthreads_stream_udp_socket_ops /* TCP_NODELAY is only applicable for TCP */
#ifdef AF_UNIX
		&& stream->ops != &pthreads_stream_unix_socket_ops
		&& stream->ops != &pthreads_stream_unixdg_socket_ops
#endif
		&& pthreads_stream_get_context(threaded_stream)
		&& (tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "tcp_nodelay")) != NULL
		&& zend_is_true(tmpzval)
	) {
		sockopts |= STREAM_SOCKOP_TCP_NODELAY;
	}

	/* Note: the test here for pthreads_stream_udp_socket_ops is important, because we
	 * want the default to be TCP sockets so that the openssl extension can
	 * re-use this code. */

	sock->socket = php_network_connect_socket_to_host(host, portno,
			stream->ops == &pthreads_stream_udp_socket_ops ? SOCK_DGRAM : SOCK_STREAM,
			xparam->op == PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC,
			xparam->inputs.timeout,
			xparam->want_errortext ? &xparam->outputs.error_text : NULL,
			&err,
			bindto,
			bindport,
			sockopts
			);

	ret = sock->socket == -1 ? -1 : 0;
	xparam->outputs.error_code = err;

	if (host) {
		efree(host);
	}
	if (bindto) {
		efree(bindto);
	}

#ifdef AF_UNIX
out:
#endif

	if (ret >= 0 && xparam->op == PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC && err == EINPROGRESS) {
		/* indicates pending connection */
		return 1;
	}

	return ret;
}

static inline int pthreads_tcp_sockop_accept(pthreads_stream_t *threaded_stream, pthreads_netstream_data_t *sock, pthreads_stream_xport_param *xparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream *client_stream = NULL;
	int clisock;
	zend_bool nodelay = 0;
	zval *tmpzval = NULL;

	xparam->outputs.client = NULL;

	if ((NULL != pthreads_stream_get_context(threaded_stream)) &&
		(tmpzval = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), "socket", "tcp_nodelay")) != NULL &&
		zend_is_true(tmpzval)) {
		nodelay = 1;
	}

	clisock = php_network_accept_incoming(sock->socket,
		xparam->want_textaddr ? &xparam->outputs.textaddr : NULL,
		xparam->want_addr ? &xparam->outputs.addr : NULL,
		xparam->want_addr ? &xparam->outputs.addrlen : NULL,
		xparam->inputs.timeout,
		xparam->want_errortext ? &xparam->outputs.error_text : NULL,
		&xparam->outputs.error_code,
		nodelay);

	if (clisock >= 0) {
		pthreads_netstream_data_t *clisockdata = (pthreads_netstream_data_t*) malloc(sizeof(*clisockdata));

		memcpy(clisockdata, sock, sizeof(*clisockdata));
		clisockdata->socket = clisock;

		xparam->outputs.client = PTHREADS_STREAM_NEW(stream->ops, clisockdata, "r+");
		if (xparam->outputs.client) {
			pthreads_stream_set_context(xparam->outputs.client, pthreads_stream_get_context(threaded_stream));
		}
	}

	return xparam->outputs.client == NULL ? -1 : 0;
}

static int pthreads_tcp_sockop_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_netstream_data_t *sock = (pthreads_netstream_data_t*)stream->abstract;
	pthreads_stream_xport_param *xparam;

	switch(option) {
		case PTHREADS_STREAM_OPTION_XPORT_API:
			xparam = (pthreads_stream_xport_param *)ptrparam;

			switch(xparam->op) {
				case PTHREADS_STREAM_XPORT_OP_CONNECT:
				case PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC:
					xparam->outputs.returncode = pthreads_tcp_sockop_connect(threaded_stream, sock, xparam);
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_BIND:
					xparam->outputs.returncode = pthreads_tcp_sockop_bind(threaded_stream, sock, xparam);
					return PTHREADS_STREAM_OPTION_RETURN_OK;

				case PTHREADS_STREAM_XPORT_OP_ACCEPT:
					xparam->outputs.returncode = pthreads_tcp_sockop_accept(threaded_stream, sock, xparam);
					return PTHREADS_STREAM_OPTION_RETURN_OK;
				default:
					/* fall through */
					;
			}
	}
	return pthreads_sockop_set_option(threaded_stream, option, value, ptrparam);
}

pthreads_stream_t *pthreads_stream_generic_socket_factory(const char *proto, size_t protolen,
		const char *resourcename, size_t resourcenamelen,
		int options, int flags, struct timeval *timeout,
		pthreads_stream_context_t *threaded_context)
{
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_netstream_data_t *sock;
	const pthreads_stream_ops *ops;

	/* which type of socket ? */
	if (strncmp(proto, "tcp", protolen) == 0) {
		ops = &pthreads_stream_socket_ops;
	} else if (strncmp(proto, "udp", protolen) == 0) {
		ops = &pthreads_stream_udp_socket_ops;
	}
#ifdef AF_UNIX
	else if (strncmp(proto, "unix", protolen) == 0) {
		ops = &pthreads_stream_unix_socket_ops;
	} else if (strncmp(proto, "udg", protolen) == 0) {
		ops = &pthreads_stream_unixdg_socket_ops;
	}
#endif
	else {
		/* should never happen */
		return NULL;
	}

	sock = malloc(sizeof(pthreads_netstream_data_t));
	memset(sock, 0, sizeof(pthreads_netstream_data_t));

	sock->is_blocked = 1;
	sock->timeout.tv_sec = FG(default_socket_timeout);
	sock->timeout.tv_usec = 0;

	/* we don't know the socket until we have determined if we are binding or connecting */
	sock->socket = -1;

	threaded_stream = PTHREADS_STREAM_CLASS_NEW(ops, sock, "r+", pthreads_socket_stream_entry);

	if (threaded_stream == NULL)	{
		free(sock);
		return NULL;
	}

	return threaded_stream;
}

#endif
