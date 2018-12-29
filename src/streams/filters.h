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
#ifndef HAVE_PTHREADS_STREAMS_FILTERS_H
#define HAVE_PTHREADS_STREAMS_FILTERS_H

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_BUCKETS_H
#	include <src/streams/buckets.h>
#endif

#define PTHREADS_STREAM_BUCKET_PROP_DATA	"data"
#define PTHREADS_STREAM_BUCKET_PROP_DATALEN "datalen"

#define PTHREADS_STREAM_FILTER_PROP_STREAM	"stream"
#define PTHREADS_STREAM_FILTER_PROP_FILTER	"filter"

#define PTHREADS_STREAM_FILTER_FUNC_FILTER		"filter"
#define PTHREADS_STREAM_FILTER_FUNC_ONCREATE	"oncreate"
#define PTHREADS_STREAM_FILTER_FUNC_ONCLOSE		"onclose"

#define PTHREADS_STREAM_FILTER_READ		0x0001
#define PTHREADS_STREAM_FILTER_WRITE	0x0002
#define PTHREADS_STREAM_FILTER_ALL		(PTHREADS_STREAM_FILTER_READ | PTHREADS_STREAM_FILTER_WRITE)


typedef enum {
	PTHREADS_SFS_ERR_FATAL,	/* error in data stream */
	PTHREADS_SFS_FEED_ME,	/* filter needs more data; stop processing chain until more is available */
	PTHREADS_SFS_PASS_ON	/* filter generated output buckets; pass them on to next in chain */
} pthreads_stream_filter_status_t;

#define PTHREADS_SFS_FLAG_NORMAL		0	/* regular read/write */
#define PTHREADS_SFS_FLAG_FLUSH_INC		1	/* an incremental flush */
#define PTHREADS_SFS_FLAG_FLUSH_CLOSE	2	/* final flush prior to closing */

struct pthreads_user_filter_data {
	/* variable length; this *must* be last in the structure */
	zend_string *classname;
};

typedef struct _pthreads_stream_filter_ops {

	pthreads_stream_filter_status_t (*filter)(
			pthreads_stream_t *threaded_stream,
			pthreads_stream_filter_t *threaded_thisfilter,
			pthreads_stream_bucket_brigade_t *threaded_buckets_in,
			pthreads_stream_bucket_brigade_t *threaded_buckets_out,
			size_t *bytes_consumed,
			int flags
			);

	void (*dtor)(pthreads_stream_filter_t *thisfilter);

	const char *label;

} pthreads_stream_filter_ops;

struct _pthreads_stream_filter {
	const pthreads_stream_filter_ops *fops;
	zval abstract; /* for use by filter implementation */
};

int pthreads_init_stream_filters();
int pthreads_shutdown_stream_filters();

/* stack filter onto a stream */
void _pthreads_stream_filter_prepend(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter);
int pthreads_stream_filter_prepend_ex(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter);
void _pthreads_stream_filter_append(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter);
int pthreads_stream_filter_append_ex(pthreads_stream_filter_chain_t *threaded_chain, pthreads_stream_filter_t *threaded_filter);
int _pthreads_stream_filter_flush(pthreads_stream_filter_t *threaded_filter, int finish);
pthreads_stream_filter_t *_pthreads_stream_filter_remove(pthreads_stream_filter_t *threaded_filter, int call_dtor);
#define pthreads_stream_filter_remove(threaded_filter) _pthreads_stream_filter_remove((threaded_filter), 1)
int _pthreads_stream_filter_is_integrated(pthreads_stream_filter_t *threaded_filter);
#define pthreads_stream_filter_is_integrated(threaded_filter) _pthreads_stream_filter_is_integrated((threaded_filter))

void pthreads_stream_filter_free(pthreads_stream_filter *filter, pthreads_stream_filter_t *threaded_filter);
pthreads_stream_filter *_pthreads_stream_filter_alloc(const pthreads_stream_filter_ops *fops, void *abstract);

#define pthreads_stream_filter_alloc(fops, thisptr) _pthreads_stream_filter_alloc((fops), (thisptr))
#define pthreads_stream_filter_prepend(chain, filter) _pthreads_stream_filter_prepend((chain), (filter))
#define pthreads_stream_filter_append(chain, filter) _pthreads_stream_filter_append((chain), (filter))
#define pthreads_stream_filter_flush(filter, finish) _pthreads_stream_filter_flush((filter), (finish))

#define pthreads_stream_is_filtered(threaded_stream)	(pthreads_chain_has_head(pthreads_stream_get_readfilters((threaded_stream))) \
		|| pthreads_chain_has_tail(pthreads_stream_get_writefilters((threaded_stream))))

typedef struct _pthreads_stream_filter_factory {
	pthreads_stream_filter_t *(*create_filter)(const char *filtername, zval *filterparams);
} pthreads_stream_filter_factory;

extern const pthreads_stream_filter_factory pthreads_user_filter_factory;

int pthreads_stream_filter_register_factory(const char *filterpattern, const pthreads_stream_filter_factory *factory);
int pthreads_stream_filter_unregister_factory(const char *filterpattern);
int pthreads_streams_add_user_filter_map_entry(zend_string *filtername, zend_string *classname);
void pthreads_streams_drop_user_filter_map_entry(zend_string *filtername);
pthreads_stream_filter_t *pthreads_stream_filter_create(const char *filtername, zval *filterparams);

pthreads_hashtable *_pthreads_get_stream_filters_hash(void);
#define pthreads_get_stream_filters_hash()	_pthreads_get_stream_filters_hash()

/* API */

void pthreads_streams_api_buffer_construct(zval *object, zend_string *buffer);

#endif
