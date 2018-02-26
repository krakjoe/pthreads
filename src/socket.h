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
#ifndef HAVE_PTHREADS_SOCKET_H
#define HAVE_PTHREADS_SOCKET_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef PHP_WIN32
# include "sockets/windows_common.h"
# define SOCK_EINVAL WSAEINVAL
#else
# define set_errno(a) (errno = a)
# define SOCK_EINVAL EINVAL
#endif

typedef struct _pthreads_socket_t {
	php_socket_t fd;
	zend_long domain;
	zend_long type;
	zend_long error;
	zend_long protocol;
	zend_bool blocking;
} pthreads_socket_t;


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

pthreads_socket_t* pthreads_socket_alloc(void);
void pthreads_socket_construct(zval *object, zend_long domain, zend_long type, zend_long protocol);
void pthreads_socket_set_option(zval *object, zend_long level, zend_long name, zval *value, zval *return_value);
void pthreads_socket_get_option(zval *object, zend_long level, zend_long name, zval *return_value);
void pthreads_socket_bind(zval *object, zend_string *address, zend_long port, zval *return_value);
void pthreads_socket_listen(zval *object, zend_long backlog, zval *return_value);
void pthreads_socket_accept(zval *object, zend_class_entry *ce, zval *return_value);
void pthreads_socket_connect(zval *object, int argc, zend_string *address, zend_long port, zval *return_value);
void pthreads_socket_read(zval *object, zend_long length, zend_long flags, zval *return_value);
void pthreads_socket_write(zval *object, zend_string *buf, zend_long length, zval *return_value);
void pthreads_socket_send(zval *object, zend_string *buf, zend_long length, zend_long flags, zval *return_value);
void pthreads_socket_close(zval *object, zval *return_value);
void pthreads_socket_set_blocking(zval *object, zend_bool blocking, zval *return_value);
void pthreads_socket_get_peer_name(zval *object, zend_bool port, zval *return_value);
void pthreads_socket_get_sock_name(zval *object, zend_bool port, zval *return_value);
void pthreads_socket_select(zval *read, zval *write, zval *except, zval *sec, uint32_t usec, zval *errorno, zval *return_value);
void pthreads_socket_free(pthreads_socket_t *socket, zend_bool closing);
void pthreads_socket_recvfrom(zval *object, zval *buffer, zend_long len, zend_long flags, zval *name, zval *port, zval *return_value);
void pthreads_socket_sendto(zval *object, int argc, zend_string *buf, zend_long len, zend_long flags, zend_string *addr, zend_long port, zval *return_value);
void pthreads_socket_get_last_error(zval *object, zend_bool clear, zval *return_value);
void pthreads_socket_clear_error(zval *object);

#endif
