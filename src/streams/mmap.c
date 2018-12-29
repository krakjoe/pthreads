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
#ifndef HAVE_PTHREADS_STREAMS_MMAP
#define HAVE_PTHREADS_STREAMS_MMAP

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

/* Memory Mapping interface for streams */
//#include "php.h"
#ifndef HAVE_PTHREADS_STREAMS_INTERNAL_H
#	include <src/streams/internal.h>
#endif

char *_pthreads_stream_mmap_range(pthreads_stream_t *threaded_stream, size_t offset, size_t length, pthreads_stream_mmap_access_t mode, size_t *mapped_len) {
	pthreads_stream_mmap_range range;

	range.offset = offset;
	range.length = length;
	range.mode = mode;
	range.mapped = NULL;

	/* For now, we impose an arbitrary limit to avoid
	 * runaway swapping when large files are passed through. */
	if (length > 4 * 1024 * 1024) {
		return NULL;
	}

	if (PTHREADS_STREAM_OPTION_RETURN_OK == pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_MMAP_API, PTHREADS_STREAM_MMAP_MAP_RANGE, &range)) {
		if (mapped_len) {
			*mapped_len = range.length;
		}
		return range.mapped;
	}
	return NULL;
}

int _pthreads_stream_mmap_unmap(pthreads_stream_t *threaded_stream) {
	return pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_MMAP_API, PTHREADS_STREAM_MMAP_UNMAP, NULL) == PTHREADS_STREAM_OPTION_RETURN_OK ? 1 : 0;
}

int _pthreads_stream_mmap_unmap_ex(pthreads_stream_t *threaded_stream, zend_off_t readden) {
	int ret = 1;

	if(stream_lock(threaded_stream)) {
		if (pthreads_stream_seek(threaded_stream, readden, SEEK_CUR) != 0) {
			ret = 0;
		}
		if (pthreads_stream_mmap_unmap(threaded_stream) == 0) {
			ret = 0;
		}
		stream_unlock(threaded_stream);
	}

	return ret;
}

#endif
