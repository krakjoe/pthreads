/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_SOCKET
#define HAVE_PTHREADS_SOCKET

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_SOCKET_H
#	include <src/socket.h>
#endif

#ifdef PHP_WIN32
# include "sockets/windows_common.h"
# define SOCK_EINVAL WSAEINVAL
#else
# include <fcntl.h>
# include <errno.h>
# define set_errno(a) (errno = a)
# define SOCK_EINVAL EINVAL
#endif


#define PTHREADS_IS_VALID_SOCKET(sock) !PTHREADS_IS_INVALID_SOCKET(sock)
#define PTHREADS_CLEAR_SOCKET_ERROR(sock) (sock)->error = SUCCESS

#define PTHREADS_SOCKET_CHECK(sock) do { \
	if (PTHREADS_IS_INVALID_SOCKET(sock)) { \
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, \
			"socket found in invalid state"); \
		return; \
	} \
} while(0)

#define PTHREADS_SOCKET_CHECK_EX(sock, retval) do { \
	if (PTHREADS_IS_INVALID_SOCKET(sock)) { \
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, \
			"socket found in invalid state"); \
		return (retval); \
	} \
} while(0)

#define PTHREADS_HANDLE_SOCKET_ERROR(eno, msg) do { \
	if ((eno) != EAGAIN && (eno) != EWOULDBLOCK && (eno) != EINPROGRESS && (eno) != SOCK_EINVAL) { \
		char *estr = (eno) != SUCCESS ? \
			php_socket_strerror((eno), NULL, 0) : \
			NULL; \
		zend_throw_exception_ex(spl_ce_RuntimeException, (eno), \
			"%s (%d): %s", (msg), (eno), estr ? estr : "unknown"); \
		if ((eno) != SUCCESS) { \
			if (estr) { \
				efree(estr); \
			} \
		} \
	} \
	\
} while(0)

#define PTHREADS_SOCKET_ERROR(socket, msg, eno) do { \
	(socket)->error = (eno); \
	PTHREADS_HANDLE_SOCKET_ERROR(eno, msg); \
} while(0)

pthreads_socket_t* pthreads_socket_alloc(void) {
	return (pthreads_socket_t*) ecalloc(1, sizeof(pthreads_socket_t));
}

void pthreads_socket_construct(zval *object, zend_long domain, zend_long type, zend_long protocol) {
	pthreads_object_t *threaded = 
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	threaded->store.sock->fd = socket(domain, type, protocol);

	if (PTHREADS_IS_VALID_SOCKET(threaded->store.sock)) {
		threaded->store.sock->domain = domain;
		threaded->store.sock->type = type;
		threaded->store.sock->protocol = protocol;
		threaded->store.sock->error = SUCCESS;
		return;
	}

	PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to create socket", errno);
}

void pthreads_socket_set_option(zval *object, zend_long level, zend_long name, zend_long value, zval *return_value) {
	pthreads_object_t *threaded = 
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	
	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (setsockopt(threaded->store.sock->fd, level, name, (char*) &value, sizeof(value)) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to set socket option", errno);

		RETURN_FALSE;
	}

	RETURN_TRUE;
}

void pthreads_socket_get_option(zval *object, zend_long level, zend_long name, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	socklen_t unused = sizeof(zend_long);

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (getsockopt(threaded->store.sock->fd, level, name, (void*) &Z_LVAL_P(return_value), &unused) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);

		RETURN_FALSE;
	} else {
		Z_TYPE_INFO_P(return_value) = IS_LONG;
	}
}

static inline zend_bool pthreads_socket_set_inet_addr(pthreads_socket_t *sock, struct sockaddr_in *sin, zend_string *address) {
	struct in_addr tmp;
	struct hostent *hentry;

	if (inet_aton(ZSTR_VAL(address), &tmp)) {
		sin->sin_addr.s_addr = tmp.s_addr;
	} else {
		if (ZSTR_LEN(address) > MAXFQDNLEN || !(hentry = php_network_gethostbyname(ZSTR_VAL(address)))) {
			PTHREADS_SOCKET_ERROR(sock, "Host lookup failed", errno);

			return 0;
		}

		if (hentry->h_addrtype != AF_INET) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Host lookup failed: Non AF_INET domain returned on AF_INET socket");
			return 0;
		}
		
		memcpy(&(sin->sin_addr.s_addr), hentry->h_addr_list[0], hentry->h_length);	
	}

	return 1;
}

static inline int pthreads_socket_string_to_if_index(const char *val, unsigned *out) {
#if HAVE_IF_NAMETOINDEX
	unsigned int ind = if_nametoindex(val);

	if (ind != 0) {
		*out = ind;
		return SUCCESS;
	}
	
	return FAILURE;
#else
	/* throw */
	return FAILURE;
#endif
}

