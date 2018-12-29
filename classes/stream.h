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
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_CLASS_STREAM_H
#define HAVE_PTHREADS_CLASS_STREAM_H

// Streams
/* {{{ */
static PHP_METHOD(Streams, __construct) {
	zend_throw_error(NULL, "Instantiation of 'Streams' is not allowed");
} /* }}} */
PHP_METHOD(Streams, getFilters);
PHP_METHOD(Streams, getTransports);
PHP_METHOD(Streams, getWrappers);
PHP_METHOD(Streams, registerWrapper);
PHP_METHOD(Streams, unregisterWrapper);
PHP_METHOD(Streams, registerFilter);

// StreamContext
PHP_METHOD(StreamContext, __construct);
PHP_METHOD(StreamContext, getOptions);
PHP_METHOD(StreamContext, setOptions);
PHP_METHOD(StreamContext, setOption);
PHP_METHOD(StreamContext, getParams);
PHP_METHOD(StreamContext, setParams);
PHP_METHOD(StreamContext, getDefault);
PHP_METHOD(StreamContext, setDefault);

// Stream
/* {{{ */
static PHP_METHOD(Stream, __construct) {
	zend_throw_error(NULL, "Instantiation of 'Stream' is not allowed");
} /* }}} */
PHP_METHOD(Stream, copyToStream);
PHP_METHOD(Stream, appendFilter);
PHP_METHOD(Stream, prependFilter);
PHP_METHOD(Stream, enableCrypto);
PHP_METHOD(Stream, getContents);
PHP_METHOD(Stream, getLine);
PHP_METHOD(Stream, getMetaData);
PHP_METHOD(Stream, isLocal);
PHP_METHOD(Stream, isATTY);
#ifdef PHP_WIN32
PHP_METHOD(Stream, windowsVT100Support);
#endif
PHP_METHOD(Stream, select);
PHP_METHOD(Stream, setBlocking);
PHP_METHOD(Stream, setChunkSize);
PHP_METHOD(Stream, setReadBuffer);
#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
PHP_METHOD(Stream, setTimeout);
#endif
PHP_METHOD(Stream, setWriteBuffer);
PHP_METHOD(Stream, supportsLock);
PHP_METHOD(Stream, fromResource);

// SocketStream
/* {{{ */
static PHP_METHOD(SocketStream, __construct) {
	zend_throw_error(NULL, "Instantiation of 'SocketStream' is not allowed");
} /* }}} */
PHP_METHOD(SocketStream, createClient);
PHP_METHOD(SocketStream, createServer);
#if HAVE_SOCKETPAIR
PHP_METHOD(SocketStream, createPair);
#endif
PHP_METHOD(SocketStream, accept);
PHP_METHOD(SocketStream, getName);
PHP_METHOD(SocketStream, recvfrom);
PHP_METHOD(SocketStream, sendto);
#ifdef HAVE_SHUTDOWN
PHP_METHOD(SocketStream, shutdown);
#endif

// StreamWrapper
/* {{{ */
static PHP_METHOD(StreamWrapper, __construct) {
	zend_throw_error(NULL, "Instantiation of 'StreamWrapper' is not allowed");
} /* }}} */


