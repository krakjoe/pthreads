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
#ifndef HAVE_PTHREADS_STREAMS
#define HAVE_PTHREADS_STREAMS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_HASH_H
#	include <src/hash.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_INTERNAL_H
#	include <src/streams/internal.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER_H
#	include <src/streams/wrappers/fopen_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_MEMORY_H
#	include <src/streams/memory.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_NETWORK_H
#	include <src/network.h>
#endif

#ifndef FLOCK_COMPAT_H
#	include <ext/standard/flock_compat.h>
#endif

pthreads_streams_t* pthreads_streams_alloc(void) {
	return (pthreads_streams_t*) ecalloc(1, sizeof(pthreads_streams_t));
}

void pthreads_streams_free(pthreads_streams_t *streams) {
	efree(streams);
}

int pthreads_streams_aquire_double_lock(pthreads_object_t *object_one, pthreads_object_t *object_two) {
	if(!object_one) {
		return 0;
	}

	while(1) {
		if(pthreads_monitor_trylock(object_one->monitor)) {
			if(object_two) {
				if(!pthreads_monitor_trylock(object_two->monitor)) {
					pthreads_monitor_unlock(object_one->monitor);

					continue;
				}
			}
			return 1;
		}
	}
}

void pthreads_streams_release_double_lock(pthreads_object_t *object_one, pthreads_object_t *object_two) {
	if(object_one) {
		pthreads_monitor_unlock(object_one->monitor);
	}

	if(object_two) {
		pthreads_monitor_unlock(object_two->monitor);
	}
}

pthreads_stream_t *pthreads_stream_set_parent(pthreads_stream_t *threaded_target, pthreads_stream_t *threaded_parent) {
	pthreads_stream_t *orig = NULL;

	if(stream_lock(threaded_target)) {
		orig = pthreads_get_parent_stream(threaded_target);
		pthreads_set_parent_stream(threaded_target, threaded_parent);
		stream_unlock(threaded_target);
	}
	return orig;
}

void pthreads_init_streams() {
	INIT_GLOBAL_WRAPPER(stream_php_wrapper		, pthreads_stdio_wops				, NULL, 0);
	INIT_GLOBAL_WRAPPER(stream_rfc2397_wrapper	, pthreads_stream_rfc2397_wops		, NULL, 1);
	INIT_GLOBAL_WRAPPER(plain_files_wrapper		, pthreads_plain_files_wrapper_ops	, NULL, 0);
	INIT_GLOBAL_WRAPPER(glob_stream_wrapper		, pthreads_glob_stream_wrapper_ops	, NULL, 0);
	INIT_GLOBAL_WRAPPER(stream_http_wrapper		, pthreads_http_stream_wops			, NULL, 1);
	INIT_GLOBAL_WRAPPER(stream_ftp_wrapper		, pthreads_ftp_stream_wops			, NULL, 1);

	pthreads_init_stream_filters();
	pthreads_init_stream_wrappers();
	pthreads_init_stream_transports();

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_CONNECT")			, PTHREADS_STREAM_NOTIFY_CONNECT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_AUTH_REQUIRED")	, PTHREADS_STREAM_NOTIFY_AUTH_REQUIRED);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_AUTH_RESULT")		, PTHREADS_STREAM_NOTIFY_AUTH_RESULT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_MIME_TYPE_IS")	, PTHREADS_STREAM_NOTIFY_MIME_TYPE_IS);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_FILE_SIZE_IS")	, PTHREADS_STREAM_NOTIFY_FILE_SIZE_IS);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_REDIRECTED")		, PTHREADS_STREAM_NOTIFY_REDIRECTED);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_PROGRESS")		, PTHREADS_STREAM_NOTIFY_PROGRESS);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_FAILURE")			, PTHREADS_STREAM_NOTIFY_FAILURE);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_COMPLETED")		, PTHREADS_STREAM_NOTIFY_COMPLETED);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_RESOLVE")			, PTHREADS_STREAM_NOTIFY_RESOLVE);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_SEVERITY_INFO")	, PTHREADS_STREAM_NOTIFY_SEVERITY_INFO);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_SEVERITY_WARN")	, PTHREADS_STREAM_NOTIFY_SEVERITY_WARN);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_NOTIFY_SEVERITY_ERR")	, PTHREADS_STREAM_NOTIFY_SEVERITY_ERR);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_FILTER_READ")			, PTHREADS_STREAM_FILTER_READ);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_FILTER_WRITE")			, PTHREADS_STREAM_FILTER_WRITE);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_FILTER_ALL")				, PTHREADS_STREAM_FILTER_ALL);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CLIENT_ASYNC_CONNECT")	, PTHREADS_STREAM_CLIENT_ASYNC_CONNECT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CLIENT_CONNECT")			, PTHREADS_STREAM_CLIENT_CONNECT);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_ANY_CLIENT")		, PTHREADS_STREAM_CRYPTO_METHOD_ANY_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv2_CLIENT")		, PTHREADS_STREAM_CRYPTO_METHOD_SSLv2_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv3_CLIENT")		, PTHREADS_STREAM_CRYPTO_METHOD_SSLv3_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv23_CLIENT")	, PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLS_CLIENT")		, PTHREADS_STREAM_CRYPTO_METHOD_TLS_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_ANY_SERVER")		, PTHREADS_STREAM_CRYPTO_METHOD_ANY_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv2_SERVER")		, PTHREADS_STREAM_CRYPTO_METHOD_SSLv2_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv3_SERVER")		, PTHREADS_STREAM_CRYPTO_METHOD_SSLv3_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_SSLv23_SERVER")	, PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLS_SERVER")		, PTHREADS_STREAM_CRYPTO_METHOD_TLS_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_0_SERVER")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_0_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_1_SERVER")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_1_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_METHOD_TLSv1_2_SERVER")	, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_2_SERVER);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_PROTO_SSLv3")				, PTHREADS_STREAM_CRYPTO_METHOD_SSLv3_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_PROTO_TLSv1_0")			, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_0_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_PROTO_TLSv1_1")			, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_1_SERVER);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_CRYPTO_PROTO_TLSv1_2")			, PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_2_SERVER);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SHUT_RD")	, PTHREADS_STREAM_SHUT_RD);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SHUT_WR")	, PTHREADS_STREAM_SHUT_WR);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SHUT_RDWR")	, PTHREADS_STREAM_SHUT_RDWR);

#ifdef PF_INET
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_INET"), PF_INET);
#elif defined(AF_INET)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_INET"), AF_INET);
#endif

#if HAVE_IPV6
# ifdef PF_INET6
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_INET6"), PF_INET6);
# elif defined(AF_INET6)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_INET6"), AF_INET6);
# endif
#endif

#ifdef PF_UNIX
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_UNIX"), PF_UNIX);
#elif defined(AF_UNIX)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PF_UNIX"), AF_UNIX);
#endif

#ifdef IPPROTO_IP
	/* most people will use this one when calling socket() or socketpair() */
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_IPPROTO_IP"), IPPROTO_IP);
#endif

#if defined(IPPROTO_TCP) || defined(PHP_WIN32)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_IPPROTO_TCP"), IPPROTO_TCP);
#endif

#if defined(IPPROTO_UDP) || defined(PHP_WIN32)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_IPPROTO_UDP"), IPPROTO_UDP);
#endif

#if defined(IPPROTO_ICMP) || defined(PHP_WIN32)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_IPPROTO_ICMP"), IPPROTO_ICMP);
#endif

#if defined(IPPROTO_RAW) || defined(PHP_WIN32)
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_IPPROTO_RAW"), IPPROTO_RAW);
#endif

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SOCK_STREAM"), SOCK_STREAM);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SOCK_DGRAM"), SOCK_DGRAM);

#ifdef SOCK_RAW
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SOCK_RAW"), SOCK_RAW);
#endif

#ifdef SOCK_SEQPACKET
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SOCK_SEQPACKET"), SOCK_SEQPACKET);
#endif

#ifdef SOCK_RDM
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SOCK_RDM"), SOCK_RDM);
#endif

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_PEEK"), PTHREADS_STREAM_PEEK);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_OOB"), PTHREADS_STREAM_OOB);

	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SERVER_BIND"), PTHREADS_STREAM_XPORT_BIND);
	zend_declare_class_constant_long(pthreads_streams_entry, ZEND_STRL("STREAM_SERVER_LISTEN"), PTHREADS_STREAM_XPORT_LISTEN);
}

void pthreads_shutdown_streams() {
	pthreads_shutdown_stream_wrappers();
	pthreads_shutdown_stream_filters();
}

