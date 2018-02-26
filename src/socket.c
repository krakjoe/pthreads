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

#include "sockets/sockaddr_conv.h"
#include "sockets/multicast.h"
#include "sockets/conversions.h"


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

void pthreads_socket_set_option(zval *object, zend_long level, zend_long name, zval *value, zval *return_value) {

	struct linger			lv;
	int						ov, optlen;
#ifdef PHP_WIN32
	int						timeout;
#else
	struct					timeval tv;
#endif
	void 					*opt_ptr;
	HashTable		 		*opt_ht;
	zval 					*l_onoff, *l_linger;
	zval		 			*sec, *usec;

	pthreads_object_t *threaded = 
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	set_errno(0);

#define HANDLE_SUBCALL(res) \
	do { \
		if (res == 1) { goto default_case; } \
		else if (res == SUCCESS) { RETURN_TRUE; } \
		else { RETURN_FALSE; } \
	} while (0)


	if (level == IPPROTO_IP) {
		int res = pthreads_do_setsockopt_ip_mcast(threaded->store.sock, level, name, value);
		HANDLE_SUBCALL(res);
	}

#if HAVE_IPV6
	else if (level == IPPROTO_IPV6) {
		int res = pthreads_do_setsockopt_ipv6_mcast(threaded->store.sock, level, name, value);
		if (res == 1) {
			res = pthreads_do_setsockopt_ipv6_rfc3542(threaded->store.sock, level, name, value);
		}
		HANDLE_SUBCALL(res);
	}
#endif

	switch (name) {
		case SO_LINGER: {
			const char l_onoff_key[] = "l_onoff";
			const char l_linger_key[] = "l_linger";

			convert_to_array_ex(value);
			opt_ht = Z_ARRVAL_P(value);

			if ((l_onoff = zend_hash_str_find(opt_ht, l_onoff_key, sizeof(l_onoff_key) - 1)) == NULL) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "no key \"%s\" passed in optval", l_onoff_key);
				RETURN_FALSE;
			}
			if ((l_linger = zend_hash_str_find(opt_ht, l_linger_key, sizeof(l_linger_key) - 1)) == NULL) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "no key \"%s\" passed in optval", l_linger_key);
				RETURN_FALSE;
			}

			convert_to_long_ex(l_onoff);
			convert_to_long_ex(l_linger);

			lv.l_onoff = (unsigned short)Z_LVAL_P(l_onoff);
			lv.l_linger = (unsigned short)Z_LVAL_P(l_linger);

			optlen = sizeof(lv);
			opt_ptr = &lv;
			break;
		}

		case SO_RCVTIMEO:
		case SO_SNDTIMEO: {
			const char sec_key[] = "sec";
			const char usec_key[] = "usec";

			convert_to_array_ex(value);
			opt_ht = Z_ARRVAL_P(value);

			if ((sec = zend_hash_str_find(opt_ht, sec_key, sizeof(sec_key) - 1)) == NULL) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "no key \"%s\" passed in optval", sec_key);
				RETURN_FALSE;
			}
			if ((usec = zend_hash_str_find(opt_ht, usec_key, sizeof(usec_key) - 1)) == NULL) {
				zend_throw_exception_ex(spl_ce_RuntimeException, 0, "no key \"%s\" passed in optval", usec_key);
				RETURN_FALSE;
			}

			convert_to_long_ex(sec);
			convert_to_long_ex(usec);
#ifndef PHP_WIN32
			tv.tv_sec = Z_LVAL_P(sec);
			tv.tv_usec = Z_LVAL_P(usec);
			optlen = sizeof(tv);
			opt_ptr = &tv;
#else
			timeout = Z_LVAL_P(sec) * 1000 + Z_LVAL_P(usec) / 1000;
			optlen = sizeof(int);
			opt_ptr = &timeout;
#endif
			break;
		}
#ifdef SO_BINDTODEVICE
		case SO_BINDTODEVICE: {
			if (Z_TYPE_P(value) == IS_STRING) {
				opt_ptr = Z_STRVAL_P(value);
				optlen = Z_STRLEN_P(value);
			} else {
				opt_ptr = "";
				optlen = 0;
			}
			break;
		}
#endif
		default:
default_case:
			convert_to_long_ex(value);
			ov = Z_LVAL_P(value);

			optlen = sizeof(ov);
			opt_ptr = &ov;
			break;
	}

	if (setsockopt(threaded->store.sock->fd, level, name, opt_ptr, optlen) != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to set socket option", errno);

		RETURN_FALSE;
	}

	RETURN_TRUE;
}

