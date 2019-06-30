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
#ifndef HAVE_PTHREADS_CLASS_FILTER_H
#define HAVE_PTHREADS_CLASS_FILTER_H
PHP_METHOD(StreamBucket, __construct);

/* {{{ */
static PHP_METHOD(StreamBucketBrigade, __construct) {
	zend_throw_error(NULL, "Instantiation of 'StreamBucketBrigade' is not allowed");
} /* }}} */
PHP_METHOD(StreamBucketBrigade, append);
PHP_METHOD(StreamBucketBrigade, prepend);
PHP_METHOD(StreamBucketBrigade, fetch);

/* {{{ */
static PHP_METHOD(StreamFilter, __construct) {
	zend_throw_error(NULL, "Instantiation of 'StreamFilter' is not allowed");
} /* }}} */
PHP_METHOD(StreamFilter, remove);

PHP_FUNCTION(user_filter_nop)
{
}

ZEND_BEGIN_ARG_INFO_EX(StreamBucket___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, buffer, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(pthreads_user_filter_class_filter, 0, 0, 4)
	ZEND_ARG_OBJ_INFO(0, in, StreamBucketBrigade, 0)
	ZEND_ARG_OBJ_INFO(0, out, StreamBucketBrigade, 0)
	ZEND_ARG_TYPE_INFO(1, consumed, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, closing, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(filters_noargs, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(StreamBucketBrigade_append, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, bucket, StreamBucket, 0)
	ZEND_ARG_TYPE_INFO(0, separate, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(StreamBucketBrigade_prepend, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, bucket, StreamBucket, 0)
	ZEND_ARG_TYPE_INFO(0, separate, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(StreamFilter_remove, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_streams_user_filter_class_methods[];
extern zend_function_entry pthreads_streams_bucket_methods[];
extern zend_function_entry pthreads_streams_bucketbrigade_methods[];
extern zend_function_entry pthreads_streams_filter_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_FILTER
#	define HAVE_PTHREADS_CLASS_FILTER
zend_function_entry pthreads_streams_user_filter_class_methods[] = {
	PHP_NAMED_FE(filter       , PHP_FN(user_filter_nop), pthreads_user_filter_class_filter)
	PHP_NAMED_FE(onCreate     , PHP_FN(user_filter_nop), filters_noargs)
	PHP_NAMED_FE(onClose      , PHP_FN(user_filter_nop), filters_noargs)
	PHP_FE_END
};

zend_function_entry pthreads_streams_bucket_methods[] = {
	PHP_ME(StreamBucket       , __construct , StreamBucket___construct      , ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_function_entry pthreads_streams_bucketbrigade_methods[] = {
	PHP_ME(StreamBucketBrigade, __construct , NULL                          , ZEND_ACC_PRIVATE)
	PHP_ME(StreamBucketBrigade, append      , StreamBucketBrigade_append    , ZEND_ACC_PUBLIC)
	PHP_ME(StreamBucketBrigade, prepend     , StreamBucketBrigade_prepend   , ZEND_ACC_PUBLIC)
	PHP_ME(StreamBucketBrigade, fetch       , filters_noargs                , ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_function_entry pthreads_streams_filter_methods[] = {
	PHP_ME(StreamFilter       , __construct , NULL                          , ZEND_ACC_PRIVATE)
	PHP_ME(StreamFilter       , remove      , StreamFilter_remove           , ZEND_ACC_PUBLIC)
	PHP_FE_END
};


/* {{{ proto StreamBucket::__construct(string buffer)
	Create a new bucket for use on the current stream  */
PHP_METHOD(StreamBucket, __construct) {
	zend_string *buffer;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &buffer) != SUCCESS) {
		RETURN_FALSE;
	}

	pthreads_streams_api_bucket_construct(getThis(), buffer, return_value);
} /* }}} */

/* {{{ proto void StreamBucketBrigade::append(Bucket bucket)
   Append bucket to brigade */
PHP_METHOD(StreamBucketBrigade, append) {
	zval *bucket = NULL;
	zend_bool separate = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|b", &bucket, pthreads_stream_bucket_entry, &separate) != SUCCESS) {
		RETURN_FALSE;
	}

	if (!instanceof_function(Z_OBJCE_P(bucket), pthreads_stream_bucket_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Bucket objects may be submitted, %s is no Bucket",
			ZSTR_VAL(Z_OBJCE_P(bucket)->name));
		return;
	}

	pthreads_streams_api_bucket_attach(getThis(), bucket, 1, separate);

} /* }}} */

/* {{{ proto void StreamBucketBrigade::prepend(Bucket bucket)
   Prepend bucket to brigade */
PHP_METHOD(StreamBucketBrigade, prepend) {
	zval *bucket = NULL;
	zend_bool separate = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|b", &bucket, pthreads_stream_bucket_entry, &separate) != SUCCESS) {
		RETURN_FALSE;
	}

	if (!instanceof_function(Z_OBJCE_P(bucket), pthreads_stream_bucket_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Bucket objects may be submitted, %s is no Bucket",
			ZSTR_VAL(Z_OBJCE_P(bucket)->name));
		return;
	}

	pthreads_streams_api_bucket_attach(getThis(), bucket, 0, separate);

} /* }}} */

/* {{{ proto Bucket StreamBucketBrigade::fetch(void)
   Return a bucket object from the brigade for operating on */
PHP_METHOD(StreamBucketBrigade, fetch) {

	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_bucket_fetch(getThis(), return_value);
} /* }}} */


/* {{{ proto bool StreamFilter::remove() */
PHP_METHOD(StreamFilter, remove) {
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	pthreads_streams_api_filter_remove(getThis(), return_value);
} /* }}} */

#	endif
#endif