zend_bool stream_lock(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	int locked;

	if(!stream) {
		PTHREADS_STREAM_CORRUPTED();
	}
	PTHREADS_STREAM_PRE_CHECK(stream);
	//printf("stream_lock requested (%i) \n", (ulong) pthread_self());
	locked = MONITOR_LOCK(threaded_stream);

	//printf("gathered stream_lock (%i) \n", (ulong) pthread_self());
	if(locked) {
		PTHREADS_STREAM_POST_CHECK(threaded_stream, stream);
	}
	return locked;
}

pthreads_stream_t *_pthreads_stream_new(const pthreads_stream_ops *ops, void *abstract, const char *mode, zend_class_entry *stream_ce) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	pthreads_hashtable *streams_list;
	zend_string *str;

	if(stream_ce == NULL) {
		stream_ce = pthreads_stream_entry;
	}

	if(!instanceof_function(stream_ce, pthreads_stream_entry)) {
		php_error_docref(NULL, E_WARNING, "Class \"%s\" must be an instance of Stream", ZSTR_VAL(stream_ce->name));
		return NULL;
	}
	threaded_stream = pthreads_object_init(stream_ce);
	stream = pthreads_stream_alloc(ops, abstract, mode);

	pthreads_stream_set_readfilters(threaded_stream, pthreads_object_init(pthreads_volatile_map_entry));
	pthreads_stream_set_writefilters(threaded_stream, pthreads_object_init(pthreads_volatile_map_entry));

	pthreads_chain_set_stream(pthreads_stream_get_readfilters(threaded_stream), threaded_stream);

	/* Unwanted addref by pthreads_chain_set_stream */
	pthreads_ptr_dtor(threaded_stream);

	pthreads_chain_set_stream(pthreads_stream_get_writefilters(threaded_stream), threaded_stream);

	/* Unwanted addref by pthreads_chain_set_stream */
	pthreads_ptr_dtor(threaded_stream);

	PTHREADS_FETCH_STREAMS_STREAM(threaded_stream) = stream;

	pthreads_set_parent_stream(threaded_stream, NULL);
	pthreads_stream_set_context(threaded_stream, NULL);

	return threaded_stream;
}

pthreads_stream_filter_t *pthreads_stream_filter_new(const pthreads_stream_filter_ops *fops, void *abstract) {
	pthreads_object_t *threaded_filter = pthreads_stream_filter_init();
	PTHREADS_FETCH_STREAMS_FILTER(threaded_filter) = pthreads_stream_filter_alloc(fops, abstract);

	return threaded_filter;
}

pthreads_stream_filter_t *pthreads_stream_filter_init() {
	return pthreads_object_init(pthreads_stream_filter_entry);
}

pthreads_stream_bucket_t *pthreads_stream_bucket_new(char *buf, size_t buflen) {
	pthreads_object_t *threaded_bucket = pthreads_stream_bucket_init();
	PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket) = pthreads_stream_bucket_alloc(buf, buflen);

	return threaded_bucket;
}

pthreads_stream_bucket_t *pthreads_stream_bucket_init() {
	return pthreads_object_init(pthreads_stream_bucket_entry);
}

pthreads_stream_bucket_brigade_t *pthreads_stream_bucket_brigade_new() {
	return pthreads_object_init(pthreads_stream_brigade_entry);
}

pthreads_stream_wrapper_t *pthreads_stream_wrapper_new() {
	return pthreads_object_init(pthreads_stream_wrapper_entry);
}

pthreads_stream_context_t *pthreads_stream_context_new() {
	return pthreads_object_init(pthreads_stream_context_entry);
}

int pthreads_stream_has_threaded_property(pthreads_object_t *threaded, int property) {
	return pthreads_stream_read_threaded_property(threaded, property) == NULL ? 0 : 1;
}

int pthreads_stream_count_threaded_properties(pthreads_object_t *threaded) {
	zval obj;
	zend_long count;
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));

	pthreads_store_count(&obj, &count);

	return count;
}

pthreads_object_t *pthreads_stream_read_threaded_property(pthreads_object_t *threaded, int property) {
	pthreads_object_t * result = NULL;
	zval key, read, obj;

	ZVAL_NULL(&read);
	ZVAL_LONG(&key, property);
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));

	if(_pthreads_store_read(&obj, &key, 0, &read, !(EG(flags) & EG_FLAGS_IN_SHUTDOWN)) == SUCCESS && !ZVAL_IS_NULL(&read)) {
		result = PTHREADS_FETCH_FROM(Z_OBJ(read));

		if(IS_PTHREADS_OBJECT(&read)) {
			zval_ptr_dtor(&read);
		}
	}
	return result;
}

int pthreads_stream_write_threaded_property(pthreads_object_t *threaded, int property, pthreads_object_t *val) {
	zval key, write, obj;

	if(val == NULL) {
		return pthreads_stream_delete_threaded_property(threaded, property);
	}
	ZVAL_LONG(&key, property);
	ZVAL_OBJ(&write, PTHREADS_STD_P(val));
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));

	return _pthreads_store_write(&obj, &key, &write, !(EG(flags) & EG_FLAGS_IN_SHUTDOWN));
}

int pthreads_stream_delete_threaded_property(pthreads_object_t *threaded, int property) {
	zval key, obj;
	ZVAL_LONG(&key, property);
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));

	return _pthreads_store_delete(&obj, &key, !(EG(flags) & EG_FLAGS_IN_SHUTDOWN));
}

/* {{{ context API */

void pthreads_stream_notification_notify(pthreads_stream_context_t *threaded_context, int notifycode, int severity,
		char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr)
{
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			context->notifier->func(context, notifycode, severity, xmsg, xcode, bytes_sofar, bytes_max, ptr);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

pthreads_stream_context *pthreads_stream_context_alloc(void)
{
	pthreads_stream_context *context;

	context = calloc(1, sizeof(pthreads_stream_context));
	context->notifier = NULL;
	context->options = pthreads_object_init(pthreads_volatile_map_entry);

	return context;
}

void pthreads_stream_context_free(pthreads_stream_context *context) {
	if (context->options != NULL) {
		zval options;
		ZVAL_OBJ(&options, PTHREADS_STD_P(context->options));
		zval_ptr_dtor(&options);
		context->options = NULL;
	}
	if (context->notifier) {
		pthreads_stream_notification_free(context->notifier);
		context->notifier = NULL;
	}
	free(context);
}

pthreads_stream_notifier *pthreads_stream_notification_alloc(void) {
	return calloc(1, sizeof(pthreads_stream_notifier));
}

void pthreads_stream_notification_free(pthreads_stream_notifier *notifier) {
	if (notifier->dtor) {
		notifier->dtor(notifier);
	}
	free(notifier);
}

pthreads_stream_context_t *pthreads_stream_context_set(pthreads_stream_t *threaded_stream, pthreads_stream_context_t *threaded_context) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_context_t *oldcontext = pthreads_stream_get_context(threaded_stream);

	pthreads_stream_set_context(threaded_stream, threaded_context);

	if (oldcontext) {
		zval ctx;
		ZVAL_OBJ(&ctx, PTHREADS_STD_P(oldcontext));
		Z_DELREF(ctx);
	}

	return oldcontext;
}

zval *pthreads_stream_context_get_option(pthreads_stream_context_t *threaded_context, const char *wrappername, const char *optionname) {
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);
	pthreads_object_t *threaded_options = NULL;
	zval *wrapperhash, *result;
	zval rv, ret, options;

	ZVAL_NULL(&rv);
	ZVAL_NULL(&ret);

	if (MONITOR_LOCK(threaded_context)) {
		threaded_options = context->options;
		ZVAL_OBJ(&options, PTHREADS_STD_P(threaded_options));

		wrapperhash = zend_read_property(pthreads_volatile_entry, &options, wrappername, strlen(wrappername), 1, &rv);

		if (NULL == wrapperhash || Z_ISNULL_P(wrapperhash)) {
			result = NULL;
		} else {
			result = zend_read_property(pthreads_volatile_entry, wrapperhash, optionname, strlen(optionname), 1, &ret);

			if (NULL != result && Z_ISNULL_P(result)) {
				result = NULL;
			}
		}
		MONITOR_UNLOCK(threaded_context);
	}
	return result;
}

int pthreads_stream_context_set_option(pthreads_stream_context_t *threaded_context, const char *wrappername, const char *optionname, zval *optionvalue) {
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);
	pthreads_object_t *threaded_options = NULL;
	zval *wrapperhash, *category;
	zval rv, options;

	ZVAL_NULL(&rv);

	if (MONITOR_LOCK(threaded_context)) {
		threaded_options = context->options;
		ZVAL_OBJ(&options, PTHREADS_STD_P(threaded_options));

		wrapperhash = zend_read_property(pthreads_volatile_map_entry, &options, wrappername, strlen(wrappername), 1, &rv);

		if (NULL == wrapperhash || Z_ISNULL_P(wrapperhash)) {
			ZVAL_OBJ(wrapperhash, PTHREADS_STD_P(pthreads_object_init(pthreads_volatile_map_entry)));
			zend_update_property(pthreads_volatile_map_entry, &options, (char*)wrappername, strlen(wrappername), wrapperhash);
		}
		ZVAL_DEREF(optionvalue);
		Z_TRY_ADDREF_P(optionvalue);
		zend_update_property(pthreads_volatile_map_entry, wrapperhash, optionname, strlen(optionname), optionvalue);

		MONITOR_UNLOCK(threaded_context);
	}
	return SUCCESS;
}

