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
#ifndef HAVE_PTHREADS_STREAMS_MMAP_H
#define HAVE_PTHREADS_STREAMS_MMAP_H

/* Memory Mapping interface for streams.
 * The intention is to provide a uniform interface over the most common
 * operations that are used within PHP itself, rather than a complete
 * API for all memory mapping needs.
 *
 * ATM, we support only mmap(), but win32 memory mapping support will
 * follow soon.
 * */

typedef enum {
	/* Does the stream support mmap ? */
	PTHREADS_STREAM_MMAP_SUPPORTED,
	/* Request a range and offset to be mapped;
	 * while mapped, you MUST NOT use any read/write functions
	 * on the stream (win9x compatibility) */
	PTHREADS_STREAM_MMAP_MAP_RANGE,
	/* Unmap the last range that was mapped for the stream */
	PTHREADS_STREAM_MMAP_UNMAP
} pthreads_stream_mmap_operation_t;

typedef enum {
	PTHREADS_STREAM_MAP_MODE_READONLY,
	PTHREADS_STREAM_MAP_MODE_READWRITE,
	PTHREADS_STREAM_MAP_MODE_SHARED_READONLY,
	PTHREADS_STREAM_MAP_MODE_SHARED_READWRITE
} pthreads_stream_mmap_access_t;

typedef struct {
	/* requested offset and length.
	 * If length is 0, the whole file is mapped */
	size_t offset;
	size_t length;

	pthreads_stream_mmap_access_t mode;

	/* returned mapped address */
	char *mapped;

} pthreads_stream_mmap_range;

#define PTHREADS_STREAM_MMAP_ALL 0

#define pthreads_stream_mmap_supported(threaded_stream)	(_pthreads_stream_set_option((threaded_stream), PTHREADS_STREAM_OPTION_MMAP_API, PTHREADS_STREAM_MMAP_SUPPORTED, NULL) == 0 ? 1 : 0)

/* Returns 1 if the stream in its current state can be memory mapped, 0 otherwise */
#define pthreads_stream_mmap_possible(threaded_stream)	(!pthreads_stream_is_filtered((threaded_stream)) && pthreads_stream_mmap_supported(threaded_stream))

char *_pthreads_stream_mmap_range(pthreads_stream_t *threaded_stream, size_t offset, size_t length, pthreads_stream_mmap_access_t mode, size_t *mapped_len);
#define pthreads_stream_mmap_range(threaded_stream, offset, length, mode, mapped_len)	_pthreads_stream_mmap_range((threaded_stream), (offset), (length), (mode), (mapped_len))

/* un-maps the last mapped range */
int _pthreads_stream_mmap_unmap(pthreads_stream_t *threaded_stream);
#define pthreads_stream_mmap_unmap(threaded_stream)	_pthreads_stream_mmap_unmap((threaded_stream))

int _pthreads_stream_mmap_unmap_ex(pthreads_stream_t *threaded_stream, zend_off_t readden);
#define pthreads_stream_mmap_unmap_ex(threaded_stream, readden)	_pthreads_stream_mmap_unmap_ex((threaded_stream), (readden))

#endif
