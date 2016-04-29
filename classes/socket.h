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

PHP_METHOD(Socket, read);
PHP_METHOD(Socket, write);

PHP_METHOD(Socket, setBlocking);

PHP_METHOD(Socket, close);

ZEND_BEGIN_ARG_INFO_EX(Socket___construct, 0, 0, 3)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_setOption, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, value, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_getOption, 0, 2, IS_LONG, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, level, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, name,  IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_bind, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_listen, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, backlog, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_connect, 0, 2, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, host, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_read, 0, 1, IS_STRING, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_write, 0, 1, IS_LONG, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Socket_setBlocking, 0, 1, _IS_BOOL, NULL, 0)
	ZEND_ARG_TYPE_INFO(0, blocking, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

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
	PHP_ME(Socket, accept, Socket_void, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, connect, Socket_connect, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, read, Socket_read, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, write, Socket_write, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, setBlocking, Socket_setBlocking, ZEND_ACC_PUBLIC)
	PHP_ME(Socket, close, Socket_void, ZEND_ACC_PUBLIC)
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
		return;
	}

	pthreads_socket_set_option(getThis(), level, name, value, return_value);
} /* }}} */

/* {{{ proto int Socket::getOption(int level, int name)
	Get long socket option */
PHP_METHOD(Socket, getOption) {
	zend_long level = 0;
	zend_long name = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &level, &name) != SUCCESS) {
		return;
	}

	pthreads_socket_get_option(getThis(), level, name, return_value);
} /* }}} */

/* {{{ proto bool Socket::bind(string host, int port) */
PHP_METHOD(Socket, bind) {
	zend_string *host = NULL;
	zend_long port = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &host, &port) != SUCCESS) {
		return;
	}

	pthreads_socket_bind(getThis(), host, port, return_value);
} /* }}} */

/* {{{ proto bool Socket::listen(int backlog) */
PHP_METHOD(Socket, listen) {
	zend_long backlog = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &backlog) != SUCCESS) {
		return;
	}

	pthreads_socket_listen(getThis(), backlog, return_value);
} /* }}} */

/* {{{ proto Socket Socket::accept(void) */
PHP_METHOD(Socket, accept) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_accept(getThis(), return_value);
} /* }}} */

/* {{{ proto bool Socket::connect(string host, int port) */
PHP_METHOD(Socket, connect) {
	zend_string *host = NULL;
	zend_long port = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sl", &host, &port) != SUCCESS) {
		return;
	}

	pthreads_socket_connect(getThis(), host, port, return_value);
} /* }}} */

/* {{{ proto string Socket::read(int length) */
PHP_METHOD(Socket, read) {
	zend_long length = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &length) != SUCCESS) {
		return;
	}

	pthreads_socket_read(getThis(), length, return_value);
} /* }}} */

/* {{{ proto string Socket::write(string buffer [, int length]) */
PHP_METHOD(Socket, write) {
	zend_string *buffer = NULL;
	zend_long length = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &buffer, &length) != SUCCESS) {
		return;
	}

	pthreads_socket_write(getThis(), buffer, length, return_value);
} /* }}} */

/* {{{ proto bool Socket::setBlocking(bool blocking) */
PHP_METHOD(Socket, setBlocking) {
	zend_bool blocking = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &blocking) != SUCCESS) {
		return;
	}

	pthreads_socket_set_blocking(getThis(), blocking, return_value);
} /* }}} */

/* {{{ proto bool Socket::close(void) */
PHP_METHOD(Socket, close) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	pthreads_socket_close(getThis(), return_value);
} /* }}} */
#	endif
#endif
