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

#ifndef PHP_WIN32
#define PTHREADS_INVALID_SOCKET -1
#define PTHREADS_IS_INVALID_SOCKET(sock) ((sock)->fd < 0)
#define PTHREADS_CLOSE_SOCKET_INTERNAL(sock) close((sock)->fd)
#else
#define PTHREADS_INVALID_SOCKET INVALID_SOCKET
#define PTHREADS_IS_INVALID_SOCKET(sock) ((sock)->fd == INVALID_SOCKET)
#define PTHREADS_CLOSE_SOCKET_INTERNAL(sock) closesocket((sock)->fd)
#endif

#define PTHREADS_IS_VALID_SOCKET(sock) !PTHREADS_IS_INVALID_SOCKET(sock)

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

#define PTHREADS_SOCKET_ERROR() do { \
	int eno = php_socket_errno(); \
	char *estr = eno != SUCCESS ? \
		php_socket_strerror(eno, NULL, 0) : \
		NULL; \
	zend_throw_exception_ex(spl_ce_RuntimeException, eno, \
		"Error (%d): %s", eno, estr ? estr : "unknown"); \
	if (eno != SUCCESS) { \
		if (estr) { \
			efree(estr); \
		} \
	} \
	\
	return; \
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
		return;
	}

	PTHREADS_SOCKET_ERROR();
}

void pthreads_socket_set_option(zval *object, zend_long level, zend_long name, zend_long value, zval *return_value) {
	pthreads_object_t *threaded = 
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	
	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (setsockopt(threaded->store.sock->fd, level, name, (char*) &value, sizeof(value)) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
	}

	RETURN_TRUE;
}

void pthreads_socket_get_option(zval *object, zend_long level, zend_long name, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	socklen_t unused;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (getsockopt(threaded->store.sock->fd, level, name, (void*) &Z_LVAL_P(return_value), &unused) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
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
			return 0;
		}

		if (hentry->h_addrtype != AF_INET) {
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
			return 0;
		}

		if (addrinfo->ai_family != PF_INET6 || addrinfo->ai_addrlen != sizeof(struct sockaddr_in6)) {
			freeaddrinfo(addrinfo);
			return 0;
		}

		memcpy(&(sin->sin6_addr.s6_addr), ((struct sockaddr_in6*)(addrinfo->ai_addr))->sin6_addr.s6_addr, sizeof(struct in6_addr));
		freeaddrinfo(addrinfo);
#else
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
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	php_sockaddr_storage  sa_storage = {0};
	struct sockaddr     *sock_type = (struct sockaddr*) &sa_storage;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	switch (threaded->store.sock->domain) {
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un *sa = (struct sockaddr_un *) sock_type;

			sa->sun_family = AF_UNIX;

			if (ZSTR_LEN(address) >= sizeof(sa->sun_path)) {
				/* throw */
				return;
			}

			memcpy(&sa->sun_path, ZSTR_VAL(address), ZSTR_LEN(address));

			if (bind(threaded->store.sock->fd, (struct sockaddr *) sa, offsetof(struct sockaddr_un, sun_path) + ZSTR_LEN(address)) != SUCCESS) {
				PTHREADS_SOCKET_ERROR();
			}

			RETURN_TRUE;
		} break;
#endif

		case AF_INET: {
			struct sockaddr_in *sa = (struct sockaddr_in *) sock_type;

			sa->sin_family = AF_INET;
			sa->sin_port = htons((unsigned short) port);
			
			if (!pthreads_socket_set_inet_addr(threaded->store.sock, sa, address)) {
				/* throw */
				return;
			}

			if (bind(threaded->store.sock->fd, (struct sockaddr *)sa, sizeof(struct sockaddr_in)) != SUCCESS) {
				PTHREADS_SOCKET_ERROR();
			}
			
			RETURN_TRUE;
		} break;

#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 *sa = (struct sockaddr_in6 *) sock_type;

			sa->sin6_family = AF_INET6;
			sa->sin6_port = htons((unsigned short) port);
			
			if (!pthreads_socket_set_inet6_addr(threaded->store.sock, sa, address)) {
				/* throw */
				return;
			}

			if (bind(threaded->store.sock->fd, (struct sockaddr *)sa, sizeof(struct sockaddr_in6)) != SUCCESS) {
				PTHREADS_SOCKET_ERROR();
			}

			RETURN_TRUE;
		} break;