static inline zend_bool pthreads_socket_set_inet6_addr(pthreads_socket_t *sock, struct sockaddr_in6 *sin, zend_string *address) {
	struct in6_addr tmp;
#if HAVE_GETADDRINFO
	struct addrinfo hints;
	struct addrinfo *addrinfo = NULL;	
#endif
	char *scope = strchr(ZSTR_VAL(address), '%');
	
	if (inet_pton(AF_INET6, ZSTR_VAL(address), &tmp)) {
		memcpy(&(sin->sin6_addr.s6_addr), &(tmp.s6_addr), sizeof(struct in6_addr));
	} else {
#if HAVE_GETADDRINFO
		memset(&hints, 0, sizeof(struct addrinfo));
		
		hints.ai_family = AF_INET6;
#if HAVE_AI_V4MAPPED
		hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
#else
		hints.ai_flags = AI_ADDRCONFIG;
#endif
		getaddrinfo(ZSTR_VAL(address), NULL, &hints, &addrinfo);
		if (!addrinfo) {
			PTHREADS_SOCKET_ERROR(sock, "Host lookup failed", errno);
			return 0;
		}

		if (addrinfo->ai_family != PF_INET6 || addrinfo->ai_addrlen != sizeof(struct sockaddr_in6)) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Host lookup failed: Non AF_INET6 domain returned on AF_INET6 socket");

			freeaddrinfo(addrinfo);
			return 0;
		}

		memcpy(&(sin->sin6_addr.s6_addr), ((struct sockaddr_in6*)(addrinfo->ai_addr))->sin6_addr.s6_addr, sizeof(struct in6_addr));
		freeaddrinfo(addrinfo);
#else
		/* No IPv6 specific hostname resolution is available on this system? */
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Host lookup failed: getaddrinfo() not available on this system");

		return 0;
#endif

		if (scope++) {
			zend_long lval = 0;
			double dval = 0;
			unsigned scope_id = 0;

			if (IS_LONG == is_numeric_string(scope, strlen(scope), &lval, &dval, 0)) {
				if (lval > 0 && lval <= UINT_MAX) {
					scope_id = lval;
				} else {
					pthreads_socket_string_to_if_index(scope, &scope_id);
				}
				sin->sin6_scope_id = scope_id;
			}
		}
	}

	return 1;
}

void pthreads_socket_bind(zval *object, zend_string *address, zend_long port, zval *return_value) {
	pthreads_object_t	*threaded =
			PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	php_sockaddr_storage	sa_storage = {0};
	struct sockaddr		*sock_type = (struct sockaddr*) &sa_storage;
	zend_long		retval = 0;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	switch (threaded->store.sock->domain) {
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un *sa = (struct sockaddr_un *) sock_type;

			sa->sun_family = AF_UNIX;

			if (ZSTR_LEN(address) >= sizeof(sa->sun_path)) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0,
						"Invalid path: too long (maximum size is %d)",
						(int)sizeof(sa->sun_path) - 1);
				return;
			}

			memcpy(&sa->sun_path, ZSTR_VAL(address), ZSTR_LEN(address));

			retval = bind(threaded->store.sock->fd, (struct sockaddr *) sa,
					offsetof(struct sockaddr_un, sun_path) + ZSTR_LEN(address));
		} break;
#endif
		case AF_INET: {
			struct sockaddr_in *sa = (struct sockaddr_in *) sock_type;

			sa->sin_family = AF_INET;
			sa->sin_port = htons((unsigned short) port);
			
			if (!pthreads_socket_set_inet_addr(threaded->store.sock, sa, address)) {
				RETURN_FALSE;
			}

			retval = bind(threaded->store.sock->fd, (struct sockaddr *)sa, sizeof(struct sockaddr_in));
		} break;

#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 *sa = (struct sockaddr_in6 *) sock_type;

			sa->sin6_family = AF_INET6;
			sa->sin6_port = htons((unsigned short) port);
			
			if (!pthreads_socket_set_inet6_addr(threaded->store.sock, sa, address)) {
				RETURN_FALSE;
			}

			retval = bind(threaded->store.sock->fd, (struct sockaddr *)sa, sizeof(struct sockaddr_in6));
		} break;
#endif
	}

	if (retval != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to bind address", errno);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

void pthreads_socket_listen(zval *object, zend_long backlog, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (listen(threaded->store.sock->fd, backlog) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to listen on socket", errno);

		RETURN_FALSE;
	}

	RETURN_TRUE;
}

void pthreads_socket_accept(zval *object, zend_class_entry *ce, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_object_t *accepted;

	php_sockaddr_storage sa;
	socklen_t            sa_len = sizeof(sa);
	zend_bool	     blocking = threaded->store.sock->blocking;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (!instanceof_function(ce, pthreads_socket_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s is not an instance of Socket",
			ZSTR_VAL(ce->name));
		return;
	}
	php_socket_t acceptedFd = accept(threaded->store.sock->fd, (struct sockaddr*) &sa, &sa_len);

	if(acceptedFd < 0) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to accept on socket", errno);

		RETURN_FALSE;
	}
	object_init_ex(return_value, ce);

	accepted = PTHREADS_FETCH_FROM(Z_OBJ_P(return_value));
	accepted->store.sock->fd = acceptedFd;
	accepted->store.sock->blocking = 1;
	accepted->store.sock->domain = ((struct sockaddr*) &sa)->sa_family;
}

void pthreads_socket_connect(zval *object, int argc, zend_string *address, zend_long port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	int retval;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	switch (threaded->store.sock->domain) {
#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 sin6 = {0};

			if (argc != 2) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Socket of type AF_INET6 requires 2 arguments");
				return;
			}

			memset(&sin6, 0, sizeof(struct sockaddr_in6));

			sin6.sin6_family = AF_INET6;
			sin6.sin6_port   = htons((unsigned short int)port);

			if (!pthreads_socket_set_inet6_addr(threaded->store.sock, &sin6, address)) {
				RETURN_FALSE;
			}

			retval = connect(threaded->store.sock->fd, (struct sockaddr *)&sin6, sizeof(struct sockaddr_in6));
		} break;
