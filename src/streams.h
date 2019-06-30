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
#ifndef HAVE_PTHREADS_STREAMS_H
#define HAVE_PTHREADS_STREAMS_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

#ifndef FILE_H
#	include <ext/standard/file.h>
#endif

#ifndef HAVE_PTHREADS_UTILS_H
#	include <src/utils.h>
#endif

typedef struct _pthreads_stream                 pthreads_stream;
typedef struct _pthreads_stream_wrapper         pthreads_stream_wrapper;
typedef struct _pthreads_stream_context         pthreads_stream_context;
typedef struct _pthreads_stream_filter          pthreads_stream_filter;
typedef struct _pthreads_stream_bucket          pthreads_stream_bucket;
typedef struct _pthreads_stream_bucket_brigade	pthreads_stream_bucket_brigade;

typedef struct _pthreads_object_t pthreads_stream_t;
typedef struct _pthreads_object_t pthreads_stream_wrapper_t;
typedef struct _pthreads_object_t pthreads_stream_context_t;
typedef struct _pthreads_object_t pthreads_stream_filter_t;
typedef struct _pthreads_object_t pthreads_stream_bucket_t;
typedef struct _pthreads_object_t pthreads_stream_bucket_brigade_t;

typedef struct _pthreads_object_t pthreads_stream_filter_chain_t;

#define PTHREADS_FETCH_STREAMS_STREAM(object)  ((object)->store.streams->stream)
#define PTHREADS_FETCH_STREAMS_CONTEXT(object) ((object)->store.streams->context)
#define PTHREADS_FETCH_STREAMS_FILTER(object)  ((object)->store.streams->filter)
#define PTHREADS_FETCH_STREAMS_BUCKET(object)  ((object)->store.streams->bucket)
#define PTHREADS_FETCH_STREAMS_BRIGADE(object) ((object)->store.streams->brigade)
#define PTHREADS_FETCH_STREAMS_WRAPPER(object) ((object)->store.streams->wrapper)

#ifndef HAVE_PTHREADS_STREAMS_FILTERS_H
#	include "src/streams/filters.h"
#endif

#ifndef HAVE_PTHREADS_FILE_H
#	include <src/file/file.h>
#endif

typedef struct _pthreads_stream_statbuf {
	zend_stat_t sb; /* regular info */
	/* extended info to go here some day: content-type etc. etc. */
} pthreads_stream_statbuf;

typedef struct _pthreads_stream_dirent {
	char d_name[MAXPATHLEN];
} pthreads_stream_dirent;

typedef union _pthreads_streams_t {
	pthreads_stream 				*stream;
	pthreads_stream_context			*context;
	pthreads_stream_filter			*filter;
	pthreads_stream_bucket			*bucket;
	pthreads_stream_bucket_brigade	*brigade;
	pthreads_stream_wrapper         *wrapper;
} pthreads_streams_t;

/* operations on streams that are file-handles */
typedef struct _pthreads_stream_ops  {
	/* stdio like functions - these are mandatory! */
	size_t (*write)(pthreads_stream_t *threaded_stream, const char *buf, size_t count);
	size_t (*read)(pthreads_stream_t *threaded_stream, char *buf, size_t count);
	int    (*close)(pthreads_stream_t *threaded_stream, int close_handle);
	void   (*free)(pthreads_stream_t *threaded_stream, int close_handle);
	int    (*flush)(pthreads_stream_t *threaded_stream);

	const char *label; /* label for this ops structure */

	/* these are optional */
	int (*seek)(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffset);
	int (*cast)(pthreads_stream_t *threaded_stream, int castas, void **ret);
	int (*stat)(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb);
	int (*set_option)(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam);
} pthreads_stream_ops;

typedef struct _pthreads_stream_wrapper_ops {
	/* open/create a wrapped stream */
	pthreads_stream_t *(*stream_opener)(pthreads_stream_wrapper_t *threaded_wrapper, const char *filename, const char *mode,
			int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
	/* close/destroy a wrapped stream */
	int (*stream_closer)(pthreads_stream_wrapper_t *threaded_wrapper, pthreads_stream_t *stream);
	/* stat a wrapped stream */
	int (*stream_stat)(pthreads_stream_wrapper_t *threaded_wrapper, pthreads_stream_t *stream, pthreads_stream_statbuf *ssb);
	/* stat a URL */
	int (*url_stat)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int flags, pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context);
	/* open a "directory" stream */
	pthreads_stream_t *(*dir_opener)(pthreads_stream_wrapper_t *wrapper, const char *filename, const char *mode,
			int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);

	const char *label;

	/* delete a file */
	int (*unlink)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context);

	/* rename a file */
	int (*rename)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url_from, const char *url_to, int options, pthreads_stream_context_t *threaded_context);

	/* Create/Remove directory */
	int (*stream_mkdir)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int mode, int options, pthreads_stream_context_t *threaded_context);
	int (*stream_rmdir)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context);
	/* Metadata handling */
	int (*stream_metadata)(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, void *value, pthreads_stream_context_t *threaded_context);
} pthreads_stream_wrapper_ops;