void pthreads_stream_notify_info(pthreads_stream_context_t *threaded_context, int code, char *xmsg, int xcode) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			pthreads_stream_notification_notify(threaded_context, code, PTHREADS_STREAM_NOTIFY_SEVERITY_INFO, xmsg, xcode, 0, 0, NULL);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

void pthreads_stream_notify_progress(pthreads_stream_context_t *threaded_context, size_t bsofar, size_t bmax) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			pthreads_stream_notification_notify(threaded_context, PTHREADS_STREAM_NOTIFY_PROGRESS, PTHREADS_STREAM_NOTIFY_SEVERITY_INFO, NULL, 0, bsofar, bmax, NULL);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

void pthreads_stream_notify_progress_init(pthreads_stream_context_t *threaded_context, size_t sofar, size_t bmax) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			context->notifier->progress = sofar;
			context->notifier->progress_max = bmax;
			context->notifier->mask |= PTHREADS_STREAM_NOTIFIER_PROGRESS;

			pthreads_stream_notify_progress(threaded_context, sofar, bmax);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

void pthreads_stream_notify_progress_increment(pthreads_stream_context_t *threaded_context, size_t sofar, size_t max) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier && context->notifier->mask & PTHREADS_STREAM_NOTIFIER_PROGRESS) {
			context->notifier->progress += sofar;
			context->notifier->progress_max += max;

			pthreads_stream_notify_progress(threaded_context, context->notifier->progress, context->notifier->progress_max);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

void pthreads_stream_notify_file_size(pthreads_stream_context_t *threaded_context, size_t file_size, char *xmsg, int xcode) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			pthreads_stream_notification_notify(threaded_context, PTHREADS_STREAM_NOTIFY_FILE_SIZE_IS, PTHREADS_STREAM_NOTIFY_SEVERITY_INFO, xmsg, xcode, 0, file_size, NULL);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}

void pthreads_stream_notify_error(pthreads_stream_context_t *threaded_context, int code, char *xmsg, int xcode) {
	pthreads_stream_context *context;

	if(!threaded_context) {
		return;
	}
	context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	if(MONITOR_LOCK(threaded_context)) {
		if (context && context->notifier) {
			pthreads_stream_notification_notify(threaded_context, code, PTHREADS_STREAM_NOTIFY_SEVERITY_ERR, xmsg, xcode, 0, 0, NULL);
		}
		MONITOR_UNLOCK(threaded_context);
	}
}
/* }}} */

/* {{{ allocate a new stream for a particular ops */
pthreads_stream *_pthreads_stream_alloc(const pthreads_stream_ops *ops, void *abstract, const char *mode)
{
	pthreads_stream *ret = (pthreads_stream*) pemalloc(sizeof(pthreads_stream), 1);

	memset(ret, 0, sizeof(pthreads_stream));

#if STREAM_DEBUG
fprintf(stderr, "stream_alloc: %s:%p\n", ops->label, ret);
#endif

	ret->state = PTHREADS_STREAM_STATE_OPEN;
	ret->ops = ops;
	ret->abstract = abstract;
	ret->chunk_size = FG(def_chunk_size);

	if (FG(auto_detect_line_endings)) {
		ret->flags |= PTHREADS_STREAM_FLAG_DETECT_EOL;
	}

	strlcpy(ret->mode, mode, sizeof(ret->mode));

	ret->wrapper          = NULL;
	ret->wrapperthis      = NULL;
	ret->stdiocast        = NULL;
	ret->orig_path        = NULL;
	ret->readbuf          = NULL;

	return ret;
}
/* }}} */

/* {{{ */
int _pthreads_stream_close_ignore_parent(pthreads_stream_t *threaded_stream_enclosed, int close_options) {
	return pthreads_stream_close(threaded_stream_enclosed,
		close_options | PTHREADS_STREAM_FREE_IGNORE_PARENT);
}
/* }}} */

#if PTHREADS_STREAM_DEBUG
static const char *_pthreads_stream_pretty_free_options(int close_options, char *out) {
	if (close_options & PTHREADS_STREAM_FREE_CALL_DTOR)
		strcat(out, "CALL_DTOR, ");
	if (close_options & PTHREADS_STREAM_FREE_PRESERVE_HANDLE)
		strcat(out, "PREVERSE_HANDLE, ");
	if (close_options & PTHREADS_STREAM_FREE_IGNORE_PARENT)
		strcat(out, "IGNORE_PARENT, ");
	if (out[0] != '\0')
		out[strlen(out) - 2] = '\0';
	return out;
}
#endif

/* {{{ */
int _pthreads_stream_close(pthreads_stream_t *threaded_stream, int close_options, int skip_check) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *parent_stream;
	pthreads_hashtable *streams_list;
	zend_string *str;
	int ret = 1;
	int preserve_handle = close_options & PTHREADS_STREAM_FREE_PRESERVE_HANDLE ? 1 : 0;

	if(skip_check ? MONITOR_LOCK(threaded_stream) : stream_lock(threaded_stream)) {
		stream->state |= PTHREADS_STREAM_STATE_CLOSING;

		if (stream->flags & PTHREADS_STREAM_FLAG_NO_CLOSE) {
			preserve_handle = 1;
		}
		stream->preserve_handle = preserve_handle;

#if PTHREADS_STREAM_DEBUG
		{
			char out[200] = "";
			fprintf(stderr, "stream_close: %s:%p[%s] in_free=%d opts=%s\n",
				stream->ops->label, stream, stream->orig_path, stream->in_free, _pthreads_stream_pretty_free_options(close_options, out));
		}

#endif

		if (stream->in_free) {
			/* hopefully called recursively from the enclosing stream; the pointer was NULLed below */
			stream_unlock(threaded_stream);
			return 1; /* recursion protection */
		}
		stream->in_free++;

		parent_stream = pthreads_get_parent_stream(threaded_stream);

		/* force correct order on enclosing/enclosed stream destruction */
		if (parent_stream != NULL && !PTHREADS_IS_STREAM_CLOSING(PTHREADS_FETCH_STREAMS_STREAM(parent_stream)) &&
				threaded_stream != parent_stream && !(close_options & PTHREADS_STREAM_FREE_IGNORE_PARENT) &&
				(close_options & PTHREADS_STREAM_FREE_CALL_DTOR)) { /* always? */
			pthreads_delete_parent_stream(stream);

			/* remove closing state, otherwise ops->close() can not close this innerstream */
			stream->state &= ~PTHREADS_STREAM_STATE_CLOSING;

			ret = pthreads_stream_close(parent_stream,
							(close_options | PTHREADS_STREAM_FREE_CALL_DTOR));

			/**
			 * Scenario (innerstream gets closed before linked parent stream)
			 * 1. pthreads_stream_close(innerstream)
			 * 2. innerstream has parent
			 * 3. innerstream -> pthreads_stream_close(parent_stream)
			 * 4. parent_stream -> ops->close()
			 * 5. ops->close() -> pthreads_stream_close_ignore_parent(innerstream)
			 * 6. close innerstream properly
			 */

			stream_unlock(threaded_stream);
			/* we force PTHREADS_STREAM_CALL_DTOR because that's from where the
			 * enclosing stream can free this stream. */
			return ret;
		}

		/* if we are releasing the stream only (and preserving the underlying handle),
		 * we need to do things a little differently.
		 * We are only ever called like this when the stream is cast to a FILE*
		 * for include (or other similar) purposes.
		 * */
		if (preserve_handle) {
			if (stream->fclose_stdiocast == PTHREADS_STREAM_FCLOSE_FOPENCOOKIE) {
				/* If the stream was fopencookied, we must NOT touch anything
				 * here, as the cookied stream relies on it all.
				 * Instead, mark the stream as OK to auto-clean */
				stream->in_free--;
				stream->state = PTHREADS_STREAM_STATE_CLOSED;

				stream_unlock(threaded_stream);
				return 0;
			}
			/* otherwise, make sure that we don't close the FILE* from a cast */
		}

#if PTHREADS_STREAM_DEBUG
fprintf(stderr, "stream_close: %s:%p[%s] preserve_handle=%d\n",
		stream->ops->label, stream, stream->orig_path, preserve_handle);
#endif

		if (stream->flags & PTHREADS_STREAM_FLAG_WAS_WRITTEN) {
			/* make sure everything is saved */
			_pthreads_stream_flush(threaded_stream, 1);
		}

		if (close_options & PTHREADS_STREAM_FREE_CALL_DTOR) {
			if (!preserve_handle && stream->fclose_stdiocast == PTHREADS_STREAM_FCLOSE_FOPENCOOKIE) {
				/* calling fclose on an fopencookied stream will ultimately
					call this very same function.  If we were called via fclose,
					the cookie_closer unsets the fclose_stdiocast flags, so
					we can be sure that we only reach here when PHP code calls
					pthreads_stream_close.
					Lets let the cookie code clean it all up.
				 */
				stream->in_free = 0;

				ret = fclose(stream->stdiocast);

				stream_unlock(threaded_stream);
				return ret;
			}
			ret = stream->ops->close(threaded_stream, preserve_handle ? 0 : 1);

			/* tidy up any FILE* that might have been fdopened */
			if (!preserve_handle && stream->fclose_stdiocast == PTHREADS_STREAM_FCLOSE_FDOPEN && stream->stdiocast) {
				fclose(stream->stdiocast);
				stream->stdiocast = NULL;
				stream->fclose_stdiocast = PTHREADS_STREAM_FCLOSE_NONE;
			}
		}
		stream->state |= PTHREADS_STREAM_STATE_CLOSED;
		stream_unlock(threaded_stream);

		zval sobj;
		ZVAL_OBJ(&sobj, PTHREADS_STD_P(threaded_stream));
		zval_ptr_dtor(&sobj);
	}

	return ret;
}
/* }}} */