#endif

		case AF_INET: {
			struct sockaddr_in sin = {0};

			if (argc != 2) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Socket of type AF_INET requires 2 arguments");
				return;
			}

			sin.sin_family = AF_INET;
			sin.sin_port   = htons((unsigned short int)port);
			
			if (!pthreads_socket_set_inet_addr(threaded->store.sock, &sin, address)) {
				RETURN_FALSE;
			}

			retval = connect(threaded->store.sock->fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in));
		} break;

#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un s_un = {0};

			if (ZSTR_LEN(address) >= sizeof(s_un.sun_path)) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Path too long");
				return;
			}

			s_un.sun_family = AF_UNIX;
			memcpy(&s_un.sun_path, ZSTR_VAL(address), ZSTR_LEN(address));

			retval = connect(threaded->store.sock->fd, (struct sockaddr *) &s_un,
					(socklen_t)(XtOffsetOf(struct sockaddr_un, sun_path) + ZSTR_LEN(address)));
		} break;
#endif
		default:
			RETURN_FALSE;
	}

	if (retval != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "unable to connect", errno);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

static int pthreads_normal_read(pthreads_object_t *threaded, void *buf, size_t maxlen, int flags) /* {{{ */
{
	int m = 0;
	size_t n = 0;
	int no_read = 0;
	int nonblock = 0;
	char *t = (char *) buf;

	PTHREADS_SOCKET_CHECK_EX(threaded->store.sock, -1);

#ifndef PHP_WIN32
	m = fcntl(threaded->store.sock->fd, F_GETFL);
	if (m < 0) {
		return m;
	}
	nonblock = (m & O_NONBLOCK);
	m = 0;
#else
	nonblock = !threaded->store.sock->blocking;
#endif
	set_errno(0);

	*t = '\0';
	while (*t != '\n' && *t != '\r' && n < maxlen) {
		if (m > 0) {
			t++;
			n++;
		} else if (m == 0) {
			no_read++;
			if (nonblock && no_read >= 2) {
				return n;
				/* The first pass, m always is 0, so no_read becomes 1
				 * in the first pass. no_read becomes 2 in the second pass,
				 * and if this is nonblocking, we should return.. */
			}

			if (no_read > 200) {
				set_errno(ECONNRESET);
				return -1;
			}
		}

		if (n < maxlen) {
			m = recv(threaded->store.sock->fd, (void *) t, 1, flags);
		}

		if (errno != 0 && errno != ESPIPE && errno != EAGAIN) {
			return -1;
		}

		set_errno(0);
	}

	if (n < maxlen) {
		n++;
		/* The only reasons it makes it to here is
		 * if '\n' or '\r' are encountered. So, increase
		 * the return by 1 to make up for the lack of the
		 * '\n' or '\r' in the count (since read() takes
		 * place at the end of the loop..) */
	}

	return n;
}

void pthreads_socket_read(zval *object, zend_long length, zend_long flags, zend_long type, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *buf;
	int bytes;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (length < 1) {
		RETURN_FALSE;
	}

	buf = zend_string_alloc(length, 0);

	if (type == PTHREADS_NORMAL_READ) {
		bytes = pthreads_normal_read(threaded, ZSTR_VAL(buf), length, flags);
	} else {
		bytes = recv(threaded->store.sock->fd, ZSTR_VAL(buf), length, flags);
	}

	if (bytes == -1) {
		zend_string_free(buf);
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to read from socket", errno);

		RETURN_FALSE;
	} else if (!bytes) {
		zend_string_release(buf);
		RETURN_EMPTY_STRING();
	}
	buf = zend_string_truncate(buf, bytes, 0);

	ZSTR_LEN(buf) = bytes;
	ZSTR_VAL(buf)[ZSTR_LEN(buf)] = 0;

	RETURN_NEW_STR(buf);
}

void pthreads_socket_write(zval *object, zend_string *buf, zend_long length, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);
	int bytes;

	if (!length) {
		length = ZSTR_LEN(buf);
	}

#ifndef PHP_WIN32
	bytes = write(threaded->store.sock->fd, ZSTR_VAL(buf), MIN(length, ZSTR_LEN(buf)));
#else
	bytes = send(threaded->store.sock->fd, ZSTR_VAL(buf), min(length, ZSTR_LEN(buf)), 0);
#endif

	if (bytes < 0) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to write to socket", errno);

		RETURN_FALSE;
	}
	
	ZVAL_LONG(return_value, bytes);
}

void pthreads_socket_send(zval *object, zend_string *buf, zend_long length, zend_long flags, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	int bytes;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	bytes = send(threaded->store.sock->fd, ZSTR_VAL(buf), (ZSTR_LEN(buf) < length ? ZSTR_LEN(buf) : length), flags);

	if (bytes == -1) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to write to socket", errno);

		RETURN_FALSE;
	}

	ZVAL_LONG(return_value, bytes);
}

void pthreads_socket_close(zval *object, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (PTHREADS_CLOSE_SOCKET_INTERNAL(threaded->store.sock) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to close socket", errno);

		RETURN_FALSE;
	}

	threaded->store.sock->fd = PTHREADS_INVALID_SOCKET;
}

