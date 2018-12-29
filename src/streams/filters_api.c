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
#ifndef HAVE_PTHREADS_API_FILTERS
#define HAVE_PTHREADS_API_FILTERS

/* {{{ pthreads_streams_api_buffer_construct */
void pthreads_streams_api_bucket_construct(zval *object, zend_string *buffer, zval *return_value) {
	pthreads_object_t *threaded =
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_bucket *bucket;

	bucket = pthreads_stream_bucket_alloc(ZSTR_VAL(buffer), ZSTR_LEN(buffer));

	if (bucket == NULL) {
		php_error_docref(NULL, E_WARNING, "Could not create StreamBucket");
		RETURN_FALSE;
	}
	PTHREADS_FETCH_STREAMS_BUCKET(threaded) = bucket;

	add_property_stringl(object, PTHREADS_STREAM_BUCKET_PROP_DATA, bucket->buf, bucket->buflen);
	add_property_long(object, PTHREADS_STREAM_BUCKET_PROP_DATALEN, bucket->buflen);

	Z_ADDREF_P(object);
}
/* }}} */

/* {{{ pthreads_streams_api_bucket_attach */
void pthreads_streams_api_bucket_attach(zval *object, zval *bucket_object, int append, int separate) {
	pthreads_stream_bucket_brigade_t *threaded_brigade = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_bucket_t *threaded_bucket = PTHREADS_FETCH_FROM(Z_OBJ_P(bucket_object));

	pthreads_stream_bucket_sync_properties(threaded_bucket);

	if (append) {
		pthreads_stream_bucket_append(threaded_brigade, threaded_bucket, separate);
	} else {
		pthreads_stream_bucket_prepend(threaded_brigade, threaded_bucket, separate);
	}
}
/* }}} */

/* {{{ pthreads_streams_api_bucket_fetch */
void pthreads_streams_api_bucket_fetch(zval *object, zval *return_value) {
	pthreads_stream_bucket_brigade_t *threaded_brigade = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_bucket_brigade *brigade = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_brigade);
	pthreads_stream_bucket_t *threaded_bucket = NULL;
	pthreads_stream_bucket *bucket;

	ZVAL_NULL(return_value);

	if(MONITOR_LOCK(threaded_brigade)) {
		if (brigade->head) {
			threaded_bucket = brigade->head;

			ZVAL_OBJ(return_value, PTHREADS_STD_P(threaded_bucket));
			Z_ADDREF_P(return_value);

			pthreads_stream_bucket_unlink(threaded_bucket);

			bucket = PTHREADS_FETCH_STREAMS_BUCKET(threaded_bucket);

			add_property_stringl(return_value, PTHREADS_STREAM_BUCKET_PROP_DATA, bucket->buf, bucket->buflen);
			add_property_long(return_value, PTHREADS_STREAM_BUCKET_PROP_DATALEN, bucket->buflen);
		}
		MONITOR_UNLOCK(threaded_brigade);
	}
}
/* }}} */

/* {{{ pthreads_streams_api_filter_remove */
void pthreads_streams_api_filter_remove(zval *object, zval *return_value) {
	pthreads_stream_filter_t *threaded_filter = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	// pthreads_stream_filter_remove() returns NULL in case of success!
	if(threaded_filter && !pthreads_stream_filter_remove(threaded_filter)) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

#endif