/**
 * Streams
 */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(Streams_getFilters, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(Streams_getTransports, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(Streams_getWrappers, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Streams_registerWrapper, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, classname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Streams_unregisterWrapper, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Streams_registerFilter, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, filtername, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, classname, IS_STRING, 0)
ZEND_END_ARG_INFO()

/**
 * StreamContext
 */
ZEND_BEGIN_ARG_INFO_EX(StreamContext___construct, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(StreamContext_getOptions, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(StreamContext_setOptions, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(StreamContext_setOption, 0, 3, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, wrapper, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, option, IS_STRING, 0)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(StreamContext_getParams, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(StreamContext_setParams, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, params, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(StreamContext_getDefault, 0, 0, StreamContext, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(StreamContext_setDefault, 0, 1, StreamContext, 0)
	ZEND_ARG_TYPE_INFO(0, options, IS_ARRAY, 0)
ZEND_END_ARG_INFO()

/**
 * Stream
 */
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_copyToStream, 0, 1, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, dest, Stream, 0)
	ZEND_ARG_TYPE_INFO(0, maxlength, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(Stream_appendFilter, 0, 1, StreamFilter, 1)
	ZEND_ARG_TYPE_INFO(0, filtername, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, read_write, IS_LONG, 0)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(Stream_prependFilter, 0, 1, StreamFilter, 1)
	ZEND_ARG_TYPE_INFO(0, filtername, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, read_write, IS_LONG, 0)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stream_enableCrypto, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, crypto_type, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, session_stream, Stream, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_getContents, 0, 0, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, maxlength, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_getLine, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, ending, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_getMetaData, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_isLocal, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, stream_or_url, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_isATTY, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

#ifdef PHP_WIN32
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_windowsVT100Support, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, enable, _IS_BOOL, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(Stream_select, 0, 0, 4)
	ZEND_ARG_TYPE_INFO(1, read, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, write, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(1, except, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, tv_sec, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, tv_usec, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_setBlocking, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, mode, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_setChunkSize, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, chunk_size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_setReadBuffer, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_LONG, 0)
ZEND_END_ARG_INFO()

#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_setTimeout, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, seconds, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, microseconds, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_setWriteBuffer, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Stream_supportsLock, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(Stream_fromResource, 0, 1, Stream, 0)
	ZEND_ARG_TYPE_INFO(0, stream, IS_RESOURCE, 0)
ZEND_END_ARG_INFO()

/**
 * SocketStream
 */
ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(SocketStream_createClient, 0, 1, SocketStream, 1)
	ZEND_ARG_TYPE_INFO(0, remote_socket, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(1, errno, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(1, errstr, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(SocketStream_createServer, 0, 1, SocketStream, 1)
	ZEND_ARG_TYPE_INFO(0, local_socket, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(1, errno, IS_LONG, 1)
	ZEND_ARG_TYPE_INFO(1, errstr, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

#if HAVE_SOCKETPAIR
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SocketStream_createPair, 0, 3, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, domain, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, type, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, protocol, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(SocketStream_accept, 0, 0, SocketStream, 1)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
	ZEND_ARG_TYPE_INFO(1, peername, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SocketStream_getName, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, $want_peer, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SocketStream_recvfrom, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(1, address, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SocketStream_sendto, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, data, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, address, IS_STRING, 0)
ZEND_END_ARG_INFO()

#ifdef HAVE_SHUTDOWN
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(SocketStream_shutdown, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, $how, IS_LONG, 0)
ZEND_END_ARG_INFO()
#endif

/**
 * StreamWrapper
 */

extern zend_function_entry pthreads_streams_streams_methods[];
extern zend_function_entry pthreads_streams_context_methods[];
extern zend_function_entry pthreads_streams_stream_methods[];
extern zend_function_entry pthreads_streams_socket_stream_methods[];
extern zend_function_entry pthreads_streams_wrapper_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_STREAM
#	define HAVE_PTHREADS_CLASS_STREAM

zend_function_entry pthreads_streams_streams_methods[] = {
	PHP_ME(Streams, __construct         , NULL                      , ZEND_ACC_PRIVATE)
	PHP_ME(Streams, getFilters          , Streams_getFilters        , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Streams, getTransports       , Streams_getTransports     , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Streams, getWrappers         , Streams_getWrappers       , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Streams, registerWrapper     , Streams_registerWrapper   , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Streams, unregisterWrapper   , Streams_unregisterWrapper , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Streams, registerFilter      , Streams_registerFilter    , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

zend_function_entry pthreads_streams_context_methods[] = {
	PHP_ME(StreamContext, __construct   , StreamContext___construct , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, getOptions    , StreamContext_getOptions  , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, setOptions    , StreamContext_setOptions  , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, setOption     , StreamContext_setOption   , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, getParams     , StreamContext_getParams   , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, setParams     , StreamContext_setParams   , ZEND_ACC_PUBLIC)
	PHP_ME(StreamContext, getDefault    , StreamContext_getDefault  , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(StreamContext, setDefault    , StreamContext_setDefault  , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

zend_function_entry pthreads_streams_stream_methods[] = {
	PHP_ME(Stream, __construct          , NULL                      , ZEND_ACC_PRIVATE)
	PHP_ME(Stream, copyToStream         , Stream_copyToStream       , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, appendFilter         , Stream_appendFilter       , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, prependFilter        , Stream_prependFilter      , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, enableCrypto         , Stream_enableCrypto       , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, getContents          , Stream_getContents        , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, getLine              , Stream_getLine            , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, getMetaData          , Stream_getMetaData        , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, isLocal              , Stream_isLocal            , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, isATTY               , Stream_isATTY             , ZEND_ACC_PUBLIC)
#ifdef PHP_WIN32
	PHP_ME(Stream, windowsVT100Support  , Stream_windowsVT100Support, ZEND_ACC_PUBLIC)
#endif
	PHP_ME(Stream, select               , Stream_select             , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Stream, setBlocking          , Stream_setBlocking        , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, setChunkSize         , Stream_setChunkSize       , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, setReadBuffer        , Stream_setReadBuffer      , ZEND_ACC_PUBLIC)
#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
	PHP_ME(Stream, setTimeout           , Stream_setTimeout         , ZEND_ACC_PUBLIC)
#endif
	PHP_ME(Stream, setWriteBuffer       , Stream_setWriteBuffer     , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, supportsLock         , Stream_supportsLock       , ZEND_ACC_PUBLIC)
	PHP_ME(Stream, fromResource         , Stream_fromResource       , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

zend_function_entry pthreads_streams_socket_stream_methods[] = {
	PHP_ME(SocketStream, __construct    , NULL                      , ZEND_ACC_PRIVATE)
	PHP_ME(SocketStream, createClient   , SocketStream_createClient , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(SocketStream, createServer   , SocketStream_createServer , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#if HAVE_SOCKETPAIR
	PHP_ME(SocketStream, createPair     , SocketStream_createPair   , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
#endif
	PHP_ME(SocketStream, accept         , SocketStream_accept       , ZEND_ACC_PUBLIC)
	PHP_ME(SocketStream, getName        , SocketStream_getName      , ZEND_ACC_PUBLIC)
	PHP_ME(SocketStream, recvfrom       , SocketStream_recvfrom     , ZEND_ACC_PUBLIC)
	PHP_ME(SocketStream, sendto         , SocketStream_sendto       , ZEND_ACC_PUBLIC)
#ifdef HAVE_SHUTDOWN
	PHP_ME(SocketStream, shutdown       , SocketStream_shutdown     , ZEND_ACC_PUBLIC)
#endif
	PHP_FE_END
};

zend_function_entry pthreads_streams_wrapper_methods[] = {
	PHP_ME(StreamWrapper, __construct   , NULL                      , ZEND_ACC_PRIVATE)
	PHP_FE_END
};

/**
 *
 * Streams
 *
 */
/* {{{ proto array Streams::getFilters(void)
   Returns a list of registered filters */
PHP_METHOD(Streams, getFilters) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_get_filters(return_value);
} /* }}} */

/* {{{ proto array Streams::getTransports(void)
   Returns a list of registered filters */
PHP_METHOD(Streams, getTransports) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_get_transports(return_value);
} /* }}} */

/* {{{ proto array Streams::getWrappers(void)
   Returns a list of registered filters */
PHP_METHOD(Streams, getWrappers) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_get_wrappers(return_value);
} /* }}} */

/* {{{ proto bool Streams::registerWrapper(string protocol, string classname [, int flags = 0]) */
PHP_METHOD(Streams, registerWrapper) {

	zend_string *protocol = NULL, *classname = NULL;
	zend_long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS|l", &protocol, &classname, &flags) != SUCCESS) {
		RETURN_FALSE;
	}

	if (!ZSTR_LEN(protocol)) {
		php_error_docref(NULL, E_WARNING, "Protocol name cannot be empty");
		return;
	}

	if (!ZSTR_LEN(classname)) {
		php_error_docref(NULL, E_WARNING, "Class name cannot be empty");
		return;
	}

	pthreads_streams_api_register_wrapper(protocol, classname, flags, return_value);
} /* }}} */

/* {{{ proto bool Streams::unregisterWrapper(string protocol) */
PHP_METHOD(Streams, unregisterWrapper) {
	zend_string *protocol = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &protocol) != SUCCESS) {
		RETURN_FALSE;
	}

	if (!ZSTR_LEN(protocol)) {
		php_error_docref(NULL, E_WARNING, "Protocol name cannot be empty");
		return;
	}

	pthreads_streams_api_unregister_wrapper(protocol, return_value);
} /* }}} */

/* {{{ proto bool Streams::registerFilter(string filtername, string classname)  */
PHP_METHOD(Streams, registerFilter) {
	zend_string *filtername = NULL, *classname = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &filtername, &classname) != SUCCESS) {
		RETURN_FALSE;
	}

	if (!ZSTR_LEN(filtername)) {
		php_error_docref(NULL, E_WARNING, "Filter name cannot be empty");
		return;
	}

	if (!ZSTR_LEN(classname)) {
		php_error_docref(NULL, E_WARNING, "Class name cannot be empty");
		return;
	}

	pthreads_streams_api_register_filter(filtername, classname, return_value);
} /* }}} */

/**
 *
 * StreamContext
 *
 */

/* {{{ proto StreamContext::__construct([array options [, array $params]]) */
PHP_METHOD(StreamContext, __construct) {
	zval *options = NULL, *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|aa", &options, &params) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_create(getThis(), options, params, return_value);
} /* }}} */

/* {{{ proto array StreamContext::getOptions() */
PHP_METHOD(StreamContext, getOptions) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_context_get_options(getThis(), return_value);
} /* }}} */

/* {{{ proto bool StreamContext::setOptions() */
PHP_METHOD(StreamContext, setOptions) {
	zval *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|a", &options) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_set_options(getThis(), options, return_value);
} /* }}} */

/* {{{ proto bool StreamContext::setOption(string wrapper, string option, value) */
PHP_METHOD(StreamContext, setOption) {
	zend_string *wrapper, *option;
	zval *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SSz", &wrapper, &option, &value) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_set_option(getThis(), wrapper, option, value, return_value);
} /* }}} */

/* {{{ proto array StreamContext::getParams() */
PHP_METHOD(StreamContext, getParams) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_context_get_params(getThis(), return_value);
} /* }}} */

/* {{{ proto bool StreamContext::setParams() */
PHP_METHOD(StreamContext, setParams) {
	zval *params = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|a", &params) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_set_params(getThis(), params, return_value);
} /* }}} */

/* {{{ proto static StreamContext StreamContext::getDefault([array options]) */
PHP_METHOD(StreamContext, getDefault) {
	zval *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|a", &options) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_get_default(options, return_value);
} /* }}} */

/* {{{ proto static StreamContext StreamContext::setDefault(array options) */
PHP_METHOD(StreamContext, setDefault) {
	zval *options = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a", &options) != SUCCESS) {
		return;
	}

	pthreads_streams_api_context_set_default(options, return_value);
} /* }}} */

/**
 *
 * Stream
 *
 */

/* {{{ proto int Stream::copyToStream(Stream dest [, int maxlength = -1 [, int offset = 0]]) */
PHP_METHOD(Stream, copyToStream) {
	zval *dest = NULL;
	zend_long maxlength = PTHREADS_STREAM_COPY_ALL, offset = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|ll", &dest, pthreads_stream_entry, &maxlength, &offset) != SUCCESS) {
		return;
	}

	if (!instanceof_function(Z_OBJCE_P(dest), pthreads_stream_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Stream objects may be submitted, %s is no Stream",
			ZSTR_VAL(Z_OBJCE_P(dest)->name));
		return;
	}

	pthreads_streams_api_stream_copy_to_stream(getThis(), dest, maxlength, offset, return_value);
} /* }}} */

/* {{{ proto ?StreamFilter Stream::appendFilter(string filtername [, int read_write [, mixed params]]) */
PHP_METHOD(Stream, appendFilter) {
	zval *params = NULL;
	zend_long read_write = 0;
	zend_string *filtername;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lz", &filtername, &read_write, &params) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_apply_filter_to_stream(1, getThis(), filtername, read_write, params, return_value);
} /* }}} */

/* {{{ proto ?StreamFilter Stream::prependFilter(string filtername [, int read_write [, mixed params]]) */
PHP_METHOD(Stream, prependFilter) {
	zval *params = NULL;
	zend_long read_write = 0;
	zend_string *filtername;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lz", &filtername, &read_write, &params) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_apply_filter_to_stream(0, getThis(), filtername, read_write, params, return_value);
} /* }}} */

/* {{{ proto mixed Stream::enableCrypto(bool enable [, int crypto_type [, Stream session_stream ]]) */
PHP_METHOD(Stream, enableCrypto) {
	zend_bool enable;
	zend_long cryptokind = 0;
	zval *zsession_stream = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b|lO", &enable, &cryptokind, &zsession_stream, pthreads_stream_entry) != SUCCESS) {
		return;
	}

	if (enable && (ZEND_NUM_ARGS() < 3 || cryptokind == 0)) {
		zval *val;
		pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(getThis()));

		if (!PTHREADS_GET_CTX_OPT(threaded_stream, "ssl", "crypto_method", val)) {
			php_error_docref(NULL, E_WARNING, "When enabling encryption you must specify the crypto type");
			RETURN_FALSE;
		}

		cryptokind = Z_LVAL_P(val);
	}

	pthreads_streams_api_stream_enable_crypto(getThis(), enable, cryptokind, zsession_stream, return_value);
} /* }}} */

/* {{{ proto string|null Stream::getContents([int maxlength = -1 [, int offset = -1]]) */
PHP_METHOD(Stream, getContents) {
	zend_long maxlength = (ssize_t) PTHREADS_STREAM_COPY_ALL, offset = -1L;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &maxlength, &offset) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_get_contents(getThis(), maxlength, offset, return_value);
} /* }}} */

/* {{{ proto string|null Stream::getLine(int length [, string ending]) */
PHP_METHOD(Stream, getLine) {
	zend_long length;
	zend_string *ending = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|ll", &length, &ending) != SUCCESS) {
		return;
	}

	if (length < 0) {
		php_error_docref(NULL, E_WARNING, "The maximum allowed length must be greater than or equal to zero");
		RETURN_NULL();
	}

	pthreads_streams_api_stream_get_line(getThis(), length, ending, return_value);
} /* }}} */

/* {{{ proto array|null Stream::getMetaData(void) */
PHP_METHOD(Stream, getMetaData) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_stream_get_meta_data(getThis(), return_value);
} /* }}} */

/* {{{ proto bool Stream::isLocal(string stream_or_url) */
PHP_METHOD(Stream, isLocal) {
	zval *stream_or_url = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &stream_or_url) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_is_local(getThis(), stream_or_url, return_value);
} /* }}} */

/* {{{ proto bool Stream::isATTY(void) */
PHP_METHOD(Stream, isATTY) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_stream_isatty(getThis(), return_value);
} /* }}} */

#ifdef PHP_WIN32
/* {{{ proto bool Stream::windowsVT100Support([bool enable]) */
PHP_METHOD(Stream, windowsVT100Support) {
	zend_bool enable = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &enable) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_windows_vt100_support(ZEND_NUM_ARGS(), getThis(), enable, return_value);
} /* }}} */
#endif

/* {{{ proto static mixed Stream::select(array &read, array &write, array &except, int tv_sec [, int tv_usec = 0]) */
PHP_METHOD(Stream, select) {
	zval *read, *write, *except, *sec = NULL;
	zend_long usec = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "a/!a/!a/!z!|l", &read, &write, &except, &sec, &usec) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_select(read, write, except, sec, usec, return_value);
} /* }}} */

/* {{{ proto bool Stream::setBlocking(bool mode) */
PHP_METHOD(Stream, setBlocking) {
	zend_bool blocking = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &blocking) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_set_blocking(getThis(), blocking, return_value);
} /* }}} */

/* {{{ proto int Stream::setChunkSize(int chunk_size) */
PHP_METHOD(Stream, setChunkSize) {
	zend_long csize;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &csize) != SUCCESS) {
		return;
	}

	if (csize <= 0) {
		php_error_docref(NULL, E_WARNING, "The chunk size must be a positive integer, given " ZEND_LONG_FMT, csize);
		RETURN_FALSE;
	}

	/* stream.chunk_size is actually a size_t, but pthreads_stream_set_option
	 * can only use an int to accept the new value and return the old one.
	 * In any case, values larger than INT_MAX for a chunk size make no sense.
	 */
	if (csize > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "The chunk size cannot be larger than %d", INT_MAX);
		RETURN_FALSE;
	}

	pthreads_streams_api_stream_set_chunk_size(getThis(), csize, return_value);
} /* }}} */