void pthreads_socket_set_blocking(zval *object, zend_bool blocking, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (php_set_sock_blocking(threaded->store.sock->fd, blocking) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to set blocking mode", errno);

		RETURN_FALSE;
	}

	threaded->store.sock->blocking = blocking;
	RETURN_TRUE;
}

void pthreads_socket_get_sockaddr(zval *object, zend_long port, struct sockaddr *sa, zval *return_value) {
	array_init(return_value);

	switch (sa->sa_family) {
#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) sa;
			char                 addr6[INET6_ADDRSTRLEN+1];

			inet_ntop(AF_INET6, &sin6->sin6_addr, addr6, INET6_ADDRSTRLEN);

			add_assoc_string(return_value, "host", addr6);
			if (port) {
				add_assoc_long(return_value, "port", htons(sin6->sin6_port));
			}
		} break;
#endif
		case AF_INET: {
			struct sockaddr_in  *sin = (struct sockaddr_in *) sa;

			add_assoc_string(return_value, "host", inet_ntoa(sin->sin_addr));
			if (port) {
				add_assoc_long(return_value, "port", htons(sin->sin_port));
			}
		} break;
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un  *s_un = (struct sockaddr_un *) sa;

			add_assoc_string(return_value, "host", s_un->sun_path);
		} break;
#endif
	}
}

void pthreads_socket_get_peer_name(zval *object, zend_bool port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	php_sockaddr_storage  sa_storage;
    struct sockaddr     *sa = (struct sockaddr *) &sa_storage;
	socklen_t            salen = sizeof(php_sockaddr_storage);

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (getpeername(threaded->store.sock->fd, sa, &salen) < 0) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve peer name", errno);

		RETURN_FALSE;
	}

	pthreads_socket_get_sockaddr(object, port, sa, return_value);
}

void pthreads_socket_get_sock_name(zval *object, zend_bool port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	php_sockaddr_storage  sa_storage;
    struct sockaddr     *sa = (struct sockaddr *) &sa_storage;
	socklen_t            salen = sizeof(php_sockaddr_storage);

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (getsockname(threaded->store.sock->fd, sa, &salen) < 0) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket name", errno);

		RETURN_FALSE;
	}

	pthreads_socket_get_sockaddr(object, port, sa, return_value);
}

static inline int pthreads_sockets_to_fd_set(zval *sockets, fd_set *fds, php_socket_t *max_fd) /* {{{ */
{
	zval		*element;
	int			num = 0;

	if (Z_TYPE_P(sockets) != IS_ARRAY) {
		return 0;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(sockets), element) {
		pthreads_object_t *threaded;

		if (Z_TYPE_P(element) != IS_OBJECT ||
			!instanceof_function(Z_OBJCE_P(element), pthreads_socket_entry)) {
			continue;
		}

		threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(element));

		PTHREADS_SOCKET_CHECK_EX(threaded->store.sock, 0);

		PHP_SAFE_FD_SET(threaded->store.sock->fd, fds);

		if (threaded->store.sock->fd > *max_fd) {
			*max_fd = threaded->store.sock->fd;
		}
		num++;
	} ZEND_HASH_FOREACH_END();

	return num ? 1 : 0;
}
/* }}} */

static int pthreads_sockets_from_fd_set(zval *sockets, fd_set *fds) /* {{{ */
{
	zval		*element;
	zval		set;
	int			num = 0;
	zend_ulong  idx;
	zend_string *key;

	if (Z_TYPE_P(sockets) != IS_ARRAY) {
		return 0;
	}

	array_init(&set);
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(sockets), idx, key, element) {
		pthreads_object_t	*threaded;

		if (Z_TYPE_P(element) != IS_OBJECT || 
			!instanceof_function(Z_OBJCE_P(element), pthreads_socket_entry)) {
			continue;
		}

		threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(element));

		if (PHP_SAFE_FD_ISSET(threaded->store.sock->fd, fds)) {
			if (key) {
				if (!add_assoc_zval_ex(&set, ZSTR_VAL(key), ZSTR_LEN(key), element)) {
					continue;
				}
			} else {
				if (!add_index_zval(&set, idx, element)) {
					continue;
				}
			}

			Z_ADDREF_P(element);
		}

		num++;
	} ZEND_HASH_FOREACH_END();

	zval_ptr_dtor(sockets);

	ZVAL_COPY_VALUE(sockets, &set);

	return num ? 1 : 0;
}
/* }}} */