/* {{{ */
void _pthreads_stream_free(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_context_t *threaded_context = NULL;
	pthreads_stream_filter_t *threaded_filter = NULL;
	pthreads_stream_wrapper *wrapper = NULL;

	if(stream && MONITOR_LOCK(threaded_stream)) {

#if PTHREADS_STREAM_DEBUG
fprintf(stderr, "stream_free: %s:%p[%s] preserve_handle=%d\n",
		stream->ops->label, stream, stream->orig_path, stream->preserve_handle);
#endif
		stream->ops->free(threaded_stream, stream->preserve_handle ? 0 : 1);
		stream->abstract = NULL;

		threaded_context = pthreads_stream_get_context(threaded_stream);

		pthreads_chain_set_stream(pthreads_stream_get_readfilters(threaded_stream), NULL);
		pthreads_chain_set_stream(pthreads_stream_get_writefilters(threaded_stream), NULL);

		while ((threaded_filter = pthreads_chain_get_head(pthreads_stream_get_readfilters(threaded_stream))) != NULL) {
			pthreads_stream_filter_remove(threaded_filter);
		}

		while ((threaded_filter = pthreads_chain_get_head(pthreads_stream_get_writefilters(threaded_stream))) != NULL) {
			pthreads_stream_filter_remove(threaded_filter);
		}
		pthreads_ptr_dtor(pthreads_stream_get_readfilters(threaded_stream));
		pthreads_ptr_dtor(pthreads_stream_get_writefilters(threaded_stream));

		pthreads_stream_set_readfilters(threaded_stream, NULL);
		pthreads_stream_set_writefilters(threaded_stream, NULL);

		if(stream->wrapper) {
			wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(stream->wrapper);
			if (wrapper && wrapper->wops && wrapper->wops->stream_closer) {
				wrapper->wops->stream_closer(stream->wrapper, threaded_stream);
				stream->wrapper = NULL;
			}
		}

		if (pthreads_stream_has_wrapperdata(threaded_stream)) {
			pthreads_ptr_dtor(pthreads_stream_get_wrapperdata(threaded_stream));
			pthreads_stream_set_wrapperdata(threaded_stream, NULL);
		}

		if (stream->readbuf) {
			free(stream->readbuf);
			stream->readbuf = NULL;
		}

		if (stream->orig_path) {
			free(stream->orig_path);
			stream->orig_path = NULL;
		}

		if(threaded_context) {
			if(pthreads_is_default_context(threaded_context)) {
				pthreads_add_ref(threaded_context);
			}
			pthreads_stream_delete_context(threaded_stream);
		}

		free(stream);

		PTHREADS_FETCH_STREAMS_STREAM(threaded_stream) = NULL;

		MONITOR_UNLOCK(threaded_stream);
	}
}
/* }}} */

/* {{{ generic stream operations */
void _pthreads_stream_fill_read_buffer(pthreads_stream_t *threaded_stream, size_t size) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	/* allocate/fill the buffer */

	if(stream_lock(threaded_stream)) {
		if (pthreads_chain_has_head(pthreads_stream_get_readfilters(threaded_stream))) {
			char *chunk_buf;
			int err_flag = 0;
			pthreads_stream_bucket_brigade_t *brig_inp = pthreads_stream_bucket_brigade_new(),
					*brig_outp = pthreads_stream_bucket_brigade_new(), *brig_swap;

			/* Invalidate the existing cache, otherwise reads can fail, see note in
			   src/streams/filters.c::_pthreads_stream_filter_append */
			stream->writepos = stream->readpos = 0;

			/* allocate a buffer for reading chunks */
			chunk_buf = emalloc(stream->chunk_size);

			while (!stream->eof && !err_flag && (stream->writepos - stream->readpos < (zend_off_t)size)) {
				size_t justread = 0;
				int flags;
				pthreads_stream_bucket_t *threaded_bucket;
				pthreads_stream_filter_t *threaded_filter;
				pthreads_stream_filter_status_t status = PTHREADS_SFS_ERR_FATAL;
				pthreads_stream_bucket *bucket;

				/* read a chunk into a bucket */
				justread = stream->ops->read(threaded_stream, chunk_buf, stream->chunk_size);
				if (justread && justread != (size_t)-1) {
					threaded_bucket = pthreads_stream_bucket_new(chunk_buf, justread);

					/* after this call, bucket is owned by the brigade */
					pthreads_stream_bucket_append(brig_inp, threaded_bucket, 0);

					flags = PTHREADS_SFS_FLAG_NORMAL;
				} else {
					flags = stream->eof ? PTHREADS_SFS_FLAG_FLUSH_CLOSE : PTHREADS_SFS_FLAG_FLUSH_INC;
				}

				/* wind the handle... */
				for (threaded_filter = pthreads_chain_get_head(pthreads_stream_get_readfilters(threaded_stream)); threaded_filter; threaded_filter = pthreads_filter_get_next(threaded_filter)) {
					status = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter)->fops->filter(threaded_stream, threaded_filter, brig_inp, brig_outp, NULL, flags);

					if (status != PTHREADS_SFS_PASS_ON) {
						break;
					}

					/* brig_out becomes brig_in.
					 * brig_in will always be empty here, as the filter MUST attach any un-consumed buckets
					 * to its own brigade */
					brig_swap = brig_inp;
					brig_inp = brig_outp;
					brig_outp = brig_swap;
					memset(PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp), 0, sizeof(*PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp)));
				}

				switch (status) {
					case PTHREADS_SFS_PASS_ON:
						/* we get here when the last filter in the chain has data to pass on.
						 * in this situation, we are passing the brig_in brigade into the
						 * stream read buffer */
						while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(brig_inp)->head) != NULL) {
							pthreads_stream_bucket_sync_properties(threaded_bucket);
							bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);
							/* grow buffer to hold this bucket
							 * TODO: this can fail for persistent streams */
							if (stream->readbuflen - stream->writepos < bucket->buflen) {
								stream->readbuflen += bucket->buflen;
								stream->readbuf = realloc(stream->readbuf, stream->readbuflen);
							}
							memcpy(stream->readbuf + stream->writepos, bucket->buf, bucket->buflen);
							stream->writepos += bucket->buflen;

							pthreads_stream_bucket_destroy(threaded_bucket);
						}
						break;

					case PTHREADS_SFS_FEED_ME:
						/* when a filter needs feeding, there is no brig_out to deal with.
						 * we simply continue the loop; if the caller needs more data,
						 * we will read again, otherwise out job is done here */
						if (justread == 0) {
							/* there is no data */
							err_flag = 1;
							break;
						}
						continue;

					case PTHREADS_SFS_ERR_FATAL:
						/* some fatal error. Theoretically, the stream is borked, so all
						 * further reads should fail. */
						err_flag = 1;
						break;
				}

				if (justread == 0 || justread == (size_t)-1) {
					break;
				}
			}
			pthreads_ptr_dtor(brig_inp);
			pthreads_ptr_dtor(brig_outp);

			efree(chunk_buf);

		} else {
			/* is there enough data in the buffer ? */
			if (stream->writepos - stream->readpos < (zend_off_t)size) {
				size_t justread = 0;

				/* reduce buffer memory consumption if possible, to avoid a realloc */
				if (stream->readbuf && stream->readbuflen - stream->writepos < stream->chunk_size) {
					if (stream->writepos > stream->readpos) {
						memmove(stream->readbuf, stream->readbuf + stream->readpos, stream->writepos - stream->readpos);
					}
					stream->writepos -= stream->readpos;
					stream->readpos = 0;
				}

				/* grow the buffer if required
				 * TODO: this can fail for persistent streams */
				if (stream->readbuflen - stream->writepos < stream->chunk_size) {
					stream->readbuflen += stream->chunk_size;
					stream->readbuf = realloc(stream->readbuf, stream->readbuflen);
				}

				justread = stream->ops->read(threaded_stream, (char*)stream->readbuf + stream->writepos,
						stream->readbuflen - stream->writepos
						);

				if (justread != (size_t)-1) {
					stream->writepos += justread;
				}
			}
		}
		stream_unlock(threaded_stream);
	}
}

