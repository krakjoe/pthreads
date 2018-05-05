/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2016                                       |
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
#ifndef HAVE_PTHREADS_CLASS_SOCKET_H
#define HAVE_PTHREADS_CLASS_SOCKET_H
PHP_METHOD(Socket, __construct);

PHP_METHOD(Socket, setOption);
PHP_METHOD(Socket, getOption);

PHP_METHOD(Socket, bind);
PHP_METHOD(Socket, listen);
PHP_METHOD(Socket, accept);
PHP_METHOD(Socket, connect);
PHP_METHOD(Socket, select);

PHP_METHOD(Socket, read);
PHP_METHOD(Socket, write);
PHP_METHOD(Socket, send);
PHP_METHOD(Socket, recvfrom);
PHP_METHOD(Socket, sendto);

PHP_METHOD(Socket, setBlocking);
PHP_METHOD(Socket, getPeerName);
PHP_METHOD(Socket, getSockName);

PHP_METHOD(Socket, getLastError);
PHP_METHOD(Socket, clearError);
PHP_METHOD(Socket, strerror);

#ifdef HAVE_SHUTDOWN
PHP_METHOD(Socket, shutdown);
#endif
PHP_METHOD(Socket, close);

ZEND_BEGIN_ARG_INFO_EX(Socket___construct, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_setOption, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_getOption, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_bind, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_listen, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_connect, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_read, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_write, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_send, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_recvfrom, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(1, buffer, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, name, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(1, port, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_sendto, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, addr, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_setBlocking, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_getHost, 0, 0, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, port, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_select, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(1, read, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, write, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, except, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, sec, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(0, usec, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, error, IS_LONG, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_accept, 0, 0, 0)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Socket_getLastError, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, clear, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_strerror, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, error, IS_LONG, 0)
ZEND_END_ARG_INFO()

#ifdef HAVE_SHUTDOWN
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_shutdown, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, how, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(Socket_void, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_socket_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_SOCKET
#	define HAVE_PTHREADS_CLASS_SOCKET
zend_function_entry pthreads_socket_methods[] = {
	PHP_ME(Socket, __construct, Socket___construct, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, setOption, Socket_setOption, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, getOption, Socket_getOption, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, bind, Socket_bind, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, listen, Socket_listen, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, accept, Socket_accept, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, connect, Socket_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, select, Socket_select, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Socket, read, Socket_read, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, write, Socket_write, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, send, Socket_send, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, recvfrom, Socket_recvfrom, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, sendto, Socket_sendto, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, setBlocking, Socket_setBlocking, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, getPeerName, Socket_getHost, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, getSockName, Socket_getHost, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, close, Socket_void, ZEND_ACC_PUBLIC)
#ifdef HAVE_SHUTDOWN
	PHP_ME(Socket, shutdown, Socket_shutdown, ZEND_ACC_PUBLIC)
#endif
	PHP_ME(Socket, getLastError, Socket_getLastError, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, clearError, Socket_void, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, strerror, Socket_strerror, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ proto Socket::__construct(int domain, int type, int protocol) 
	Create a Threaded Socket */
PHP_METHOD(Socket, __construct) {
	zend_long domain = AF_INET;
	zend_long type = SOCK_STREAM;
	zend_long protocol = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll", &domain, &type, &protocol) != SUCCESS) {
		return;
	}

	pthreads_socket_construct(getThis(), domain, type, protocol);
} /* }}} */

/* {{{ proto bool Socket::setOption(int level, int name, int value) 
	Sets long socket option */
PHP_METHOD(Socket, setOption) {
	zend_long level = 0;
	zend_long name = 0;
	zend_long value = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll", &level, &name, &value) != SUCCESS) {
		RETURN_FALSE;
	}

	pthreads_socket_set_option(getThis(), level, name, value, return_value);
} /* }}} */

/* {{{ proto int Socket::getOption(int level, int name)
	Get long socket option */
PHP_METHOD(Socket, getOption) {
	zend_long level = 0;
	zend_long name = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &level, &name) != SUCCESS) {
		RETURN_LONG(0);
	}

	pthreads_socket_get_option(getThis(), level, name, return_value);
} /* }}} */

/* {{{ proto bool Socket::bind(string host [, int port]) */
PHP_METHOD(Socket, bind) {
	zend_string *host = NULL;
	zend_long port = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &host, &port) != SUCCESS) {
		RETURN_FALSE;
	}

	pthreads_socket_bind(getThis(), host, port, return_value);
} /* }}} */

/* {{{ proto bool Socket::listen([int backlog = 0]) */
PHP_METHOD(Socket, listen) {
	zend_long backlog = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &backlog) != SUCCESS) {
		return;
	}

	pthreads_socket_listen(getThis(), backlog, return_value);
} /* }}} */

/* {{{ proto Socket|bool Socket::accept([string class = self::class]) */
PHP_METHOD(Socket, accept) {
	zend_class_entry *ce = zend_get_called_scope(execute_data);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|C", &ce) != SUCCESS) {
		return;
	}

	pthreads_socket_accept(getThis(), ce, return_value);
} /* }}} */

/* {{{ proto bool Socket::connect(string host[, int port]) */
PHP_METHOD(Socket, connect) {
	zend_string *host = NULL;
	zend_long port = 0;
	int argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc, "S|l", &host, &port) != SUCCESS) {
		RETURN_FALSE;
	}

	pthreads_socket_connect(getThis(), argc, host, port, return_value);
} /* }}} */

