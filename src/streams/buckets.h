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
#ifndef HAVE_PTHREADS_STREAMS_BUCKETS_H
#define HAVE_PTHREADS_STREAMS_BUCKETS_H

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

struct _pthreads_stream_bucket {
	pthreads_stream_bucket_t *next, *prev;
	pthreads_stream_bucket_brigade_t *brigade;

	char *buf;
	size_t buflen;
};

struct _pthreads_stream_bucket_brigade {
	pthreads_stream_bucket_t *head, *tail;
};

/* Buckets API. */
pthreads_stream_bucket_brigade *pthreads_stream_bucket_brigade_alloc();
void pthreads_stream_bucket_brigade_free(pthreads_stream_bucket_brigade *brigade);
void pthreads_stream_bucket_sync_properties(pthreads_stream_bucket_t *threaded_bucket);
pthreads_stream_bucket *pthreads_stream_bucket_alloc(char *buf, size_t buflen);
void pthreads_stream_bucket_free(pthreads_stream_bucket *bucket);
pthreads_stream_bucket *pthreads_stream_bucket_fetch(pthreads_stream_bucket_t *threaded_bucket);

void pthreads_stream_bucket_prepend(pthreads_stream_bucket_brigade_t *threaded_brigade, pthreads_stream_bucket_t *threaded_bucket, int separate);
void pthreads_stream_bucket_append(pthreads_stream_bucket_brigade_t *threaded_brigade, pthreads_stream_bucket_t *threaded_bucket, int separate);
void pthreads_stream_bucket_unlink(pthreads_stream_bucket_t *threaded_bucket);
void pthreads_stream_bucket_destroy(pthreads_stream_bucket_t *threaded_bucket);

#endif