/* {{{ proto int Stream::setReadBuffer(int buffer) */
PHP_METHOD(Stream, setReadBuffer) {
	zend_long buffer;
	size_t buff;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &buffer) != SUCCESS) {
		return;
	}
	buff = buffer;

	pthreads_streams_api_stream_set_read_buffer(getThis(), buff, return_value);
} /* }}} */

#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
/* {{{ proto bool Stream::setTimeout(int seconds [, int microseconds = 0]) */
PHP_METHOD(Stream, setTimeout) {
	zend_long seconds, microseconds = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &seconds, &microseconds) != SUCCESS) {
		return;
	}

	pthreads_streams_api_stream_set_timeout(ZEND_NUM_ARGS(), getThis(), seconds, microseconds, return_value);
} /* }}} */
#endif

/* {{{ proto int Stream::setWriteBuffer(int buffer) */
PHP_METHOD(Stream, setWriteBuffer) {
	zend_long buffer;
	size_t buff;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &buffer) != SUCCESS) {
		return;
	}
	buff = buffer;

	pthreads_streams_api_stream_set_write_buffer(getThis(), buff, return_value);
} /* }}} */

/* {{{ proto bool Stream::supportsLock(void) */
PHP_METHOD(Stream, supportsLock) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_stream_supports_lock(getThis(), return_value);
} /* }}} */