struct _pthreads_stream_wrapper	{
	const pthreads_stream_wrapper_ops *wops;	/* operations the wrapper can perform */
	void *abstract;					/* context for the wrapper */
	int is_url;						/* so that PG(allow_url_fopen) can be respected */
};

struct _pthreads_stream  {
	const pthreads_stream_ops *ops;
	void *abstract;			/* convenience pointer for abstraction */

	pthreads_stream_wrapper_t *wrapper; /* which wrapper was used to open the stream */
	void *wrapperthis;		/* convenience pointer for a instance of a wrapper */

	uint8_t state:3;
	uint8_t in_free:2;			/* to prevent recursion during free */
	uint8_t eof:1;
	uint8_t preserve_handle:1;

	/* so we know how to clean it up correctly.  This should be set to
	 * PTHREADS_STREAM_FCLOSE_XXX as appropriate */
	uint8_t fclose_stdiocast:2;

	uint8_t fgetss_state;		/* for fgetss to handle multiline tags */

	char mode[16];			/* "rwb" etc. ala stdio */

	uint32_t flags;	/* PTHREADS_STREAM_FLAG_XXX */

	FILE *stdiocast;    /* cache this, otherwise we might leak! */
	char *orig_path;

	/* buffer */
	zend_off_t position; /* of underlying stream */
	unsigned char *readbuf;
	size_t readbuflen;
	zend_off_t readpos;
	zend_off_t writepos;

	/* how much data to read when filling buffer */
	size_t chunk_size;

}; /* pthreads_stream */

#ifndef HAVE_PTHREADS_STREAMS_CONTEXT_H
#	include "src/streams/context.h"
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_H
#	include "src/streams/wrappers.h"
#endif

#ifndef HAVE_PTHREADS_STREAM_GLOBALS_H
#	include "src/streams/streamglobals.h"
#endif

#define PTHREADS_STREAM_STATE_OPEN 			0
#define PTHREADS_STREAM_STATE_CLOSING		1
#define PTHREADS_STREAM_STATE_CLOSED		2

#define PTHREADS_IS_STREAM_CLOSING(stream) 	(((stream)->state & PTHREADS_STREAM_STATE_CLOSING) == PTHREADS_STREAM_STATE_CLOSING)
#define PTHREADS_IS_INVALID_STREAM(stream) 	((stream) == NULL || ((stream)->state & PTHREADS_STREAM_STATE_CLOSED) == PTHREADS_STREAM_STATE_CLOSED)
#define PTHREADS_IS_VALID_STREAM(stream) 	!PTHREADS_IS_INVALID_STREAM(stream)

#define PTHREADS_STREAM_PRE_CHECK(stream) do { \
	if (PTHREADS_IS_INVALID_STREAM(stream)) { \
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, \
			"stream found in invalid state(%i)", (stream)->state); \
		return; \
	} \
} while(0)

#define PTHREADS_STREAM_POST_CHECK(threaded_stream, stream) do { \
	if (PTHREADS_IS_INVALID_STREAM(stream)) { \
		MONITOR_UNLOCK((threaded_stream)); \
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, \
			"stream found in invalid state(%i)", (stream)->state); \
		return; \
	} \
} while(0)

#define PTHREADS_STREAM_CORRUPTED()  do { \
	zend_throw_exception_ex(spl_ce_RuntimeException, 0, "stream corrupted"); \
	return; \
} while(0)

#define PTHREADS_STREAM_FLAG_NO_SEEK					0x1
#define PTHREADS_STREAM_FLAG_NO_BUFFER					0x2

#define PTHREADS_STREAM_FLAG_EOL_UNIX					0x0 /* also includes DOS */
#define PTHREADS_STREAM_FLAG_DETECT_EOL					0x4
#define PTHREADS_STREAM_FLAG_EOL_MAC					0x8

/* set this when the stream might represent "interactive" data.
 * When set, the read buffer will avoid certain operations that
 * might otherwise cause the read to block for much longer than
 * is strictly required. */
