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

#define PTHREADS_SOCKET_CHECK(sock) do { \
	if ((sock)->fd < 0) { \
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, \
			"socket found in invalid state"); \
		return; \
	} \
} while(0)

#define PTHREADS_SOCKET_ERROR() do { \
	int eno = php_socket_errno(); \
	char *estr = eno != SUCCESS ? \
		php_socket_strerror(eno, NULL, 0) : \
		NULL; \
	zend_throw_exception_ex(spl_ce_RuntimeException, eno, \
		"socket error (%d): %s", eno, estr ? estr : "unknown"); \
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

	if (threaded->store.sock->fd > -1) {
		threaded->store.sock->domain = domain;
		threaded->store.sock->type = type;
		threaded->store.sock->protocol = protocol;
	}
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
#ifdef AF_UNIX
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

#ifdef HAVE_IPV6
		case AF_INET6: {
			struct sockaddr_in6 *sa = (struct sockaddr_in6 *) sock_type;
			socklen_t			  sock_len;

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
#ifdef HAVE_IPV6
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

#ifdef AF_UNIX
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

void pthreads_socket_read(zval *object, zend_long length, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *buf;
	int bytes;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	buf = zend_string_alloc(length, 0);
	bytes = recv(threaded->store.sock->fd, ZSTR_VAL(buf), length, 0);

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

void pthreads_socket_close(zval *object, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (close(threaded->store.sock->fd) != SUCCESS) {
		PTHREADS_SOCKET_ERROR();
	}

	threaded->store.sock->fd = -1;
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

void pthreads_socket_free(pthreads_socket_t *socket, zend_bool closing) {
	if (closing) {
		if (socket->fd > -1)
			close(socket->fd);
	}

	efree(socket);
}
#endif