size_t _pthreads_stream_read(pthreads_stream_t *threaded_stream, char *buf, size_t size) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t toread = 0, didread = 0;

	if(stream_lock(threaded_stream)) {
		while (size > 0) {

			/* take from the read buffer first.
			 * It is possible that a buffered stream was switched to non-buffered, so we
			 * drain the remainder of the buffer before using the "raw" read mode for
			 * the excess */
			if (stream->writepos > stream->readpos) {

				toread = stream->writepos - stream->readpos;
				if (toread > size) {
					toread = size;
				}

				memcpy(buf, stream->readbuf + stream->readpos, toread);
				stream->readpos += toread;
				size -= toread;
				buf += toread;
				didread += toread;
			}

			/* ignore eof here; the underlying state might have changed */
			if (size == 0) {
				break;
			}

			if (!pthreads_chain_has_head(pthreads_stream_get_readfilters(threaded_stream)) && (stream->flags & PTHREADS_STREAM_FLAG_NO_BUFFER || stream->chunk_size == 1)) {
				toread = stream->ops->read(threaded_stream, buf, size);
				if (toread == (size_t) -1) {
					/* e.g. underlying read(2) returned -1 */
					break;
				}
			} else {
				pthreads_stream_fill_read_buffer(threaded_stream, size);

				toread = stream->writepos - stream->readpos;
				if (toread > size) {
					toread = size;
				}

				if (toread > 0) {
					memcpy(buf, stream->readbuf + stream->readpos, toread);
					stream->readpos += toread;
				}
			}
			if (toread > 0) {
				didread += toread;
				buf += toread;
				size -= toread;
			} else {
				/* EOF, or temporary end of data (for non-blocking mode). */
				break;
			}

			/* just break anyway, to avoid greedy read for file://, php://memory, and php://temp */
			if ((stream->wrapper != PTHREADS_STREAMG(plain_files_wrapper)) &&
				(stream->ops != &pthreads_stream_memory_ops) &&
				(stream->ops != &pthreads_stream_temp_ops)) {
				break;
			}
		}

		if (didread > 0) {
			stream->position += didread;
		}
		stream_unlock(threaded_stream);
	}

	return didread;
}

int _pthreads_stream_eof(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		/* if there is data in the buffer, it's not EOF */
		if (stream->writepos - stream->readpos > 0) {
			stream_unlock(threaded_stream);
			return 0;
		}

		/* use the configured timeout when checking eof */
		if (!stream->eof && PTHREADS_STREAM_OPTION_RETURN_ERR ==
				pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_CHECK_LIVENESS,
				0, NULL)) {
			stream->eof = 1;
		}
		stream_unlock(threaded_stream);
	}
	return stream->eof;
}

int _pthreads_stream_putc(pthreads_stream_t *threaded_stream, int c) {
	unsigned char buf = c;

	if (pthreads_stream_write(threaded_stream, (char*)&buf, 1) > 0) {
		return 1;
	}
	return EOF;
}

int _pthreads_stream_getc(pthreads_stream_t *threaded_stream) {
	char buf;

	if (pthreads_stream_read(threaded_stream, &buf, 1) > 0) {
		return buf & 0xff;
	}
	return EOF;
}

int _pthreads_stream_puts(pthreads_stream_t *threaded_stream, const char *buf) {
	size_t len;
	char newline[2] = "\n"; /* is this OK for Win? */
	len = strlen(buf);

	if(stream_lock(threaded_stream)) {
		if (len > 0 && pthreads_stream_write(threaded_stream, buf, len) && pthreads_stream_write(threaded_stream, newline, 1)) {
			stream_unlock(threaded_stream);
			return 1;
		}
		stream_unlock(threaded_stream);
	}
	return 0;
}

int _pthreads_stream_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_wrapper *wrapper = NULL;

	memset(ssb, 0, sizeof(*ssb));

	if(stream_lock(threaded_stream)) {
		/* if the stream was wrapped, allow the wrapper to stat it */
		if(stream->wrapper) {
			wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(stream->wrapper);

			if (wrapper && wrapper->wops->stream_stat != NULL) {
				stream_unlock(threaded_stream);

				return wrapper->wops->stream_stat(stream->wrapper, threaded_stream, ssb);
			}
		}

		/* if the stream doesn't directly support stat-ing, return with failure.
		 * We could try and emulate this by casting to a FD and fstat-ing it,
		 * but since the fd might not represent the actual underlying content
		 * this would give bogus results. */
		if (stream->ops->stat == NULL) {
			stream_unlock(threaded_stream);
			return -1;
		}
		stream_unlock(threaded_stream);
	}
	return (stream->ops->stat)(threaded_stream, ssb);
}

const char *pthreads_stream_locate_eol(pthreads_stream_t *threaded_stream, zend_string *buf) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t avail;
	const char *cr, *lf, *eol = NULL;
	const char *readptr;

	if(stream_lock(threaded_stream)) {
		if (!buf) {
			readptr = (char*)stream->readbuf + stream->readpos;
			avail = stream->writepos - stream->readpos;
		} else {
			readptr = ZSTR_VAL(buf);
			avail = ZSTR_LEN(buf);
		}

		/* Look for EOL */
		if (stream->flags & PTHREADS_STREAM_FLAG_DETECT_EOL) {
			cr = memchr(readptr, '\r', avail);
			lf = memchr(readptr, '\n', avail);

			if (cr && lf != cr + 1 && !(lf && lf < cr)) {
				/* mac */
				stream->flags ^= PTHREADS_STREAM_FLAG_DETECT_EOL;
				stream->flags |= PTHREADS_STREAM_FLAG_EOL_MAC;
				eol = cr;
			} else if ((cr && lf && cr == lf - 1) || (lf)) {
				/* dos or unix endings */
				stream->flags ^= PTHREADS_STREAM_FLAG_DETECT_EOL;
				eol = lf;
			}
		} else if (stream->flags & PTHREADS_STREAM_FLAG_EOL_MAC) {
			eol = memchr(readptr, '\r', avail);
		} else {
			/* unix (and dos) line endings */
			eol = memchr(readptr, '\n', avail);
		}
		stream_unlock(threaded_stream);
	}
	return eol;
}

/* If buf == NULL, the buffer will be allocated automatically and will be of an
 * appropriate length to hold the line, regardless of the line length, memory
 * permitting */