#define PTHREADS_STREAM_FLAG_AVOID_BLOCKING				0x10
#define PTHREADS_STREAM_FLAG_NO_CLOSE					0x20
#define PTHREADS_STREAM_FLAG_IS_DIR						0x40
#define PTHREADS_STREAM_FLAG_NO_FCLOSE					0x80
#define PTHREADS_STREAM_FLAG_WAS_WRITTEN				0x80000000

#define PTHREADS_STREAM_STORAGE_KEY_STREAM_CONTEXT		1
#define PTHREADS_STREAM_STORAGE_KEY_STREAM_INNERSTREAM	2
#define PTHREADS_STREAM_STORAGE_KEY_STREAM_PARENT		3
#define PTHREADS_STREAM_STORAGE_KEY_STREAM_READFILTERS	4
#define PTHREADS_STREAM_STORAGE_KEY_STREAM_WRITEFILTERS	5
#define PTHREADS_STREAM_STORAGE_KEY_STREAM_WRAPPERDATA	6

#define PTHREADS_STREAM_STORAGE_KEY_CHAIN_HEAD			7
#define PTHREADS_STREAM_STORAGE_KEY_CHAIN_TAIL			8
#define PTHREADS_STREAM_STORAGE_KEY_CHAIN_STREAM		9

#define PTHREADS_STREAM_STORAGE_KEY_FILTER_PREV			10
#define PTHREADS_STREAM_STORAGE_KEY_FILTER_NEXT			11
#define PTHREADS_STREAM_STORAGE_KEY_FILTER_CHAIN		12


#define PTHREADS_GET_CTX_OPT(threaded_stream, wrapper, name, val) (pthreads_stream_get_context(threaded_stream) && NULL != (val = pthreads_stream_context_get_option(pthreads_stream_get_context(threaded_stream), wrapper, name)))

/**
 * Stream
 */
#define pthreads_stream_get_readfilters(threaded_stream) \
	((pthreads_stream_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_READFILTERS))

#define pthreads_stream_set_readfilters(threaded_stream, threaded_chain) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_READFILTERS, (threaded_chain)))

#define pthreads_stream_get_writefilters(threaded_stream) \
	((pthreads_stream_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_WRITEFILTERS))

#define pthreads_stream_set_writefilters(threaded_stream, threaded_chain) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_WRITEFILTERS, (threaded_chain)))

#define pthreads_stream_has_wrapperdata(threaded_stream) \
	(pthreads_stream_has_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_WRAPPERDATA))

#define pthreads_stream_get_wrapperdata(threaded_stream) \
	((pthreads_stream_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_WRAPPERDATA))

#define pthreads_stream_set_wrapperdata(threaded_stream, wrapperdata) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_WRAPPERDATA, (wrapperdata)))


/**
 * Context
 */
#define pthreads_stream_get_context(threaded_stream) \
	((pthreads_stream_context_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_CONTEXT))

#define pthreads_stream_set_context(threaded_stream, threaded_context) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_CONTEXT, (threaded_context)))

#define pthreads_stream_delete_context(threaded_stream) \
	(pthreads_stream_delete_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_CONTEXT))

/**
 * Inner Stream
 */
#define pthreads_get_inner_stream(threaded_stream) \
	((pthreads_stream_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_INNERSTREAM))

#define pthreads_set_inner_stream(threaded_stream, threaded_innerstream) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_INNERSTREAM, (threaded_innerstream)))

/**
 * Parent Stream
 */
#define pthreads_get_parent_stream(threaded_stream) \
	((pthreads_stream_t*) pthreads_stream_read_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_PARENT))

#define pthreads_set_parent_stream(threaded_stream, threaded_parent_stream) \
	(pthreads_stream_write_threaded_property((threaded_stream), PTHREADS_STREAM_STORAGE_KEY_STREAM_PARENT, (threaded_parent_stream)))

/**
 * Chain
 */
#define pthreads_chain_has_head(threaded_chain) \
	(pthreads_stream_has_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_HEAD))

#define pthreads_chain_get_head(threaded_chain) \
	((pthreads_stream_filter_t*) pthreads_stream_read_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_HEAD))

#define pthreads_chain_set_head(threaded_chain, threaded_filter) \
	(pthreads_stream_write_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_HEAD, (threaded_filter)))

#define pthreads_chain_has_tail(threaded_chain) \
	(pthreads_stream_has_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_TAIL))

#define pthreads_chain_get_tail(threaded_chain) \
	((pthreads_stream_filter_t*) pthreads_stream_read_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_TAIL))

#define pthreads_chain_set_tail(threaded_chain, threaded_filter) \
	(pthreads_stream_write_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_TAIL, (threaded_filter)))

#define pthreads_chain_num_elements(threaded_chain) \
	(pthreads_stream_count_threaded_properties((threaded_chain)))