/* {{{ proto bool Stream::fromResource(resource stream) */
PHP_METHOD(Stream, fromResource) {
	zval *zstream;
	php_stream *stream;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &zstream) != SUCCESS) {
		return;
	}

	if ((stream = (php_stream*)zend_fetch_resource2_ex(zstream,
				"stream", php_file_le_stream(), php_file_le_pstream())) == NULL) {
		RETURN_NULL();
	}

	pthreads_streams_api_stream_from_resource(stream, return_value);
} /* }}} */



/**
 *
 * SocketStream
 *
 */

/* {{{ proto static SocketStream|null SocketStream::createClient(string remote_socket [, int &errno [, string &errstr [,
			float timeout = ini_get("default_socket_timeout") [, int flags = PTHREADS_STREAM_CLIENT_CONNECT [, StreamContext context ]]]]]) */
PHP_METHOD(SocketStream, createClient) {
	zend_string *remote_socket = NULL;
	zval *zcontext = NULL, *errstr, *errorno = NULL;
	double timeout = (double)FG(default_socket_timeout);
	zend_long flags = PTHREADS_STREAM_CLIENT_CONNECT;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|z/z/dlO", &remote_socket, &errorno, &errstr, &timeout, &flags, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_NULL();
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext) && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_create_client(remote_socket, errorno, errstr, timeout, flags, zcontext, return_value);
} /* }}} */