void pthreads_socket_get_option(zval *object, zend_long level, zend_long name, zval *return_value) {

	struct linger	linger_val;
	struct timeval	tv;
#ifdef PHP_WIN32
	int				timeout = 0;
#endif
	socklen_t		optlen;
	zend_long		other_val;

	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (level == IPPROTO_IP) {
		switch (name) {
			case IP_MULTICAST_IF: {
				struct in_addr if_addr;
				unsigned int if_index;
				optlen = sizeof(if_addr);
				if (getsockopt(threaded->store.sock->fd, level, name, (char*)&if_addr, &optlen) != 0) {
					PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);
					RETURN_FALSE;
				}

				if (pthreads_add4_to_if_index(&if_addr, threaded->store.sock, &if_index) == SUCCESS) {
					RETURN_LONG((zend_long) if_index);
				} else {
					RETURN_FALSE;
				}
			}
		}
	}
#if HAVE_IPV6
	else if (level == IPPROTO_IPV6) {
		int ret = pthreads_do_getsockopt_ipv6_rfc3542(threaded->store.sock, level, name, return_value);
		if (ret == SUCCESS) {
			return;
		} else if (ret == FAILURE) {
			RETURN_FALSE;
		} /* else continue */
	}
#endif

	/* sol_socket options and general case */
	switch(name) {
		case SO_LINGER:
			optlen = sizeof(linger_val);

			if (getsockopt(threaded->store.sock->fd, level, name, (char*)&linger_val, &optlen) != SUCCESS) {
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);
				RETURN_FALSE;
			}

			array_init(return_value);
			add_assoc_long(return_value, "l_onoff", linger_val.l_onoff);
			add_assoc_long(return_value, "l_linger", linger_val.l_linger);
			break;

		case SO_RCVTIMEO:
		case SO_SNDTIMEO:
#ifndef PHP_WIN32
			optlen = sizeof(tv);

			if (getsockopt(threaded->store.sock->fd, level, name, (char*)&tv, &optlen) != SUCCESS) {
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);
				RETURN_FALSE;
			}
#else
			optlen = sizeof(int);

			if (getsockopt(threaded->store.sock->fd, level, name, (char*)&timeout, &optlen) != SUCCESS) {
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);
				RETURN_FALSE;
			}

			tv.tv_sec = timeout ? timeout / 1000 : 0;
			tv.tv_usec = timeout ? (timeout * 1000) % 1000000 : 0;
#endif

			array_init(return_value);

			add_assoc_long(return_value, "sec", tv.tv_sec);
			add_assoc_long(return_value, "usec", tv.tv_usec);
			break;

		default:
			optlen = sizeof(other_val);

			if (getsockopt(threaded->store.sock->fd, level, name, (char*)&other_val, &optlen) != SUCCESS) {
				PTHREADS_SOCKET_ERROR(threaded->store.sock, "Unable to retrieve socket option", errno);
				RETURN_FALSE;
			}
			if (optlen == 1)
				other_val = *((unsigned char *)&other_val);

			RETURN_LONG(other_val);
			break;
	}
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
	}

	if (retval != SUCCESS) {
		PTHREADS_SOCKET_ERROR(threaded->store.sock, "unable to connect", errno);
		RETURN_FALSE;
	}

	RETURN_TRUE;
}

void pthreads_socket_read(zval *object, zend_long length, zend_long flags, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *buf;
	int bytes;

	PTHREADS_SOCKET_CHECK(threaded->store.sock);

	if (length < 1) {
		RETURN_FALSE;
	}

	buf = zend_string_alloc(length, 0);
	bytes = recv(threaded->store.sock->fd, ZSTR_VAL(buf), length, flags);

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

			if (!pthreads_socket_set_inet6_addr(threaded->store.sock, &sin6, addr)) {
				RETURN_FALSE;
			}

			retval = sendto(threaded->store.sock->fd, ZSTR_VAL(buf), ((size_t)len > ZSTR_LEN(buf)) ? ZSTR_LEN(buf) : (size_t)len, flags, (struct sockaddr *) &sin6, sizeof(sin6));
		} break;
#endif
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

void pthreads_socket_clear_error(zval *object) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	PTHREADS_SOCKET_CHECK(threaded->store.sock);
	PTHREADS_CLEAR_SOCKET_ERROR(threaded->store.sock);
}
#endif