#define pthreads_chain_get_stream(threaded_chain) \
	((pthreads_stream_filter_t*) pthreads_stream_read_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_STREAM))

#define pthreads_chain_set_stream(threaded_chain, threaded_stream) \
	(pthreads_stream_write_threaded_property((threaded_chain), PTHREADS_STREAM_STORAGE_KEY_CHAIN_STREAM, (threaded_stream)))


/**
 * Filter
 */
#define pthreads_filter_get_next(threaded_filter) \
	((pthreads_stream_filter_t*) pthreads_stream_read_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_NEXT))

#define pthreads_filter_set_next(threaded_filter, threaded_next_filter) \
	(pthreads_stream_write_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_NEXT, (threaded_next_filter)))

#define pthreads_filter_get_prev(threaded_filter) \
	((pthreads_stream_filter_t*) pthreads_stream_read_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_PREV))

#define pthreads_filter_set_prev(threaded_filter, threaded_prev_filter) \
	(pthreads_stream_write_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_PREV, (threaded_prev_filter)))

#define pthreads_filter_has_chain(threaded_filter) \
	(pthreads_stream_has_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_CHAIN))

#define pthreads_filter_get_chain(threaded_filter) \
	((pthreads_stream_filter_chain_t*) pthreads_stream_read_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_CHAIN))

#define pthreads_filter_set_chain(threaded_filter, threaded_chain) \
	(pthreads_stream_write_threaded_property((threaded_filter), PTHREADS_STREAM_STORAGE_KEY_FILTER_CHAIN, (threaded_chain)))


void pthreads_init_streams();
void pthreads_shutdown_streams();

zend_bool stream_lock(pthreads_stream_t *threaded_stream);
#define stream_unlock(threaded_stream) \
	(MONITOR_UNLOCK(threaded_stream))

pthreads_stream_t *_pthreads_stream_new(const pthreads_stream_ops *ops, void *abstract, const char *mode, zend_class_entry *stream_ce);
#define PTHREADS_STREAM_NEW(ops, abstract, mode) _pthreads_stream_new((ops), (abstract), (mode), pthreads_stream_entry)
#define PTHREADS_STREAM_CLASS_NEW(ops, abstract, mode, ce) _pthreads_stream_new((ops), (abstract), (mode), (ce))
pthreads_stream_filter_t *pthreads_stream_filter_new(const pthreads_stream_filter_ops *fops, void *abstract);
pthreads_stream_filter_t *pthreads_stream_filter_init();
pthreads_stream_bucket_t *pthreads_stream_bucket_new(char *buf, size_t buflen);
pthreads_stream_bucket_t *pthreads_stream_bucket_init();
pthreads_stream_bucket_brigade_t *pthreads_stream_bucket_brigade_new();
pthreads_stream_wrapper_t *pthreads_stream_wrapper_new();
pthreads_stream_context_t *pthreads_stream_context_new();

int pthreads_stream_has_threaded_property(pthreads_object_t *threaded, int property);
int pthreads_stream_count_threaded_properties(pthreads_object_t *threaded);
pthreads_object_t *pthreads_stream_read_threaded_property(pthreads_object_t *threaded, int property);
int pthreads_stream_write_threaded_property(pthreads_object_t *threaded, int property, pthreads_object_t *val);
int pthreads_stream_delete_threaded_property(pthreads_object_t *threaded, int property);

int pthreads_streams_aquire_double_lock(pthreads_object_t *object_one, pthreads_object_t *object_two);
void pthreads_streams_release_double_lock(pthreads_object_t *object_one, pthreads_object_t *object_two);

/* state definitions when closing down; these are private to streams.c */
#define PTHREADS_STREAM_FCLOSE_NONE 0
#define PTHREADS_STREAM_FCLOSE_FDOPEN	1
#define PTHREADS_STREAM_FCLOSE_FOPENCOOKIE 2


/* allocate a new stream for a particular ops */

/* Threaded streams container */
pthreads_streams_t* pthreads_streams_alloc(void);
void pthreads_streams_free(pthreads_streams_t *streams);

/* Plain stream object */
pthreads_stream *_pthreads_stream_alloc(const pthreads_stream_ops *ops, void *abstract, const char *mode);

#define pthreads_stream_alloc(ops, thisptr, mode)	_pthreads_stream_alloc((ops), (thisptr), (mode))

/* use this to assign the stream to a zval and tell the stream that is
 * has been exported to the engine; it will expect to be closed automatically
 * when the resources are auto-destructed */
#define pthreads_stream_to_zval(threaded_stream, zval)	{ ZVAL_OBJ(zval, PTHREADS_STD_P((threaded_stream))); }