/* {{{ proto static SocketStream|null SocketStream::createServer(string local_socket [, int &errno [, string &errstr [,
 	 	 	 int flags = PTHREADS_STREAM_SERVER_BIND | PTHREADS_STREAM_SERVER_LISTEN [, StreamContext context ]]]]]) */
PHP_METHOD(SocketStream, createServer) {
	zend_string *local_socket = NULL;
	zval *zcontext = NULL, *errstr, *errorno = NULL;
	zend_long flags = PTHREADS_STREAM_XPORT_BIND | PTHREADS_STREAM_XPORT_LISTEN;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|z/z/lO", &local_socket, &errorno, &errstr, &flags, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_NULL();
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_create_server(local_socket, errorno, errstr, flags, zcontext, return_value);
} /* }}} */

#if HAVE_SOCKETPAIR
/* {{{ proto static array|null SocketStream::createPair(int domain, int type, int protocol) */
PHP_METHOD(SocketStream, createPair) {
	zend_long domain, type, protocol = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lll", &domain, &type, &protocol) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_pair(domain, type, protocol, return_value);
} /* }}} */
#endif

/* {{{ proto SocketStream|null SocketStream::accept([float timeout = ini_get("default_socket_timeout") [, string $peername ]]) */
PHP_METHOD(SocketStream, accept) {
	double timeout = (double)FG(default_socket_timeout);
	zval *zpeername = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|dz/", &timeout, &zpeername) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_accept(getThis(), timeout, zpeername, return_value);
} /* }}} */