#endif

		default:
			return;
			/* throw */
	}
}

void pthreads_socket_listen(zval *object, zend_long backlog, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (listen(threaded->store.sock->fd, backlog) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
	}

	RETURN_TRUE;
}

void pthreads_socket_accept(zval *object, zend_class_entry *ce, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_object_t *accepted;

	php_sockaddr_storage sa;
	socklen_t            sa_len = sizeof(sa);

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (!instanceof_function(ce, pthreads_socket_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s is not an instance of Socket",
			ZSTR_VAL(ce->name));
		return;
	}

	object_init_ex(return_value, ce);

	accepted = PTHREADS_FETCH_FROM(Z_OBJ_P(return_value));
	accepted->store.sock->fd = 
		accept(threaded->store.sock->fd, (struct sockaddr*) &sa, &sa_len);

	PTHREADS_SOCKET_CHECK(accepted->store.sock);

	accepted->store.sock->domain = ((struct sockaddr*) &sa)->sa_family;
}

void pthreads_socket_connect(zval *object, zend_string *address, zend_long port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	switch (threaded->store.sock->domain) {
#if HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 sin6 = {0};

			memset(&sin6, 0, sizeof(struct sockaddr_in6));

			sin6.sin6_family = AF_INET6;
			sin6.sin6_port   = htons((unsigned short int)port);

			if (!pthreads_socket_set_inet6_addr(threaded->store.sock, &sin6, address)) {
				PTHREADS_SOCKET_ERROR();
			}

			if (connect(threaded->store.sock->fd, (struct sockaddr *)&sin6, sizeof(struct sockaddr_in6)) != SUCCESS) {
				PTHREADS_SOCKET_ERROR();
			}
			
			RETURN_TRUE;
		} break;
#endif

		case AF_INET: {
			struct sockaddr_in sin = {0};

			sin.sin_family = AF_INET;
			sin.sin_port   = htons((unsigned short int)port);
			
			if (!pthreads_socket_set_inet_addr(threaded->store.sock, &sin, address)) {
				PTHREADS_SOCKET_ERROR();
			}

			if (connect(threaded->store.sock->fd, (struct sockaddr *)&sin, sizeof(struct sockaddr_in)) != SUCCESS) {
				PTHREADS_SOCKET_ERROR();
			}

			RETURN_TRUE;
		} break;

#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un s_un = {0};

			if (ZSTR_LEN(address) >= sizeof(s_un.sun_path)) {
				/* throw */
				return;
			}

			s_un.sun_family = AF_UNIX;
			memcpy(&s_un.sun_path, ZSTR_VAL(address), ZSTR_LEN(address));
			if (connect(threaded->store.sock->fd, (struct sockaddr *) &s_un,  (socklen_t)(XtOffsetOf(struct sockaddr_un, sun_path) + ZSTR_LEN(address)))) {
				PTHREADS_SOCKET_ERROR();
			}

			RETURN_TRUE;
		} break;
#endif
	}
}

void pthreads_socket_read(zval *object, zend_long length, zend_long flags, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *buf;
	int bytes;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	buf = zend_string_alloc(length, 0);
	bytes = recv(threaded->store.sock->fd, ZSTR_VAL(buf), length, flags);

	if (bytes == -1) {
		if (errno != EAGAIN
#ifdef EWOULDBLOCK
			&& errno != EWOULDBLOCK
#endif
		) {
			zend_string_free(buf);
			PTHREADS_SOCKET_ERROR();
		} else {
			zend_string_free(buf);
			ZVAL_FALSE(return_value);
		}
	} else if (!bytes) {
		zend_string_release(buf);
		ZVAL_EMPTY_STRING(return_value);
	} else {
		buf = zend_string_truncate(buf, bytes, 0);

		ZSTR_LEN(buf) = bytes;
		ZSTR_VAL(buf)[ZSTR_LEN(buf)] = 0;

		ZVAL_NEW_STR(return_value, buf);
	}
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
		PTHREADS_SOCKET_ERROR();
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
		PTHREADS_SOCKET_ERROR();
	}

	ZVAL_LONG(return_value, bytes);
}