pthreads_stream_t *pthreads_stream_set_parent(pthreads_stream_t *threaded_target, pthreads_stream_t *threaded_parent);
#define pthreads_stream_close_ignore_parent(threaded_stream_enclosed, close_options) _pthreads_stream_close_ignore_parent((threaded_stream_enclosed), (close_options))
int _pthreads_stream_close_ignore_parent(pthreads_stream_t *threaded_stream_enclosed, int close_options);

#define PTHREADS_STREAM_FREE_CALL_DTOR			1 /* call ops->close */
#define PTHREADS_STREAM_FREE_PRESERVE_HANDLE	4 /* tell ops->close to not close it's underlying handle */
#define PTHREADS_STREAM_FREE_IGNORE_PARENT		8 /* don't close the enclosing stream instead */
#define PTHREADS_STREAM_FREE_CLOSE				(PTHREADS_STREAM_FREE_CALL_DTOR)
#define PTHREADS_STREAM_FREE_CLOSE_CASTED		(PTHREADS_STREAM_FREE_CLOSE | PTHREADS_STREAM_FREE_PRESERVE_HANDLE)

void _pthreads_stream_free(pthreads_stream_t *threaded_stream);
#define pthreads_stream_free(threaded_stream)	_pthreads_stream_free((threaded_stream))
int _pthreads_stream_close(pthreads_stream_t *threaded_stream, int close_options, int skip_check);
#define pthreads_stream_close(threaded_stream, close_options)	_pthreads_stream_close((threaded_stream), (close_options), 0) // PTHREADS_STREAM_FREE_CLOSE
#define pthreads_stream_close_unsafe(threaded_stream, close_options)	_pthreads_stream_close((threaded_stream), (close_options), 1)

int _pthreads_stream_seek(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence);
#define pthreads_stream_rewind(threaded_stream)	_pthreads_stream_seek((threaded_stream), 0L, SEEK_SET)
#define pthreads_stream_seek(threaded_stream, offset, whence)	_pthreads_stream_seek((threaded_stream), (offset), (whence))

zend_off_t _pthreads_stream_tell(pthreads_stream_t *threaded_stream);
#define pthreads_stream_tell(threaded_stream)	_pthreads_stream_tell((threaded_stream))

size_t _pthreads_stream_read(pthreads_stream_t *threaded_stream, char *buf, size_t count);
#define pthreads_stream_read(threaded_stream, buf, count)	_pthreads_stream_read((threaded_stream), (buf), (count))

size_t _pthreads_stream_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count);
#define pthreads_stream_write_string(threaded_stream, str)	_pthreads_stream_write(threaded_stream, str, strlen(str))
#define pthreads_stream_write(threaded_stream, buf, count)	_pthreads_stream_write(threaded_stream, (buf), (count))

void _pthreads_stream_fill_read_buffer(pthreads_stream_t *threaded_stream, size_t size);
#define pthreads_stream_fill_read_buffer(threaded_stream, size)	_pthreads_stream_fill_read_buffer((threaded_stream), (size))

size_t _pthreads_stream_printf(pthreads_stream_t *threaded_stream, const char *fmt, ...) PHP_ATTRIBUTE_FORMAT(printf, 2, 3);

/* pthreads_stream_printf macro & function require */
#define pthreads_stream_printf _pthreads_stream_printf

int _pthreads_stream_eof(pthreads_stream_t *threaded_stream);
#define pthreads_stream_eof(threaded_stream)	_pthreads_stream_eof((threaded_stream))

int _pthreads_stream_getc(pthreads_stream_t *threaded_stream);
#define pthreads_stream_getc(threaded_stream)	_pthreads_stream_getc((threaded_stream))

int _pthreads_stream_putc(pthreads_stream_t *threaded_stream, int c);
#define pthreads_stream_putc(threaded_stream, c)	_pthreads_stream_putc((threaded_stream), (c))

int _pthreads_stream_flush(pthreads_stream_t *threaded_stream, int closing);
#define pthreads_stream_flush(threaded_stream)	_pthreads_stream_flush((threaded_stream), 0)

char *_pthreads_stream_get_line(pthreads_stream_t *threaded_stream, char *buf, size_t maxlen, size_t *returned_len);
#define pthreads_stream_gets(threaded_stream, buf, maxlen)	_pthreads_stream_get_line((threaded_stream), (buf), (maxlen), NULL)

#define pthreads_stream_get_line(threaded_stream, buf, maxlen, retlen) _pthreads_stream_get_line((threaded_stream), (buf), (maxlen), (retlen))
zend_string *pthreads_stream_get_record(pthreads_stream_t *threaded_stream, size_t maxlen, const char *delim, size_t delim_len);