char *_pthreads_stream_get_line(pthreads_stream_t *threaded_stream, char *buf, size_t maxlen, size_t *returned_len) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t avail = 0;
	size_t current_buf_size = 0;
	size_t total_copied = 0;
	int grow_mode = 0;
	char *bufstart = buf;

	if (buf == NULL) {
		grow_mode = 1;
	} else if (maxlen == 0) {
		return NULL;
	}

	/*
	 * If the underlying stream operations block when no new data is readable,
	 * we need to take extra precautions.
	 *
	 * If there is buffered data available, we check for a EOL. If it exists,
	 * we pass the data immediately back to the caller. This saves a call
	 * to the read implementation and will not block where blocking
	 * is not necessary at all.
	 *
	 * If the stream buffer contains more data than the caller requested,
	 * we can also avoid that costly step and simply return that data.
	 */
	if(stream_lock(threaded_stream)) {
		for (;;) {
			avail = stream->writepos - stream->readpos;

			if (avail > 0) {
				size_t cpysz = 0;
				char *readptr;
				const char *eol;
				int done = 0;

				readptr = (char*)stream->readbuf + stream->readpos;
				eol = pthreads_stream_locate_eol(threaded_stream, NULL);

				if (eol) {
					cpysz = eol - readptr + 1;
					done = 1;
				} else {
					cpysz = avail;
				}

				if (grow_mode) {
					/* allow room for a NUL. If this realloc is really a realloc
					 * (ie: second time around), we get an extra byte. In most
					 * cases, with the default chunk size of 8K, we will only
					 * incur that overhead once.  When people have lines longer
					 * than 8K, we waste 1 byte per additional 8K or so.
					 * That seems acceptable to me, to avoid making this code
					 * hard to follow */
					bufstart = realloc(bufstart, current_buf_size + cpysz + 1);
					current_buf_size += cpysz + 1;
					buf = bufstart + total_copied;
				} else {
					if (cpysz >= maxlen - 1) {
						cpysz = maxlen - 1;
						done = 1;
					}
				}

				memcpy(buf, readptr, cpysz);

				stream->position += cpysz;
				stream->readpos += cpysz;
				buf += cpysz;
				maxlen -= cpysz;
				total_copied += cpysz;

				if (done) {
					break;
				}
			} else if (stream->eof) {
				break;
			} else {
				/* XXX: Should be fine to always read chunk_size */
				size_t toread;

				if (grow_mode) {
					toread = stream->chunk_size;
				} else {
					toread = maxlen - 1;
					if (toread > stream->chunk_size) {
						toread = stream->chunk_size;
					}
				}

				pthreads_stream_fill_read_buffer(threaded_stream, toread);

				if (stream->writepos - stream->readpos == 0) {
					break;
				}
			}
		}
		stream_unlock(threaded_stream);
	}

	if (total_copied == 0) {
		if (grow_mode) {
			assert(bufstart == NULL);
		}
		return NULL;
	}

	buf[0] = '\0';
	if (returned_len) {
		*returned_len = total_copied;
	}

	return bufstart;
}

#define PTHREADS_STREAM_BUFFERED_AMOUNT(stream) \
	((size_t)(((stream)->writepos) - (stream)->readpos))

static const char *_pthreads_stream_search_delim(	pthreads_stream_t *threaded_stream,
													size_t maxlen,
													size_t skiplen,
													const char *delim, /* non-empty! */
													size_t delim_len) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t	seek_len;

	/* set the maximum number of bytes we're allowed to read from buffer */
	seek_len = MIN(PTHREADS_STREAM_BUFFERED_AMOUNT(stream), maxlen);
	if (seek_len <= skiplen) {
		return NULL;
	}
	const char * result = NULL;

	if (delim_len == 1) {
		if(stream_lock(threaded_stream)) {
			result = memchr(&stream->readbuf[stream->readpos + skiplen], delim[0], seek_len - skiplen);
			stream_unlock(threaded_stream);
		}
	} else {
		if(stream_lock(threaded_stream)) {
			result = php_memnstr((char*)&stream->readbuf[stream->readpos + skiplen], delim, delim_len,
				(char*)&stream->readbuf[stream->readpos + seek_len]);
			stream_unlock(threaded_stream);
		}
	}
	return result;
}

zend_string *pthreads_stream_get_record(pthreads_stream_t *threaded_stream, size_t maxlen, const char *delim, size_t delim_len) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zend_string	*ret_buf;				/* returned buffer */
	const char *found_delim = NULL;
	size_t	buffered_len,
			tent_ret_len;			/* tentative returned length */
	int	has_delim = delim_len > 0;

	if (maxlen == 0) {
		return NULL;
	}

	if(stream_lock(threaded_stream)) {
		if (has_delim) {
			found_delim = _pthreads_stream_search_delim(threaded_stream, maxlen, 0, delim, delim_len);
		}

		buffered_len = PTHREADS_STREAM_BUFFERED_AMOUNT(stream);
		/* try to read up to maxlen length bytes while we don't find the delim */
		while (!found_delim && buffered_len < maxlen) {
			size_t	just_read,
					to_read_now;

			to_read_now = MIN(maxlen - buffered_len, stream->chunk_size);

			pthreads_stream_fill_read_buffer(threaded_stream, buffered_len + to_read_now);

			just_read = PTHREADS_STREAM_BUFFERED_AMOUNT(stream) - buffered_len;

			/* Assume the stream is temporarily or permanently out of data */
			if (just_read == 0) {
				break;
			}

			if (has_delim) {
				/* search for delimiter, but skip buffered_len (the number of bytes
				 * buffered before this loop iteration), as they have already been
				 * searched for the delimiter.
				 * The left part of the delimiter may still remain in the buffer,
				 * so subtract up to <delim_len - 1> from buffered_len, which is
				 * the amount of data we skip on this search  as an optimization
				 */
				found_delim = _pthreads_stream_search_delim(
					threaded_stream, maxlen,
					buffered_len >= (delim_len - 1)
							? buffered_len - (delim_len - 1)
							: 0,
					delim, delim_len);
				if (found_delim) {
					break;
				}
			}
			buffered_len += just_read;
		}

		if (has_delim && found_delim) {
			tent_ret_len = found_delim - (char*)&stream->readbuf[stream->readpos];
		} else if (!has_delim && PTHREADS_STREAM_BUFFERED_AMOUNT(stream) >= maxlen) {
			tent_ret_len = maxlen;
		} else {
			/* return with error if the delimiter string (if any) was not found, we
			 * could not completely fill the read buffer with maxlen bytes and we
			 * don't know we've reached end of file. Added with non-blocking streams
			 * in mind, where this situation is frequent */
			if (PTHREADS_STREAM_BUFFERED_AMOUNT(stream) < maxlen && !stream->eof) {
				stream_unlock(threaded_stream);
				return NULL;
			} else if (PTHREADS_STREAM_BUFFERED_AMOUNT(stream) == 0 && stream->eof) {
				/* refuse to return an empty string just because by accident
				 * we knew of EOF in a read that returned no data */
				stream_unlock(threaded_stream);
				return NULL;
			} else {
				tent_ret_len = MIN(PTHREADS_STREAM_BUFFERED_AMOUNT(stream), maxlen);
			}
		}

		ret_buf = zend_string_alloc(tent_ret_len, 0);
		/* pthreads_stream_read will not call ops->read here because the necessary
		 * data is guaranteedly buffered */
		ZSTR_LEN(ret_buf) = pthreads_stream_read(threaded_stream, ZSTR_VAL(ret_buf), tent_ret_len);

		if (found_delim) {
			stream->readpos += delim_len;
			stream->position += delim_len;
		}
		stream_unlock(threaded_stream);
	}
	ZSTR_VAL(ret_buf)[ZSTR_LEN(ret_buf)] = '\0';
	return ret_buf;
}

/* Writes a buffer directly to a stream, using multiple of the chunk size */
static size_t _pthreads_stream_write_buffer(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	size_t didwrite = 0, towrite, justwrote;

	if(stream_lock(threaded_stream)) {
		/* if we have a seekable stream we need to ensure that data is written at the
		 * current stream->position. This means invalidating the read buffer and then
		 * performing a low-level seek */
		if (stream->ops->seek && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0 && stream->readpos != stream->writepos) {
			stream->readpos = stream->writepos = 0;

			stream->ops->seek(threaded_stream, stream->position, SEEK_SET, &stream->position);
		}

		while (count > 0) {
			towrite = count;
			if (towrite > stream->chunk_size)
				towrite = stream->chunk_size;

			justwrote = stream->ops->write(threaded_stream, buf, towrite);

			/* convert justwrote to an integer, since normally it is unsigned */
			if ((int)justwrote > 0) {
				buf += justwrote;
				count -= justwrote;
				didwrite += justwrote;

				/* Only screw with the buffer if we can seek, otherwise we lose data
				 * buffered from fifos and sockets */
				if (stream->ops->seek && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0) {
					stream->position += justwrote;
				}
			} else {
				break;
			}
		}
		stream_unlock(threaded_stream);
	}
	return didwrite;

}

/* push some data through the write filter chain.
 * buf may be NULL, if flags are set to indicate a flush.
 * This may trigger a real write to the stream.
 * Returns the number of bytes consumed from buf by the first filter in the chain.
 * */