void pthreads_socket_select(zval *read, zval *write, zval *except, zval *sec, uint32_t usec, zval *errorno, zval *return_value) {
	fd_set rfds, wfds, efds;
	php_socket_t mfd = 0;
	int result = SUCCESS, sets = 0;
	struct timeval  tv;
	struct timeval *tv_p = NULL;

	if(errorno != NULL) {
		zval_ptr_dtor(errorno);
		ZVAL_LONG(errorno, SUCCESS);
	}

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	if (read && Z_TYPE_P(read) == IS_ARRAY) {
		sets += pthreads_sockets_to_fd_set(read, &rfds, &mfd);
	}

	if (write && Z_TYPE_P(write) == IS_ARRAY) {
		sets += pthreads_sockets_to_fd_set(write, &wfds, &mfd);
	}

	if (except && Z_TYPE_P(except) == IS_ARRAY) {
		sets += pthreads_sockets_to_fd_set(except, &efds, &mfd);
	}

	if (!sets) {
		RETURN_FALSE;
	}

	PHP_SAFE_MAX_FD(mfd, 0);

	/* If seconds is not set to null, build the timeval, else we wait indefinitely */
	if (sec != NULL) {
		zval tmp;

		if (Z_TYPE_P(sec) != IS_LONG) {
			tmp = *sec;
			zval_copy_ctor(&tmp);
			convert_to_long(&tmp);
			sec = &tmp;
		}

		/* Solaris + BSD do not like microsecond values which are >= 1 sec */
		if (usec > 999999) {
			tv.tv_sec = Z_LVAL_P(sec) + (usec / 1000000);
			tv.tv_usec = usec % 1000000;
		} else {
			tv.tv_sec = Z_LVAL_P(sec);
			tv.tv_usec = usec;
		}

		tv_p = &tv;
	}

	result = select(mfd + 1, &rfds, &wfds, &efds, tv_p);

	if (result == -1) {
		int eno = php_socket_errno();

		if(errorno != NULL) {
			ZVAL_LONG(errorno, eno);
		}
		PTHREADS_HANDLE_SOCKET_ERROR(eno, "Error");

		RETURN_FALSE;
	}

	if (read) {
		pthreads_sockets_from_fd_set(read, &rfds);
	}

	if (write) {
		pthreads_sockets_from_fd_set(write, &wfds);
	}

	if (except) {
		pthreads_sockets_from_fd_set(except, &efds);
	}

	RETURN_LONG(result);
}

void pthreads_socket_free(pthreads_socket_t *socket, zend_bool closing) {
	if (closing && PTHREADS_IS_VALID_SOCKET(socket)) {
		PTHREADS_CLOSE_SOCKET_INTERNAL(socket);
	}

	efree(socket);
}

void pthreads_socket_recvfrom(zval *object, zval *buffer, zend_long len, zend_long flags, zval *name, zval *port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	socklen_t			slen;
	int					retval;
	zend_string *recv_buf;

	recv_buf = zend_string_alloc(len + 1, 0);

	switch (threaded->store.sock->domain) {
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un	s_un;

			slen = sizeof(s_un);
			s_un.sun_family = AF_UNIX;
			retval = recvfrom(threaded->store.sock->fd, ZSTR_VAL(recv_buf), len, flags, (struct sockaddr *)&s_un, (socklen_t *)&slen);

			if (retval < 0) {
				zend_string_free(recv_buf);
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to recvfrom", errno);

				RETURN_FALSE;
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_ptr_dtor(buffer);
			zval_ptr_dtor(name);

			ZVAL_NEW_STR(buffer, recv_buf);
			ZVAL_STRING(name, s_un.sun_path);
		} break;
#endif
		case AF_INET: {
			struct sockaddr_in	sin;
			char				*address;

			slen = sizeof(sin);
			memset(&sin, 0, slen);
			sin.sin_family = AF_INET;

			if (port == NULL) {
				zend_string_free(recv_buf);
				WRONG_PARAM_COUNT;
			}

			retval = recvfrom(threaded->store.sock->fd, ZSTR_VAL(recv_buf), len, flags, (struct sockaddr *)&sin, (socklen_t *)&slen);

			if (retval < 0) {
				zend_string_free(recv_buf);
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to recvfrom", errno);

				RETURN_FALSE;
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_ptr_dtor(buffer);
			zval_ptr_dtor(name);
			zval_ptr_dtor(port);

			address = inet_ntoa(sin.sin_addr);

			ZVAL_NEW_STR(buffer, recv_buf);
			ZVAL_STRING(name, address ? address : "0.0.0.0");
			ZVAL_LONG(port, ntohs(sin.sin_port));
		} break;
#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6	sin6;
			char				addr6[INET6_ADDRSTRLEN];

			slen = sizeof(sin6);
			memset(&sin6, 0, slen);
			sin6.sin6_family = AF_INET6;

			if (port == NULL) {
				zend_string_free(recv_buf);
				WRONG_PARAM_COUNT;
			}

			retval = recvfrom(threaded->store.sock->fd, ZSTR_VAL(recv_buf), len, flags, (struct sockaddr *)&sin6, (socklen_t *)&slen);

			if (retval < 0) {
				zend_string_free(recv_buf);
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to recvfrom", errno);

				RETURN_FALSE;
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_ptr_dtor(buffer);
			zval_ptr_dtor(name);
			zval_ptr_dtor(port);

			memset(addr6, 0, INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &sin6.sin6_addr, addr6, INET6_ADDRSTRLEN);

			ZVAL_NEW_STR(buffer, recv_buf);
			ZVAL_STRING(name, addr6[0] ? addr6 : "::");
			ZVAL_LONG(port, ntohs(sin6.sin6_port));
		} break;
#endif
		default:
			RETURN_FALSE;
	}

	RETURN_LONG(retval);
}

void pthreads_socket_sendto(zval *object, int argc, zend_string *buf, zend_long len, zend_long flags, zend_string *addr, zend_long port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int	retval;

	switch (threaded->store.sock->domain) {
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un	s_un;

			memset(&s_un, 0, sizeof(s_un));
			s_un.sun_family = AF_UNIX;
			snprintf(s_un.sun_path, 108, "%s", ZSTR_VAL(addr));

			retval = sendto(threaded->store.sock->fd, ZSTR_VAL(buf), ((size_t)len > ZSTR_LEN(buf)) ? ZSTR_LEN(buf) : (size_t)len,	flags, (struct sockaddr *) &s_un, SUN_LEN(&s_un));
		} break;
#endif
		case AF_INET: {
			struct sockaddr_in	sin;

			if (argc != 5) {
				WRONG_PARAM_COUNT;
			}

			memset(&sin, 0, sizeof(sin));
			sin.sin_family = AF_INET;
			sin.sin_port = htons((unsigned short) port);

			if (! pthreads_socket_set_inet_addr(threaded->store.sock, &sin, addr)) {
				RETURN_FALSE;
			}

			retval = sendto(threaded->store.sock->fd, ZSTR_VAL(buf), ((size_t)len > ZSTR_LEN(buf)) ? ZSTR_LEN(buf) : (size_t)len, flags, (struct sockaddr *) &sin, sizeof(sin));
		} break;
#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6	sin6;

			if (argc != 5) {
				WRONG_PARAM_COUNT;
			}

			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family = AF_INET6;
			sin6.sin6_port = htons((unsigned short) port);

			if (! pthreads_socket_set_inet6_addr(threaded->store.sock, &sin6, addr)) {
				RETURN_FALSE;
			}

			retval = sendto(threaded->store.sock->fd, ZSTR_VAL(buf), ((size_t)len > ZSTR_LEN(buf)) ? ZSTR_LEN(buf) : (size_t)len, flags, (struct sockaddr *) &sin6, sizeof(sin6));
		} break;
#endif
		default:
			RETURN_FALSE;
	}

	if (retval == -1) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to write to socket", errno);
		RETURN_FALSE;
	}

	RETURN_LONG(retval);
}