/* CAREFUL! this is equivalent to puts NOT fputs! */
int _pthreads_stream_puts(pthreads_stream_t *threaded_stream, const char *buf);
#define pthreads_stream_puts(threaded_stream, buf)	_pthreads_stream_puts((threaded_stream), (buf))

int _pthreads_stream_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb);
#define pthreads_stream_stat(threaded_stream, ssb)	_pthreads_stream_stat((threaded_stream), (ssb))

#define pthreads_stream_closedir(threaded_dirstream)	pthreads_stream_close((threaded_dirstream), PTHREADS_STREAM_FREE_CLOSE)
#define pthreads_stream_rewinddir(threaded_dirstream)	pthreads_stream_rewind((threaded_dirstream))

int pthreads_stream_dirent_alphasort(const zend_string **a, const zend_string **b);
int pthreads_stream_dirent_alphasortr(const zend_string **a, const zend_string **b);

int _pthreads_stream_scandir(const char *dirname, zend_string **namelist[], int flags, pthreads_stream_context_t *threaded_context,
			int (*compare) (const zend_string **a, const zend_string **b));
#define pthreads_stream_scandir(dirname, namelist, threaded_context, compare) _pthreads_stream_scandir((dirname), (namelist), 0, (threaded_context), (compare))

int _pthreads_stream_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam);
#define pthreads_stream_set_option(threaded_stream, option, value, ptrvalue)	_pthreads_stream_set_option((threaded_stream), (option), (value), (ptrvalue))

#define pthreads_stream_set_chunk_size(threaded_stream, size) _pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_SET_CHUNK_SIZE, (size), NULL)

/* Flags for mkdir method in wrapper ops */
#define PTHREADS_STREAM_MKDIR_RECURSIVE		1
/* define REPORT ERRORS 8 (below) */

/* Flags for rmdir method in wrapper ops */
/* define REPORT_ERRORS 8 (below) */

/* Flags for url_stat method in wrapper ops */
#define PTHREADS_STREAM_URL_STAT_LINK		1
#define PTHREADS_STREAM_URL_STAT_QUIET		2
#define PTHREADS_STREAM_URL_STAT_NOCACHE	4

/* change the blocking mode of stream: value == 1 => blocking, value == 0 => non-blocking. */
#define PTHREADS_STREAM_OPTION_BLOCKING	1

/* change the buffering mode of stream. value is a PTHREADS_STREAM_BUFFER_XXXX value, ptrparam is a ptr to a size_t holding
 * the required buffer size */
#define PTHREADS_STREAM_OPTION_READ_BUFFER	2
#define PTHREADS_STREAM_OPTION_WRITE_BUFFER	3

#define PTHREADS_STREAM_BUFFER_NONE	0	/* unbuffered */
#define PTHREADS_STREAM_BUFFER_LINE	1	/* line buffered */
#define PTHREADS_STREAM_BUFFER_FULL	2	/* fully buffered */

/* whether or not locking is supported */
#define PTHREADS_STREAM_LOCK_SUPPORTED			1

/* set the timeout duration for reads on the stream. ptrparam is a pointer to a struct timeval * */
#define PTHREADS_STREAM_OPTION_READ_TIMEOUT		4
#define PTHREADS_STREAM_OPTION_SET_CHUNK_SIZE	5

/* set or release lock on a stream */
#define PTHREADS_STREAM_OPTION_LOCKING			6

#define pthreads_stream_supports_lock(threaded_stream)	(_pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_LOCKING, 0, (void *) PTHREADS_STREAM_LOCK_SUPPORTED) == 0 ? 1 : 0)
#define pthreads_stream_lock(threaded_stream, mode)		_pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_LOCKING, (mode), (void *) NULL)

/* option code used by the pthreads_stream_xport_XXX api */
#define PTHREADS_STREAM_OPTION_XPORT_API			7 /* see transport.h */
#define PTHREADS_STREAM_OPTION_CRYPTO_API		8 /* see transport.h */
#define PTHREADS_STREAM_OPTION_MMAP_API			9 /* see mmap.h */
#define PTHREADS_STREAM_OPTION_TRUNCATE_API		10
#define PTHREADS_STREAM_OPTION_META_DATA_API	11 /* ptrparam is a zval* to which to add meta data information */

/* Check if the stream is still "live"; for sockets/pipes this means the socket
 * is still connected; for files, this does not really have meaning */
#define PTHREADS_STREAM_OPTION_CHECK_LIVENESS	12 /* no parameters */

/* Enable/disable blocking reads on anonymous pipes on Windows. */
#define PTHREADS_STREAM_OPTION_PIPE_BLOCKING	13