/* {{{ proto int|bool Socket::select(array &read, array &write, array &except, int sec [, int usec = 0 [, int &error]]) */
PHP_METHOD(Socket, select) {
	zval *read, *write, *except, *sec, *errorno = NULL;
	zend_long usec = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a/!a/!a/!z!|lz/", &read, &write, &except, &sec, &usec, &errorno) != SUCCESS) {
		return;
	}
	pthreads_socket_select(read, write, except, sec, usec, errorno, return_value);
} /* }}} */

/* {{{ proto string|bool Socket::read(int length [, int flags = 0 [, int type = Socket::BINARY_READ]]) */
PHP_METHOD(Socket, read) {
	zend_long length = 0;
	zend_long flags = 0;
	zend_long type = PTHREADS_BINARY_READ;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|ll", &length, &flags, &type) != SUCCESS) {
		return;
	}

	pthreads_socket_read(getThis(), length, flags, type, return_value);
} /* }}} */

/* {{{ proto int|bool Socket::write(string buffer [, int length]) */
PHP_METHOD(Socket, write) {
	zend_string *buffer = NULL;
	zend_long length = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &buffer, &length) != SUCCESS) {
		return;
	}

	pthreads_socket_write(getThis(), buffer, length, return_value);
} /* }}} */

/* {{{ proto int|bool Socket::send(string buffer, int length, int flags) */
PHP_METHOD(Socket, send) {
	zend_string *buffer = NULL;
	zend_long length = 0;
	zend_long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sll", &buffer, &length, &flags) != SUCCESS) {
		return;
	}

	pthreads_socket_send(getThis(), buffer, length, flags, return_value);
} /* }}} */

/* {{{ proto bool Socket::recvfrom(string &buf, int length, int flags, string &name [, int &port ]) */
PHP_METHOD(Socket, recvfrom) {
	zval		*buffer, *name, *port = NULL;
	zend_long	len, flags;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z/llz/|z/", &buffer, &len, &flags, &name, &port) == FAILURE) {
		return;
	}

	/* overflow check */
	if ((len + 2) < 3) {
		RETURN_FALSE;
	}

	pthreads_socket_recvfrom(getThis(), buffer, len, flags, name, port, return_value);
} /* }}} */

/* {{{ proto bool Socket::sendto(string buf, int length, int flags, string addr [, int port ]) */
PHP_METHOD(Socket, sendto) {
	zend_string *buffer, *address = NULL;
	zend_long	len, flags, port = 0;
	int	argc = ZEND_NUM_ARGS();

	if (zend_parse_parameters(argc, "SllS|l", &buffer, &len, &flags, &address, &port) == FAILURE) {
		return;
	}

	pthreads_socket_sendto(getThis(), argc, buffer, len, flags, address, port, return_value);
} /* }}} */

/* {{{ proto bool Socket::setBlocking(bool blocking) */
PHP_METHOD(Socket, setBlocking) {
	zend_bool blocking = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &blocking) != SUCCESS) {
		RETURN_FALSE;
	}

	pthreads_socket_set_blocking(getThis(), blocking, return_value);
} /* }}} */

/* {{{ proto array Socket::getPeerName([bool port = true]) */
PHP_METHOD(Socket, getPeerName) {
	zend_bool port = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &port) != SUCCESS) {
		return;
	}

	pthreads_socket_get_peer_name(getThis(), port, return_value);
} /* }}} */

/* {{{ proto array Socket::getSockName([bool port = true]) */
PHP_METHOD(Socket, getSockName) {
	zend_bool port = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &port) != SUCCESS) {
		return;
	}

	pthreads_socket_get_sock_name(getThis(), port, return_value);
} /* }}} */

/* {{{ proto int|bool Socket::getLastError([bool clear = false]) */
PHP_METHOD(Socket, getLastError) {
	zend_bool clear = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &clear) != SUCCESS) {
		return;
	}

	pthreads_socket_get_last_error(getThis(), clear, return_value);
} /* }}} */

/* {{{ proto void Socket::clearError() */
PHP_METHOD(Socket, clearError) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_clear_error(getThis());
} /* }}} */

/* {{{ proto string|null Socket::strerror(int error) */
PHP_METHOD(Socket, strerror) {
	zend_long error = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &error) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_socket_strerror(error, return_value);
} /* }}} */

#ifdef HAVE_SHUTDOWN
/* {{{ proto bool Socket::shutdown(int how = Socket::SHUTDOWN_BOTH) */
PHP_METHOD(Socket, shutdown) {
	zend_long how_shutdown = SHUT_RDWR;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &how_shutdown) != SUCCESS) {
		RETURN_FALSE;
	}

	if (how_shutdown != SHUT_RD && how_shutdown != SHUT_WR && how_shutdown != SHUT_RDWR) {
		zend_throw_exception(spl_ce_InvalidArgumentException, "Invalid shutdown type", 0);
		return;
	}

	pthreads_socket_shutdown(getThis(), how_shutdown, return_value);
} /* }}} */
#endif

/* {{{ proto bool Socket::close(void) */
PHP_METHOD(Socket, close) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_close(getThis(), return_value);
} /* }}} */
#	endif
#endif