void pthreads_socket_get_last_error(zval *object, zend_bool clear, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if(threaded->store.sock->error == SUCCESS) {
		RETURN_FALSE;
	}
	RETVAL_LONG(threaded->store.sock->error);

	if(clear) {
		PTHREADS_CLEAR_SOCKET_ERROR(threaded->store.sock);
	}
}

void pthreads_socket_strerror(zend_long error, zval *return_value) {
	char *errstr;
	errstr = php_socket_strerror(error, NULL, 0);

	RETVAL_STRING(errstr);

	efree(errstr);
}

void pthreads_socket_clear_error(zval *object) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);
	PTHREADS_CLEAR_SOCKET_ERROR(threaded->store.sock);
}

void pthreads_init_sockets() {
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_UNIX"), AF_UNIX);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_INET"), AF_INET);
#ifdef HAVE_IPV6
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_INET6"), AF_INET6);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_STREAM"), SOCK_STREAM);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_DGRAM"), SOCK_DGRAM);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_RAW"), SOCK_RAW);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_SEQPACKET"), SOCK_SEQPACKET);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_RDM"), SOCK_RDM);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_DEBUG"), SO_DEBUG);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_REUSEADDR"), SO_REUSEADDR);
#ifdef SO_REUSEPORT
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_REUSEPORT"), SO_REUSEPORT);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_KEEPALIVE"), SO_KEEPALIVE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_DONTROUTE"), SO_DONTROUTE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_LINGER"), SO_LINGER);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_BROADCAST"), SO_BROADCAST);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_OOBINLINE"), SO_OOBINLINE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDBUF"), SO_SNDBUF);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVBUF"), SO_RCVBUF);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDLOWAT"), SO_SNDLOWAT);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVLOWAT"), SO_RCVLOWAT);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDTIMEO"), SO_SNDTIMEO);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVTIMEO"), SO_RCVTIMEO);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_TYPE"), SO_TYPE);
#ifdef SO_FAMILY
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_FAMILY"), SO_FAMILY);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_ERROR"), SO_ERROR);
#ifdef SO_BINDTODEVICE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_BINDTODEVICE"), SO_BINDTODEVICE);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOMAXCONN"), SOMAXCONN);
#ifdef TCP_NODELAY
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("TCP_NODELAY"), TCP_NODELAY);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("NORMAL_READ"), PTHREADS_NORMAL_READ);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("BINARY_READ"), PTHREADS_BINARY_READ);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_SOCKET"), SOL_SOCKET);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_TCP"), IPPROTO_TCP);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_UDP"), IPPROTO_UDP);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_OOB"), MSG_OOB);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_WAITALL"), MSG_WAITALL);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CTRUNC"), MSG_CTRUNC);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_TRUNC"), MSG_TRUNC);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_PEEK"), MSG_PEEK);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_DONTROUTE"), MSG_DONTROUTE);
#ifdef MSG_EOR
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_EOR"), MSG_EOR);
#endif
#ifdef MSG_EOF
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_EOF"), MSG_EOF);
#endif
#ifdef MSG_CONFIRM
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CONFIRM"), MSG_CONFIRM);
#endif
#ifdef MSG_ERRQUEUE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_ERRQUEUE"), MSG_ERRQUEUE);
#endif
#ifdef MSG_NOSIGNAL
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_NOSIGNAL"), MSG_NOSIGNAL);
#endif
#ifdef MSG_MORE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_MORE"), MSG_MORE);
#endif
#ifdef MSG_WAITFORONE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_WAITFORONE"), MSG_WAITFORONE);
#endif
#ifdef MSG_CMSG_CLOEXEC
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CMSG_CLOEXEC"), MSG_CMSG_CLOEXEC);
#endif