#define PTHREADS_STREAM_TRUNCATE_SUPPORTED		0
#define PTHREADS_STREAM_TRUNCATE_SET_SIZE		1	/* ptrparam is a pointer to a size_t */

#define PTHREADS_STREAM_OPTION_RETURN_OK		 0 /* option set OK */
#define PTHREADS_STREAM_OPTION_RETURN_ERR		-1 /* problem setting option */
#define PTHREADS_STREAM_OPTION_RETURN_NOTIMPL	-2 /* underlying stream does not implement; streams can handle it instead */

#define pthreads_stream_truncate_supported(threaded_stream)	(_pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_TRUNCATE_API, PTHREADS_STREAM_TRUNCATE_SUPPORTED, NULL) == PTHREADS_STREAM_OPTION_RETURN_OK ? 1 : 0)


int _pthreads_stream_truncate_set_size(pthreads_stream_t *threaded_stream, size_t newsize);
#define pthreads_stream_truncate_set_size(threaded_stream, size)	_pthreads_stream_truncate_set_size((threaded_stream), (size))

#define pthreads_stream_populate_meta_data(threaded_stream, zv)	(_pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_META_DATA_API, 0, zv) == PTHREADS_STREAM_OPTION_RETURN_OK ? 1 : 0)

/* copy up to maxlen bytes from src to dest.  If maxlen is PTHREADS_STREAM_COPY_ALL, copy until eof(src). */
#define PTHREADS_STREAM_COPY_ALL	((size_t)-1)

/* output all data from a stream */
PHPAPI size_t _pthreads_stream_passthru(pthreads_stream_t * threaded_src);
#define pthreads_stream_passthru(threaded_stream)	_pthreads_stream_passthru((threaded_stream))

/* read all data from stream and put into a buffer. Caller must free buffer when done. */
zend_string *_pthreads_stream_copy_to_mem(pthreads_stream_t *threaded_src, size_t maxlen, int persistent);
#define pthreads_stream_copy_to_mem(threaded_src, maxlen, persistent) _pthreads_stream_copy_to_mem((threaded_src), (maxlen), (persistent))

int _pthreads_stream_copy_to_stream_ex(pthreads_stream_t *threaded_src, pthreads_stream_t *threaded_dest, size_t maxlen, size_t *len);
#define pthreads_stream_copy_to_stream_ex(threaded_src, threaded_dest, maxlen, len)	_pthreads_stream_copy_to_stream_ex((threaded_src), (threaded_dest), (maxlen), (len))

#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#	include <src/streams/transports.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_PLAIN_WRAPPER_H
#	include <src/streams/wrappers/plain_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_GLOB_WRAPPER_H
#	include <src/streams/wrappers/glob_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER_H
#	include <src/streams/wrappers/user_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_MMAP_H
#	include <src/streams/mmap.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_BUCKETS_H
#	include <src/streams/buckets.h>
#endif

/* coerce the stream into some other form */
/* cast as a stdio FILE * */
#define PTHREADS_STREAM_AS_STDIO     0
/* cast as a POSIX fd or socketd */
#define PTHREADS_STREAM_AS_FD		1
/* cast as a socketd */
#define PTHREADS_STREAM_AS_SOCKETD	2
/* cast as fd/socket for select purposes */
#define PTHREADS_STREAM_AS_FD_FOR_SELECT 3

/* try really, really hard to make sure the cast happens (avoid using this flag if possible) */
#define PTHREADS_STREAM_CAST_TRY_HARD	0x80000000
#define PTHREADS_STREAM_CAST_RELEASE	0x40000000	/* stream becomes invalid on success */
#define PTHREADS_STREAM_CAST_INTERNAL	0x20000000	/* stream cast for internal use */
#define PTHREADS_STREAM_CAST_MASK		(PTHREADS_STREAM_CAST_TRY_HARD | PTHREADS_STREAM_CAST_RELEASE | PTHREADS_STREAM_CAST_INTERNAL)

int _pthreads_stream_cast(pthreads_stream_t *threaded_stream, int castas, void **ret, int show_err);

/* use this to check if a stream can be cast into another form */
#define pthreads_stream_can_cast(threaded_stream, as)	_pthreads_stream_cast((threaded_stream), (as), NULL, 0)
#define pthreads_stream_cast(threaded_stream, as, ret, show_err)	_pthreads_stream_cast((threaded_stream), (as), (ret), (show_err))

/* use this to check if a stream is of a particular type:
 * int pthreads_stream_is(pthreads_stream_t *threaded_stream, pthreads_stream_ops *ops); */