/* {{{ proto string|null SocketStream::getName(bool want_peer) */
PHP_METHOD(SocketStream, getName) {
	zend_bool want_peer;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &want_peer) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_get_name(getThis(), want_peer, return_value);
} /* }}} */

/* {{{ proto string|null SocketStream::recvfrom(int length [, int flags = 0 [, string &$address ]]) */
PHP_METHOD(SocketStream, recvfrom) {
	zend_long length, flags = 0;
	zend_string *address = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|lz/", &length, &flags, &address) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_socket_stream_recvfrom(getThis(), length, flags, address, return_value);
} /* }}} */

/* {{{ proto int SocketStream::sendto(string data [, int flags = 0 [, string $address ]]) */
PHP_METHOD(SocketStream, sendto) {
	zend_long flags = 0;
	zend_string *data, *address = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lS", &data, &flags, &address) != SUCCESS) {
		RETURN_LONG(-1);
	}

	pthreads_streams_api_socket_stream_sendto(getThis(), data, flags, address, return_value);
} /* }}} */

#ifdef HAVE_SHUTDOWN
/* {{{ proto bool SocketStream::shutdown(int how) */
PHP_METHOD(SocketStream, shutdown) {
	zend_long how;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &how) != SUCCESS) {
		RETURN_FALSE;
	}

	if (how != PTHREADS_STREAM_SHUT_RD &&
	    how != PTHREADS_STREAM_SHUT_WR &&
	    how != PTHREADS_STREAM_SHUT_RDWR) {
		php_error_docref(NULL, E_WARNING, "Second parameter $how needs to be one of Streams::STREAM_SHUT_RD, Streams::STREAM_SHUT_WR or Streams::STREAM_SHUT_RDWR");
		RETURN_FALSE;
	}

	pthreads_streams_api_socket_stream_shutdown(getThis(), how, return_value);
} /* }}} */
#endif

#	endif
#endif