#ifndef _WIN32
#ifdef EPERM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPERM"), EPERM);
#endif
#ifdef ENOENT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOENT"), ENOENT);
#endif
#ifdef EINTR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINTR"), EINTR);
#endif
#ifdef EIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EIO"), EIO);
#endif
#ifdef ENXIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENXIO"), ENXIO);
#endif
#ifdef E2BIG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("E2BIG"), E2BIG);
#endif
#ifdef EBADF
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADF"), EBADF);
#endif
#ifdef EAGAIN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAGAIN"), EAGAIN);
#endif
#ifdef ENOMEM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMEM"), ENOMEM);
#endif
#ifdef EACCES
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EACCES"), EACCES);
#endif
#ifdef EFAULT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EFAULT"), EFAULT);
#endif
#ifdef ENOTBLK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTBLK"), ENOTBLK);
#endif
#ifdef EBUSY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBUSY"), EBUSY);
#endif
#ifdef EEXIST
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EEXIST"), EEXIST);
#endif
#ifdef EXDEV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EXDEV"), EXDEV);
#endif
#ifdef ENODEV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENODEV"), ENODEV);
#endif
#ifdef ENOTDIR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTDIR"), ENOTDIR);
#endif
#ifdef EISDIR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISDIR"), EISDIR);
#endif
#ifdef EINVAL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINVAL"), EINVAL);
#endif
#ifdef ENFILE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENFILE"), ENFILE);
#endif
#ifdef EMFILE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMFILE"), EMFILE);
#endif
#ifdef ENOTTY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTTY"), ENOTTY);
#endif
#ifdef ENOSPC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSPC"), ENOSPC);
#endif
#ifdef ESPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESPIPE"), ESPIPE);
#endif
#ifdef EROFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EROFS"), EROFS);
#endif
#ifdef EMLINK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMLINK"), EMLINK);
#endif
#ifdef EPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPIPE"), EPIPE);
#endif
#ifdef ENAMETOOLONG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENAMETOOLONG"), ENAMETOOLONG);
#endif
#ifdef ENOLCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOLCK"), ENOLCK);
#endif
#ifdef ENOSYS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSYS"), ENOSYS);
#endif
#ifdef ENOTEMPTY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTEMPTY"), ENOTEMPTY);
#endif
#ifdef ELOOP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELOOP"), ELOOP);
#endif
#ifdef EWOULDBLOCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EWOULDBLOCK"), EWOULDBLOCK);
#endif
#ifdef ENOMSG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMSG"), ENOMSG);
#endif
#ifdef EIDRM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EIDRM"), EIDRM);
#endif
#ifdef ECHRNG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECHRNG"), ECHRNG);
#endif
#ifdef EL2NSYNC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL2NSYNC"), EL2NSYNC);
#endif
#ifdef EL3HLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL3HLT"), EL3HLT);
#endif
#ifdef EL3RST
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL3RST"), EL3RST);
#endif
#ifdef ELNRNG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELNRNG"), ELNRNG);
#endif
#ifdef EUNATCH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUNATCH"), EUNATCH);
#endif
#ifdef ENOCSI
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOCSI"), ENOCSI);
#endif
#ifdef EL2HLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL2HLT"), EL2HLT);
#endif
#ifdef EBADE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADE"), EBADE);
#endif
#ifdef EBADR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADR"), EBADR);
#endif
#ifdef EXFULL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EXFULL"), EXFULL);
#endif
#ifdef ENOANO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOANO"), ENOANO);
#endif
#ifdef EBADRQC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADRQC"), EBADRQC);
#endif
#ifdef EBADSLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADSLT"), EBADSLT);
#endif
#ifdef ENOSTR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSTR"), ENOSTR);
#endif
#ifdef ENODATA
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENODATA"), ENODATA);
#endif
#ifdef ETIME
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIME"), ETIME);
#endif
#ifdef ENOSR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSR"), ENOSR);
#endif
#ifdef ENONET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENONET"), ENONET);
#endif
#ifdef EREMOTE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTE"), EREMOTE);
#endif
#ifdef ENOLINK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOLINK"), ENOLINK);
#endif
#ifdef EADV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADV"), EADV);
#endif
#ifdef ESRMNT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESRMNT"), ESRMNT);
#endif
#ifdef ECOMM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECOMM"), ECOMM);
#endif
#ifdef EPROTO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTO"), EPROTO);
#endif
#ifdef EMULTIHOP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMULTIHOP"), EMULTIHOP);
#endif
#ifdef EBADMSG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADMSG"), EBADMSG);
#endif
#ifdef ENOTUNIQ
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTUNIQ"), ENOTUNIQ);
#endif
#ifdef EBADFD
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADFD"), EBADFD);
#endif
#ifdef EREMCHG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMCHG"), EREMCHG);
#endif
#ifdef ERESTART
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ERESTART"), ERESTART);
#endif
#ifdef ESTRPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESTRPIPE"), ESTRPIPE);
#endif
#ifdef EUSERS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUSERS"), EUSERS);
#endif
#ifdef ENOTSOCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTSOCK"), ENOTSOCK);
#endif
#ifdef EDESTADDRREQ
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDESTADDRREQ"), EDESTADDRREQ);
#endif
#ifdef EMSGSIZE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMSGSIZE"), EMSGSIZE);
#endif
#ifdef EPROTOTYPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTOTYPE"), EPROTOTYPE);
#endif
#ifdef ENOPROTOOPT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOPROTOOPT"), ENOPROTOOPT);
#endif
#ifdef EPROTONOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTONOSUPPORT"), EPROTONOSUPPORT);
#endif
#ifdef ESOCKTNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESOCKTNOSUPPORT"), ESOCKTNOSUPPORT);
#endif
#ifdef EOPNOTSUPP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EOPNOTSUPP"), EOPNOTSUPP);
#endif
#ifdef EPFNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPFNOSUPPORT"), EPFNOSUPPORT);
#endif
#ifdef EAFNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAFNOSUPPORT"), EAFNOSUPPORT);
#endif
#ifdef EADDRINUSE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRINUSE"), EADDRINUSE);
#endif
#ifdef EADDRNOTAVAIL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRNOTAVAIL"), EADDRNOTAVAIL);
#endif
#ifdef ENETDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETDOWN"), ENETDOWN);
#endif
#ifdef ENETUNREACH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETUNREACH"), ENETUNREACH);
#endif
#ifdef ENETRESET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETRESET"), ENETRESET);
#endif
#ifdef ECONNABORTED
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNABORTED"), ECONNABORTED);
#endif
#ifdef ECONNRESET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNRESET"), ECONNRESET);
#endif
#ifdef ENOBUFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOBUFS"), ENOBUFS);
#endif
#ifdef EISCONN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISCONN"), EISCONN);
#endif
#ifdef ENOTCONN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTCONN"), ENOTCONN);
#endif
#ifdef ESHUTDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESHUTDOWN"), ESHUTDOWN);
#endif
#ifdef ETOOMANYREFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETOOMANYREFS"), ETOOMANYREFS);
#endif
#ifdef ETIMEDOUT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIMEDOUT"), ETIMEDOUT);
#endif
#ifdef ECONNREFUSED
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNREFUSED"), ECONNREFUSED);
#endif
#ifdef EHOSTDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTDOWN"), EHOSTDOWN);
#endif
#ifdef EHOSTUNREACH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTUNREACH"), EHOSTUNREACH);
#endif
#ifdef EALREADY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EALREADY"), EALREADY);
#endif
#ifdef EINPROGRESS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINPROGRESS"), EINPROGRESS);
#endif
#ifdef EISNAM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISNAM"), EISNAM);
#endif
#ifdef EREMOTEIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTEIO"), EREMOTEIO);
#endif
#ifdef EDQUOT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDQUOT"), EDQUOT);
#endif
#ifdef ENOMEDIUM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMEDIUM"), ENOMEDIUM);
#endif
#ifdef EMEDIUMTYPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMEDIUMTYPE"), EMEDIUMTYPE);
#endif
#else
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINTR"), WSAEINTR);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADF"), WSAEBADF);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EACCES"), WSAEACCES);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EFAULT"), WSAEFAULT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINVAL"), WSAEINVAL);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMFILE"), WSAEMFILE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EWOULDBLOCK"), WSAEWOULDBLOCK);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINPROGRESS"), WSAEINPROGRESS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EALREADY"), WSAEALREADY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTSOCK"), WSAENOTSOCK);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDESTADDRREQ"), WSAEDESTADDRREQ);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMSGSIZE"), WSAEMSGSIZE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTOTYPE"), WSAEPROTOTYPE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOPROTOOPT"), WSAENOPROTOOPT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTONOSUPPORT"), WSAEPROTONOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESOCKTNOSUPPORT"), WSAESOCKTNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EOPNOTSUPP"), WSAEOPNOTSUPP);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPFNOSUPPORT"), WSAEPFNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAFNOSUPPORT"), WSAEAFNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRINUSE"), WSAEADDRINUSE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRNOTAVAIL"), WSAEADDRNOTAVAIL);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETDOWN"), WSAENETDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETUNREACH"), WSAENETUNREACH);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETRESET"), WSAENETRESET);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNABORTED"), WSAECONNABORTED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNRESET"), WSAECONNRESET);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOBUFS"), WSAENOBUFS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISCONN"), WSAEISCONN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTCONN"), WSAENOTCONN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESHUTDOWN"), WSAESHUTDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETOOMANYREFS"), WSAETOOMANYREFS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIMEDOUT"), WSAETIMEDOUT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNREFUSED"), WSAECONNREFUSED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELOOP"), WSAELOOP);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENAMETOOLONG"), WSAENAMETOOLONG);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTDOWN"), WSAEHOSTDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTUNREACH"), WSAEHOSTUNREACH);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTEMPTY"), WSAENOTEMPTY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROCLIM"), WSAEPROCLIM);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUSERS"), WSAEUSERS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDQUOT"), WSAEDQUOT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESTALE"), WSAESTALE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTE"), WSAEREMOTE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDISCON"), WSAEDISCON);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("SYSNOTREADY"), WSASYSNOTREADY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("VERNOTSUPPORTED"), WSAVERNOTSUPPORTED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NOTINITIALISED"), WSANOTINITIALISED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("HOST_NOT_FOUND"), WSAHOST_NOT_FOUND);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("TRY_AGAIN"), WSATRY_AGAIN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_RECOVERY"), WSANO_RECOVERY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_DATA"), WSANO_DATA);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_ADDRESS"), WSANO_ADDRESS);
#endif
}
#endif