static size_t _pthreads_stream_write_filtered(pthreads_stream_t *threaded_stream, const char *buf, size_t count, int flags) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t consumed = 0;
	pthreads_stream_bucket *bucket;
	pthreads_stream_bucket_t *threaded_bucket;
	pthreads_stream_bucket_brigade_t *brig_inp = pthreads_stream_bucket_brigade_new(),
			*brig_outp = pthreads_stream_bucket_brigade_new(), *brig_swap;
	pthreads_stream_filter_status_t status = PTHREADS_SFS_ERR_FATAL;
	pthreads_stream_filter_t *threaded_filter;
	pthreads_stream_filter *filter;

	if (buf) {
		threaded_bucket = pthreads_stream_bucket_new((char *)buf, count);
		pthreads_stream_bucket_append(brig_inp, threaded_bucket, 0);
	}

	if(stream_lock(threaded_stream)) {
		for (threaded_filter = pthreads_chain_get_head(pthreads_stream_get_writefilters(threaded_stream)); threaded_filter; threaded_filter = pthreads_filter_get_next(threaded_filter)) {
			filter = PTHREADS_FETCH_STREAMS_FILTER(threaded_filter);
			/* for our return value, we are interested in the number of bytes consumed from
			 * the first filter in the chain */

			status = filter->fops->filter(threaded_stream, threaded_filter, brig_inp, brig_outp,
					!pthreads_object_compare(threaded_filter, pthreads_chain_get_head(pthreads_stream_get_writefilters(threaded_stream))) ? &consumed : NULL, flags);


			if (status != PTHREADS_SFS_PASS_ON) {
				break;
			}
			/* brig_out becomes brig_in.
			 * brig_in will always be empty here, as the filter MUST attach any un-consumed buckets
			 * to its own brigade */
			brig_swap = brig_inp;
			brig_inp = brig_outp;
			brig_outp = brig_swap;
			memset(PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp), 0, sizeof(*PTHREADS_FETCH_STREAMS_BRIGADE(brig_outp)));
		}

		switch (status) {
			case PTHREADS_SFS_PASS_ON:
				/* filter chain generated some output; push it through to the
				 * underlying stream */
				while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(brig_inp)->head) != NULL) {
					pthreads_stream_bucket_sync_properties(threaded_bucket);
					bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);
					_pthreads_stream_write_buffer(threaded_stream, bucket->buf, bucket->buflen);
					/* Potential error situation - eg: no space on device. Perhaps we should keep this brigade
					 * hanging around and try to write it later.
					 * At the moment, we just drop it on the floor
					 * */
					pthreads_stream_bucket_destroy(threaded_bucket);
				}
				break;
			case PTHREADS_SFS_FEED_ME:
				/* need more data before we can push data through to the stream */
				pthreads_stream_bucket_destroy(threaded_bucket);
				break;

			case PTHREADS_SFS_ERR_FATAL:
				/* some fatal error.  Theoretically, the stream is borked, so all
				 * further writes should fail. */
				pthreads_stream_bucket_destroy(threaded_bucket);
				break;
		}
		stream_unlock(threaded_stream);
	}
	pthreads_ptr_dtor(brig_inp);
	pthreads_ptr_dtor(brig_outp);

	return consumed;
}

int _pthreads_stream_flush(pthreads_stream_t *threaded_stream, int closing) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	int ret = 0;

	if(stream_lock(threaded_stream)) {
		if (pthreads_chain_has_head(pthreads_stream_get_writefilters(threaded_stream))) {
			_pthreads_stream_write_filtered(threaded_stream, NULL, 0, closing ? PTHREADS_SFS_FLAG_FLUSH_CLOSE : PTHREADS_SFS_FLAG_FLUSH_INC );
		}

		stream->flags &= ~PTHREADS_STREAM_FLAG_WAS_WRITTEN;

		if (stream->ops->flush) {
			ret = stream->ops->flush(threaded_stream);
		}
		stream_unlock(threaded_stream);
	}
	return ret;
}

size_t _pthreads_stream_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	size_t bytes;

	if (buf == NULL || count == 0 || stream->ops->write == NULL) {
		return 0;
	}

	if(stream_lock(threaded_stream)) {
		if (pthreads_chain_has_head(pthreads_stream_get_writefilters(threaded_stream))) {
			bytes = _pthreads_stream_write_filtered(threaded_stream, buf, count, PTHREADS_SFS_FLAG_NORMAL);
		} else {
			bytes = _pthreads_stream_write_buffer(threaded_stream, buf, count);
		}

		if (bytes) {
			stream->flags |= PTHREADS_STREAM_FLAG_WAS_WRITTEN;
		}
		stream_unlock(threaded_stream);
	}
	return bytes;
}

size_t _pthreads_stream_printf(pthreads_stream_t *threaded_stream, const char *fmt, ...) {
	size_t count;
	char *buf;
	va_list ap;

	va_start(ap, fmt);
	count = vspprintf(&buf, 0, fmt, ap);
	va_end(ap);

	if (!buf) {
		return 0; /* error condition */
	}

	count = pthreads_stream_write(threaded_stream, buf, count);
	efree(buf);

	return count;
}

zend_off_t _pthreads_stream_tell(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zend_off_t result;

	if(stream_lock(threaded_stream)) {
		result = stream->position;
		stream_unlock(threaded_stream);
	}
	return result;
}

int _pthreads_stream_seek(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		if (stream->fclose_stdiocast == PTHREADS_STREAM_FCLOSE_FOPENCOOKIE) {
			/* flush to commit data written to the fopencookie FILE* */
			fflush(stream->stdiocast);
		}

		/* handle the case where we are in the buffer */
		if ((stream->flags & PTHREADS_STREAM_FLAG_NO_BUFFER) == 0) {
			switch(whence) {
				case SEEK_CUR:
					if (offset > 0 && offset <= stream->writepos - stream->readpos) {
						stream->readpos += offset; /* if offset = ..., then readpos = writepos */
						stream->position += offset;
						stream->eof = 0;

						stream_unlock(threaded_stream);
						return 0;
					}
					break;
				case SEEK_SET:
					if (offset > stream->position &&
							offset <= stream->position + stream->writepos - stream->readpos) {
						stream->readpos += offset - stream->position;
						stream->position = offset;
						stream->eof = 0;

						stream_unlock(threaded_stream);
						return 0;
					}
					break;
			}
		}

		if (stream->ops->seek && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0) {
			int ret;

			if (pthreads_chain_has_head(pthreads_stream_get_writefilters(threaded_stream))) {
				_pthreads_stream_flush(threaded_stream, 0);
			}

			switch(whence) {
				case SEEK_CUR:
					offset = stream->position + offset;
					whence = SEEK_SET;
					break;
			}
			ret = stream->ops->seek(threaded_stream, offset, whence, &stream->position);

			if (((stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0) || ret == 0) {
				if (ret == 0) {
					stream->eof = 0;
				}

				/* invalidate the buffer contents */
				stream->readpos = stream->writepos = 0;
				stream_unlock(threaded_stream);

				return ret;
			}
			/* else the stream has decided that it can't support seeking after all;
			 * fall through to attempt emulation */
		}

		/* emulate forward moving seeks with reads */
		if (whence == SEEK_CUR && offset >= 0) {
			char tmp[1024];
			size_t didread;
			while(offset > 0) {
				if ((didread = pthreads_stream_read(threaded_stream, tmp, MIN(offset, sizeof(tmp)))) == 0) {
					stream_unlock(threaded_stream);
					return -1;
				}
				offset -= didread;
			}
			stream->eof = 0;
			stream_unlock(threaded_stream);
			return 0;
		}
		stream_unlock(threaded_stream);
	}
	php_error_docref(NULL, E_WARNING, "stream does not support seeking");

	return -1;
}

int _pthreads_stream_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	int ret = PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;

	if(stream_lock(threaded_stream)) {
		if (stream->ops->set_option) {
			ret = stream->ops->set_option(threaded_stream, option, value, ptrparam);
		}

		if (ret == PTHREADS_STREAM_OPTION_RETURN_NOTIMPL) {
			switch(option) {
				case PTHREADS_STREAM_OPTION_SET_CHUNK_SIZE:
					/* XXX chunk size itself is of size_t, that might be ok or not for a particular case*/
					ret = stream->chunk_size > INT_MAX ? INT_MAX : (int)stream->chunk_size;
					stream->chunk_size = value;
					stream_unlock(threaded_stream);
					return ret;

				case PTHREADS_STREAM_OPTION_READ_BUFFER:
					/* try to match the buffer mode as best we can */
					if (value == PTHREADS_STREAM_BUFFER_NONE) {
						stream->flags |= PTHREADS_STREAM_FLAG_NO_BUFFER;
					} else if (stream->flags & PTHREADS_STREAM_FLAG_NO_BUFFER) {
						stream->flags ^= PTHREADS_STREAM_FLAG_NO_BUFFER;
					}
					ret = PTHREADS_STREAM_OPTION_RETURN_OK;
					break;

				default:
					;
			}
		}
		stream_unlock(threaded_stream);
	}
	return ret;
}

int _pthreads_stream_truncate_set_size(pthreads_stream_t *threaded_stream, size_t newsize) {
	return pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_TRUNCATE_API, PTHREADS_STREAM_TRUNCATE_SET_SIZE, &newsize);
}

