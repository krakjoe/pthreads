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
#ifndef HAVE_PTHREADS_STREAMS_STRING_FILTERS
#define HAVE_PTHREADS_STREAMS_STRING_FILTERS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

/**
 * Rot13
 */

/* {{{ rot13 stream filter implementation */
static const char pthreads_rot13_from[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char pthreads_rot13_to[] = "nopqrstuvwxyzabcdefghijklmNOPQRSTUVWXYZABCDEFGHIJKLM";

static pthreads_stream_filter_status_t pthreads_strfilter_rot13_filter(
	pthreads_stream_t *threaded_stream,
	pthreads_stream_filter_t *threaded_thisfilter,
	pthreads_stream_bucket_brigade_t *threaded_buckets_in,
	pthreads_stream_bucket_brigade_t *threaded_buckets_out,
	size_t *bytes_consumed,
	int flags
	)
{
	pthreads_stream_bucket_t *threaded_bucket;
	pthreads_stream_bucket *bucket;
	size_t consumed = 0;

	while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_buckets_in)->head) != NULL) {
		bucket = pthreads_stream_bucket_fetch(threaded_bucket);

		php_strtr(bucket->buf, bucket->buflen, pthreads_rot13_from, pthreads_rot13_to, 52);
		consumed += bucket->buflen;

		pthreads_stream_bucket_append(threaded_buckets_out, threaded_bucket, 0);
	}

	if (bytes_consumed) {
		*bytes_consumed = consumed;
	}

	return PTHREADS_SFS_PASS_ON;
}

static const pthreads_stream_filter_ops pthreads_strfilter_rot13_ops = {
	pthreads_strfilter_rot13_filter,
	NULL,
	"string.rot13"
};

static pthreads_stream_filter_t *pthreads_strfilter_rot13_create(const char *filtername, zval *filterparams)
{
	return pthreads_stream_filter_new(&pthreads_strfilter_rot13_ops, NULL);
}

static const pthreads_stream_filter_factory pthreads_strfilter_rot13_factory = {
		pthreads_strfilter_rot13_create
};
/* }}} */

/**
 * toupper / tolower
 */

/* {{{ string.toupper / string.tolower stream filter implementation */
static const char pthreads_lowercase[] = "abcdefghijklmnopqrstuvwxyz";
static const char pthreads_uppercase[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static pthreads_stream_filter_status_t pthreads_strfilter_toupper_filter(
	pthreads_stream_t *threaded_stream,
	pthreads_stream_filter_t *threaded_thisfilter,
	pthreads_stream_bucket_brigade_t *threaded_buckets_in,
	pthreads_stream_bucket_brigade_t *threaded_buckets_out,
	size_t *bytes_consumed,
	int flags
	)
{
	pthreads_stream_bucket_t *threaded_bucket;
	pthreads_stream_bucket *bucket;
	size_t consumed = 0;

	while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_buckets_in)->head) != NULL) {
		bucket = pthreads_stream_bucket_fetch(threaded_bucket);

		php_strtr(bucket->buf, bucket->buflen, pthreads_lowercase, pthreads_uppercase, 26);
		consumed += bucket->buflen;

		pthreads_stream_bucket_append(threaded_buckets_out, threaded_bucket, 0);
	}

	if (bytes_consumed) {
		*bytes_consumed = consumed;
	}

	return PTHREADS_SFS_PASS_ON;
}

static pthreads_stream_filter_status_t pthreads_strfilter_tolower_filter(
	pthreads_stream_t *threaded_stream,
	pthreads_stream_filter_t *threaded_thisfilter,
	pthreads_stream_bucket_brigade_t *threaded_buckets_in,
	pthreads_stream_bucket_brigade_t *threaded_buckets_out,
	size_t *bytes_consumed,
	int flags
	)
{
	pthreads_stream_bucket_t *threaded_bucket;
	pthreads_stream_bucket *bucket;
	size_t consumed = 0;

	while ((threaded_bucket = PTHREADS_FETCH_STREAMS_BRIGADE(threaded_buckets_in)->head) != NULL) {
		bucket = pthreads_stream_bucket_fetch(threaded_bucket);

		php_strtr(bucket->buf, bucket->buflen, pthreads_uppercase, pthreads_lowercase, 26);
		consumed += bucket->buflen;

		pthreads_stream_bucket_append(threaded_buckets_out, threaded_bucket, 0);
	}

	if (bytes_consumed) {
		*bytes_consumed = consumed;
	}

	return PTHREADS_SFS_PASS_ON;
}

static const pthreads_stream_filter_ops pthreads_strfilter_toupper_ops = {
	pthreads_strfilter_toupper_filter,
	NULL,
	"string.toupper"
};

static const pthreads_stream_filter_ops pthreads_strfilter_tolower_ops = {
	pthreads_strfilter_tolower_filter,
	NULL,
	"string.tolower"
};

static pthreads_stream_filter_t *pthreads_strfilter_toupper_create(const char *filtername, zval *filterparams)
{
	return pthreads_stream_filter_new(&pthreads_strfilter_toupper_ops, NULL);
}

static pthreads_stream_filter_t *pthreads_strfilter_tolower_create(const char *filtername, zval *filterparams)
{
	return pthreads_stream_filter_new(&pthreads_strfilter_tolower_ops, NULL);
}

static const pthreads_stream_filter_factory pthreads_strfilter_toupper_factory = {
	pthreads_strfilter_toupper_create
};

static const pthreads_stream_filter_factory pthreads_strfilter_tolower_factory = {
	pthreads_strfilter_tolower_create
};
/* }}} */

#endif