void pthreads_socket_close(zval *object, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (PTHREADS_CLOSE_SOCKET_INTERNAL(threaded->store.sock) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
	}

	threaded->store.sock->fd = PTHREADS_INVALID_SOCKET;
}

void pthreads_socket_set_blocking(zval *object, zend_bool blocking, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (php_set_sock_blocking(threaded->store.sock->fd, blocking) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
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
		PTHREADS_SOCKET_ERROR();
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
		PTHREADS_SOCKET_ERROR();
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

void pthreads_socket_select(zval *read, zval *write, zval *except, uint32_t sec, uint32_t usec, zval *return_value) {
	fd_set rfds, wfds, efds;
	php_socket_t mfd = 0;
	int result = SUCCESS, sets = 0;
	struct timeval  tv;

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
		return;
	}

	PHP_SAFE_MAX_FD(mfd, 0);
	
	if (sec || usec) {
		if (usec > 999999) {
			tv.tv_sec = sec + (usec / 1000000);
			tv.tv_usec = usec % 1000000;
		} else {
			tv.tv_sec = sec;
			tv.tv_usec = usec;
		}
	}

	result = select(mfd + 1, &rfds, &wfds, &efds, sec || usec ? &tv : NULL);

	if (result == -1) {
		PTHREADS_SOCKET_ERROR();
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

	ZVAL_LONG(return_value, result);
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

	switch (threaded->store.sock->type) {
#ifndef _WIN32
		case AF_UNIX: {
			struct sockaddr_un	s_un;

			slen = sizeof(s_un);
			s_un.sun_family = AF_UNIX;
			retval = recvfrom(threaded->store.sock->fd, ZSTR_VAL(recv_buf), len, flags, (struct sockaddr *)&s_un, (socklen_t *)&slen);

			if (retval < 0) {
				zend_string_free(recv_buf);
				PTHREADS_SOCKET_ERROR();
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_dtor(buffer);
			zval_dtor(name);

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
				PTHREADS_SOCKET_ERROR();
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_dtor(buffer);
			zval_dtor(name);
			zval_dtor(port);

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
				efree(recv_buf);
				WRONG_PARAM_COUNT;
			}

			retval = recvfrom(threaded->store.sock->fd, ZSTR_VAL(recv_buf), len, flags, (struct sockaddr *)&sin6, (socklen_t *)&slen);

			if (retval < 0) {
				zend_string_free(recv_buf);
				PTHREADS_SOCKET_ERROR();
			}
			ZSTR_LEN(recv_buf) = retval;
			ZSTR_VAL(recv_buf)[ZSTR_LEN(recv_buf)] = '\0';

			zval_dtor(buffer);
			zval_dtor(name);
			zval_dtor(port);

			memset(addr6, 0, INET6_ADDRSTRLEN);
			inet_ntop(AF_INET6, &sin6.sin6_addr, addr6, INET6_ADDRSTRLEN);

			ZVAL_NEW_STR(buffer, recv_buf);
			ZVAL_STRING(name, addr6[0] ? addr6 : "::");
			ZVAL_LONG(port, ntohs(sin6.sin6_port));
		} break;
#endif
	}

	RETURN_LONG(retval);
}

void pthreads_socket_sendto(zval *object, int argc, zend_string *buf, zend_long len, zend_long flags, zend_string *addr, zend_long port, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int	retval;

	switch (threaded->store.sock->type) {
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
	}

	if (retval == -1) {
		PTHREADS_SOCKET_ERROR();
	}

	RETURN_LONG(retval);
}
#endif