size_t _pthreads_stream_passthru(pthreads_stream_t * threaded_stream) {
	size_t bcount = 0;
	char buf[8192];
	size_t b;

	if(stream_lock(threaded_stream)) {
		if (pthreads_stream_mmap_possible(threaded_stream)) {
			char *p;
			size_t mapped;

			p = pthreads_stream_mmap_range(threaded_stream, pthreads_stream_tell(threaded_stream), PTHREADS_STREAM_MMAP_ALL, PTHREADS_STREAM_MAP_MODE_SHARED_READONLY, &mapped);

			if (p) {
				do {
					/* output functions return int, so pass in int max */
					if (0 < (b = PHPWRITE(p + bcount, MIN(mapped - bcount, INT_MAX)))) {
						bcount += b;
					}
				} while (b > 0 && mapped > bcount);

				pthreads_stream_mmap_unmap_ex(threaded_stream, mapped);

				return bcount;
			}
		}

		while ((b = pthreads_stream_read(threaded_stream, buf, sizeof(buf))) > 0) {
			PHPWRITE(buf, b);
			bcount += b;
		}
		stream_unlock(threaded_stream);
	}
	return bcount;
}

zend_string *_pthreads_stream_copy_to_mem(pthreads_stream_t *threaded_src, size_t maxlen, int persistent) {
	size_t ret = 0;
	char *ptr;
	size_t len = 0, max_len;
	int step = CHUNK_SIZE;
	int min_room = CHUNK_SIZE / 4;
	pthreads_stream_statbuf ssbuf;
	zend_string *result;

	if (maxlen == 0) {
		return ZSTR_EMPTY_ALLOC();
	}

	if (maxlen == PTHREADS_STREAM_COPY_ALL) {
		maxlen = 0;
	}

	if(stream_lock(threaded_src)) {
		if (maxlen > 0) {
			result = zend_string_alloc(maxlen, persistent);
			ptr = ZSTR_VAL(result);
			while ((len < maxlen) && !pthreads_stream_eof(threaded_src)) {
				ret = pthreads_stream_read(threaded_src, ptr, maxlen - len);
				if (!ret) {
					break;
				}
				len += ret;
				ptr += ret;
			}
			if (len) {
				*ptr = '\0';
				ZSTR_LEN(result) = len;
			} else {
				zend_string_free(result);
				result = NULL;
			}
			stream_unlock(threaded_src);
			return result;
		}

		/* avoid many reallocs by allocating a good sized chunk to begin with, if
		 * we can.  Note that the stream may be filtered, in which case the stat
		 * result may be inaccurate, as the filter may inflate or deflate the
		 * number of bytes that we can read.  In order to avoid an upsize followed
		 * by a downsize of the buffer, overestimate by the step size (which is
		 * 2K).  */
		if (pthreads_stream_stat(threaded_src, &ssbuf) == 0 && ssbuf.sb.st_size > 0) {
			max_len = ssbuf.sb.st_size + step;
		} else {
			max_len = step;
		}

		result = zend_string_alloc(max_len, persistent);
		ptr = ZSTR_VAL(result);

		while ((ret = pthreads_stream_read(threaded_src, ptr, max_len - len)))	{
			len += ret;
			if (len + min_room >= max_len) {
				result = zend_string_extend(result, max_len + step, persistent);
				max_len += step;
				ptr = ZSTR_VAL(result) + len;
			} else {
				ptr += ret;
			}
		}
		stream_unlock(threaded_src);
	}

	if (len) {
		result = zend_string_truncate(result, len, persistent);
		ZSTR_VAL(result)[len] = '\0';
	} else {
		zend_string_free(result);
		result = NULL;
	}

	return result;
}

/* Returns SUCCESS/FAILURE and sets *len to the number of bytes moved */
int _pthreads_stream_copy_to_stream_ex(pthreads_stream_t *threaded_src, pthreads_stream_t *threaded_dest, size_t maxlen, size_t *len) {
	char buf[CHUNK_SIZE];
	size_t readchunk;
	size_t haveread = 0;
	size_t didread, didwrite, towrite;
	size_t dummy;
	pthreads_stream_statbuf ssbuf;
	int eof;

	if (!len) {
		len = &dummy;
	}

	if (maxlen == 0) {
		*len = 0;
		return SUCCESS;
	}

	if (maxlen == PTHREADS_STREAM_COPY_ALL) {
		maxlen = 0;
	}

	if (pthreads_stream_stat(threaded_src, &ssbuf) == 0) {
		if (ssbuf.sb.st_size == 0
#ifdef S_ISREG
			&& S_ISREG(ssbuf.sb.st_mode)
#endif
		) {
			*len = 0;
			return SUCCESS;
		}
	}

	if(pthreads_streams_aquire_double_lock(threaded_src, threaded_dest)) {
		if (pthreads_stream_mmap_possible(threaded_src)) {
			char *p;
			size_t mapped;

			p = pthreads_stream_mmap_range(threaded_src, pthreads_stream_tell(threaded_src), maxlen, PTHREADS_STREAM_MAP_MODE_SHARED_READONLY, &mapped);

			if (p) {
				didwrite = pthreads_stream_write(threaded_dest, p, mapped);

				pthreads_stream_mmap_unmap_ex(threaded_src, mapped);

				*len = didwrite;

				/* we've got at least 1 byte to read
				 * less than 1 is an error
				 * AND read bytes match written */
				if (mapped > 0 && mapped == didwrite) {
					pthreads_streams_release_double_lock(threaded_src, threaded_dest);
					return SUCCESS;
				}
				pthreads_streams_release_double_lock(threaded_src, threaded_dest);
				return FAILURE;
			}
		}

		while(1) {
			readchunk = sizeof(buf);

			if (maxlen && (maxlen - haveread) < readchunk) {
				readchunk = maxlen - haveread;
			}

			didread = pthreads_stream_read(threaded_src, buf, readchunk);

			if (didread) {
				/* extra paranoid */
				char *writeptr;

				towrite = didread;
				writeptr = buf;
				haveread += didread;

				while(towrite) {
					didwrite = pthreads_stream_write(threaded_dest, writeptr, towrite);
					if (didwrite == 0) {
						*len = haveread - (didread - towrite);
						pthreads_streams_release_double_lock(threaded_src, threaded_dest);
						return FAILURE;
					}

					towrite -= didwrite;
					writeptr += didwrite;
				}
			} else {
				break;
			}

			if (maxlen - haveread == 0) {
				break;
			}
		}
		eof = PTHREADS_FETCH_STREAMS_STREAM(threaded_src)->eof;

		pthreads_streams_release_double_lock(threaded_src, threaded_dest);
	}
	*len = haveread;

	/* we've got at least 1 byte to read.
	 * less than 1 is an error */

	if (haveread > 0 || eof) {
		return SUCCESS;
	}
	return FAILURE;
}


/* {{{ pthreads_stream_scandir */
int _pthreads_stream_scandir(const char *dirname, zend_string **namelist[], int flags, pthreads_stream_context_t *threaded_context,
			  int (*compare) (const zend_string **a, const zend_string **b)) {
	pthreads_stream_t *threaded_stream;
	pthreads_stream_dirent sdp;
	zend_string **vector = NULL;
	unsigned int vector_size = 0;
	unsigned int nfiles = 0;

	if (!namelist) {
		return FAILURE;
	}

	if(MONITOR_LOCK(threaded_context)) {
		threaded_stream = pthreads_stream_opendir(dirname, PTHREADS_REPORT_ERRORS, threaded_context);
		if (!threaded_stream) {
			return FAILURE;
		}
		if(stream_lock(threaded_stream)) {
			while (pthreads_stream_readdir(threaded_stream, &sdp)) {
				if (nfiles == vector_size) {
					if (vector_size == 0) {
						vector_size = 10;
					} else {
						if(vector_size*2 < vector_size) {
							/* overflow */
							pthreads_stream_closedir(threaded_stream);
							free(vector);
							return FAILURE;
						}
						vector_size *= 2;
					}
					vector = (zend_string **) realloc(vector, zend_safe_address_guarded(vector_size, sizeof(char *), 0));
				}

				vector[nfiles] = zend_string_init(sdp.d_name, strlen(sdp.d_name), 1);

				nfiles++;
				if(vector_size < 10 || nfiles == 0) {
					/* overflow */
					pthreads_stream_closedir(threaded_stream);
					free(vector);
					return FAILURE;
				}
			}
			pthreads_stream_closedir(threaded_stream);

			stream_unlock(threaded_stream);
		}
		MONITOR_UNLOCK(threaded_context);
	}

	*namelist = vector;

	if (nfiles > 0 && compare) {
		qsort(*namelist, nfiles, sizeof(zend_string *), (int(*)(const void *, const void *))compare);
	}
	return nfiles;
}
/* }}} */

#ifndef HAVE_PTHREADS_API_CONTEXT
#	include "src/streams/context_api.c"
#endif

#ifndef HAVE_PTHREADS_STREAMS_STREAMS_API
#	include "src/streams/streams_api.c"
#endif

#endif