#define pthreads_stream_is(threaded_stream, anops)		(PTHREADS_FETCH_STREAMS_STREAM((threaded_stream))->ops == anops)
#define PTHREADS_STREAM_IS_STDIO &pthreads_stream_stdio_ops

/* Wrappers support */

#define PTHREADS_IGNORE_PATH                     0x00000000
#define PTHREADS_USE_PATH                        0x00000001
#define PTHREADS_IGNORE_URL                      0x00000002
#define PTHREADS_REPORT_ERRORS                   0x00000008

/* If you don't need to write to the stream, but really need to
 * be able to seek, use this flag in your options. */
#define PTHREADS_STREAM_MUST_SEEK                0x00000010
/* If you are going to end up casting the stream into a FILE* or
 * a socket, pass this flag and the streams/wrappers will not use
 * buffering mechanisms while reading the headers, so that HTTP
 * wrapped streams will work consistently.
 * If you omit this flag, streams will use buffering and should end
 * up working more optimally.
 * */
#define PTHREADS_STREAM_WILL_CAST                0x00000020

/* this flag applies to pthreads_stream_locate_url_wrapper */
#define PTHREADS_STREAM_LOCATE_WRAPPERS_ONLY     0x00000040

/* this flag is only used by include/require functions */
#define PTHREADS_STREAM_OPEN_FOR_INCLUDE         0x00000080

/* this flag tells streams to ONLY open urls */
#define PTHREADS_STREAM_USE_URL                  0x00000100

/* this flag is used when only the headers from HTTP request are to be fetched */
#define PTHREADS_STREAM_ONLY_GET_HEADERS         0x00000200

/* don't apply open_basedir checks */
#define PTHREADS_STREAM_DISABLE_OPEN_BASEDIR     0x00000400

/* get (or create) a persistent version of the stream */
#define PTHREADS_STREAM_OPEN_PERSISTENT          0x00000800

/* use glob stream for directory open in plain files stream */
#define PTHREADS_STREAM_USE_GLOB_DIR_OPEN        0x00001000

/* don't check allow_url_fopen and allow_url_include */
#define PTHREADS_STREAM_DISABLE_URL_PROTECTION   0x00002000

/* assume the path passed in exists and is fully expanded, avoiding syscalls */
#define PTHREADS_STREAM_ASSUME_REALPATH          0x00004000

/* Allow blocking reads on anonymous pipes on Windows. */
#define PTHREADS_STREAM_USE_BLOCKING_PIPE        0x00008000

const char *pthreads_stream_locate_eol(pthreads_stream_t *threaded_stream, zend_string *buf);

#define PTHREADS_STREAM_UNCHANGED			0 /* orig stream was seekable anyway */
#define PTHREADS_STREAM_RELEASED			1 /* newstream should be used; origstream is no longer valid */
#define PTHREADS_STREAM_FAILED				2 /* an error occurred while attempting conversion */
#define PTHREADS_STREAM_CRITICAL			3 /* an error occurred; origstream is in an unknown state; you should close origstream */
#define PTHREADS_STREAM_NO_PREFERENCE		0
#define PTHREADS_STREAM_PREFER_STDIO		1
#define PTHREADS_STREAM_FORCE_CONVERSION	2

/* DO NOT call this on streams that are referenced by resources! */
int _pthreads_stream_make_seekable(pthreads_stream_t *threaded_origstream, pthreads_stream_t **threaded_newstream, int flags);
#define pthreads_stream_make_seekable(threaded_origstream, threaded_newstream, flags)	_pthreads_stream_make_seekable((threaded_origstream), (threaded_newstream), (flags))

/* Give other modules access to the url_stream_wrappers_hash and stream_filters_hash */
pthreads_hashtable *_pthreads_stream_get_url_stream_wrappers_hash(void);
#define pthreads_stream_get_url_stream_wrappers_hash()	_pthreads_stream_get_url_stream_wrappers_hash()

/* Definitions for user streams */
#define PTHREADS_STREAM_IS_URL		1

/* Stream metadata definitions */
/* Create if referred resource does not exist */
#define PTHREADS_STREAM_META_TOUCH		1
#define PTHREADS_STREAM_META_OWNER_NAME	2
#define PTHREADS_STREAM_META_OWNER		3
#define PTHREADS_STREAM_META_GROUP_NAME	4
#define PTHREADS_STREAM_META_GROUP		5
#define PTHREADS_STREAM_META_ACCESS		6

/* Flags for stream_socket_client */
#define PTHREADS_STREAM_CLIENT_ASYNC_CONNECT	2
#define PTHREADS_STREAM_CLIENT_CONNECT			4

#endif
