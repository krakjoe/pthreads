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
#ifndef HAVE_PTHREADS_STREAMS_MEMORY
#define HAVE_PTHREADS_STREAMS_MEMORY

#include "php.h"
#include "ext/standard/base64.h"

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_MEMORY_H
#	include <src/streams/memory.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

size_t pthreads_url_decode(char *str, size_t len);

/* Memory streams use a dynamic memory buffer to emulate a stream.
 * You can use pthreads_stream_memory_open to create a readonly stream
 * from an existing memory buffer.
 */

/* Temp streams are streams that uses memory streams as long their
 * size is less than a given memory amount. When a write operation
 * exceeds that limit the content is written to a temporary file.
 */

/* {{{ ------- MEMORY stream implementation -------*/

typedef struct {
	char        *data;
	size_t      fpos;
	size_t      fsize;
	size_t      smax;
	int			mode;
} pthreads_stream_memory_data;


/* {{{ */
static size_t pthreads_stream_memory_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
		assert(ms != NULL);

		if (ms->mode & PTHREADS_TEMP_STREAM_READONLY) {
			return 0;
		} else if (ms->mode & PTHREADS_TEMP_STREAM_APPEND) {
			ms->fpos = ms->fsize;
		}
		if (ms->fpos + count > ms->fsize) {
			char *tmp;
			if (!ms->data) {
				tmp = malloc(ms->fpos + count);
			} else {
				tmp = realloc(ms->data, ms->fpos + count);
			}
			ms->data = tmp;
			ms->fsize = ms->fpos + count;
		}
		if (!ms->data)
			count = 0;
		if (count) {
			assert(buf!= NULL);
			memcpy(ms->data + ms->fpos, (char*)buf, count);
			ms->fpos += count;
		}
		stream_unlock(threaded_stream);
	}
	return count;
}
/* }}} */

/* {{{ */
static size_t pthreads_stream_memory_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
		assert(ms != NULL);

		if (ms->fpos == ms->fsize) {
			stream->eof = 1;
			count = 0;
		} else {
			if (ms->fpos + count >= ms->fsize) {
				count = ms->fsize - ms->fpos;
			}
			if (count) {
				assert(ms->data!= NULL);
				assert(buf!= NULL);
				memcpy(buf, ms->data+ms->fpos, count);
				ms->fpos += count;
			}
		}
		stream_unlock(threaded_stream);
	}
	return count;
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_close(pthreads_stream_t *threaded_stream, int close_handle) {
	return 0;
}
/* }}} */

/* {{{ */
static void pthreads_stream_memory_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
	assert(ms != NULL);

	if (ms->data && close_handle && ms->mode != PTHREADS_TEMP_STREAM_READONLY) {
		free(ms->data);
	}
	free(ms);
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_flush(pthreads_stream_t *threaded_stream)
{
	/* nothing to do here */
	return 0;
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_seek(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffs) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
		assert(ms != NULL);

		switch(whence) {
			case SEEK_CUR:
				if (offset < 0) {
					if (ms->fpos < (size_t)(-offset)) {
						ms->fpos = 0;
						*newoffs = -1;
						return -1;
					} else {
						ms->fpos = ms->fpos + offset;
						*newoffs = ms->fpos;
						stream->eof = 0;
						return 0;
					}
				} else {
					if (ms->fpos + (size_t)(offset) > ms->fsize) {
						ms->fpos = ms->fsize;
						*newoffs = -1;
						return -1;
					} else {
						ms->fpos = ms->fpos + offset;
						*newoffs = ms->fpos;
						stream->eof = 0;
						return 0;
					}
				}
			case SEEK_SET:
				if (ms->fsize < (size_t)(offset)) {
					ms->fpos = ms->fsize;
					*newoffs = -1;
					return -1;
				} else {
					ms->fpos = offset;
					*newoffs = ms->fpos;
					stream->eof = 0;
					return 0;
				}
			case SEEK_END:
				if (offset > 0) {
					ms->fpos = ms->fsize;
					*newoffs = -1;
					return -1;
				} else if (ms->fsize < (size_t)(-offset)) {
					ms->fpos = 0;
					*newoffs = -1;
					return -1;
				} else {
					ms->fpos = ms->fsize + offset;
					*newoffs = ms->fpos;
					stream->eof = 0;
					return 0;
				}
			default:
				*newoffs = ms->fpos;
				return -1;
		}
		stream_unlock(threaded_stream);
	}
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_cast(pthreads_stream_t *threaded_stream, int castas, void **ret) {
	return FAILURE;
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb)  {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		time_t timestamp = 0;
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
		assert(ms != NULL);

		memset(ssb, 0, sizeof(pthreads_stream_statbuf));
		/* read-only across the board */

		ssb->sb.st_mode = ms->mode & PTHREADS_TEMP_STREAM_READONLY ? 0444 : 0666;

		ssb->sb.st_size = ms->fsize;
		ssb->sb.st_mode |= S_IFREG; /* regular file */
		ssb->sb.st_mtime = timestamp;
		ssb->sb.st_atime = timestamp;
		ssb->sb.st_ctime = timestamp;
		ssb->sb.st_nlink = 1;
		ssb->sb.st_rdev = -1;
		/* this is only for APC, so use /dev/null device - no chance of conflict there! */
		ssb->sb.st_dev = 0xC;
		/* generate unique inode number for alias/filename, so no phars will conflict */
		ssb->sb.st_ino = 0;

#ifndef PHP_WIN32
		ssb->sb.st_blksize = -1;
		ssb->sb.st_blocks = -1;
#endif
		stream_unlock(threaded_stream);
	}
	return 0;
}
/* }}} */

/* {{{ */
static int pthreads_stream_memory_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;
		size_t newsize;

		switch(option) {
			case PTHREADS_STREAM_OPTION_TRUNCATE_API:
				switch (value) {
					case PTHREADS_STREAM_TRUNCATE_SUPPORTED:
						stream_unlock(threaded_stream);
						return PTHREADS_STREAM_OPTION_RETURN_OK;

					case PTHREADS_STREAM_TRUNCATE_SET_SIZE:
						if (ms->mode & PTHREADS_TEMP_STREAM_READONLY) {
							stream_unlock(threaded_stream);
							return PTHREADS_STREAM_OPTION_RETURN_ERR;
						}
						newsize = *(size_t*)ptrparam;
						if (newsize <= ms->fsize) {
							if (newsize < ms->fpos) {
								ms->fpos = newsize;
							}
						} else {
							ms->data = realloc(ms->data, newsize);
							memset(ms->data+ms->fsize, 0, newsize - ms->fsize);
							ms->fsize = newsize;
						}
						ms->fsize = newsize;
						stream_unlock(threaded_stream);
						return PTHREADS_STREAM_OPTION_RETURN_OK;
				}
			default:
				stream_unlock(threaded_stream);
				return PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
		}
		stream_unlock(threaded_stream); // never used
	}
}
/* }}} */

const pthreads_stream_ops pthreads_stream_memory_ops = {
	pthreads_stream_memory_write, pthreads_stream_memory_read,
	pthreads_stream_memory_close, pthreads_stream_memory_free,
	pthreads_stream_memory_flush,
	"MEMORY",
	pthreads_stream_memory_seek,
	pthreads_stream_memory_cast,
	pthreads_stream_memory_stat,
	pthreads_stream_memory_set_option
};

/* {{{ */
int pthreads_stream_mode_from_str(const char *mode)
{
	if (strpbrk(mode, "a")) {
		return PTHREADS_TEMP_STREAM_APPEND;
	} else if (strpbrk(mode, "w+")) {
		return PTHREADS_TEMP_STREAM_DEFAULT;
	}
	return PTHREADS_TEMP_STREAM_READONLY;
}
/* }}} */

/* {{{ */
const char *_pthreads_stream_mode_to_str(int mode)
{
	if (mode == PTHREADS_TEMP_STREAM_READONLY) {
		return "rb";
	} else if (mode == PTHREADS_TEMP_STREAM_APPEND) {
		return "a+b";
	}
	return "w+b";
}
/* }}} */

/* {{{ */
pthreads_stream_t *_pthreads_stream_memory_create(int mode, zend_class_entry *ce) {
	pthreads_stream_memory_data *self;
	pthreads_stream_t *threaded_stream;

	self = malloc(sizeof(*self));
	self->data = NULL;
	self->fpos = 0;
	self->fsize = 0;
	self->smax = ~0u;
	self->mode = mode;

	if(ce == NULL) {
		ce = pthreads_file_stream_entry;
	}
	threaded_stream = PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_memory_ops, self, _pthreads_stream_mode_to_str(mode), ce);
	PTHREADS_FETCH_STREAMS_STREAM(threaded_stream)->flags |= PTHREADS_STREAM_FLAG_NO_BUFFER;

	return threaded_stream;
}
/* }}} */

/* {{{ */
pthreads_stream_t *_pthreads_stream_memory_open(int mode, char *buf, size_t length, zend_class_entry *ce) {
	pthreads_stream *stream;
	pthreads_stream_t *threaded_stream;
	pthreads_stream_memory_data *ms;

	if ((threaded_stream = pthreads_stream_memory_create(mode, ce)) != NULL) {
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
		ms = (pthreads_stream_memory_data*)stream->abstract;

		if (mode == PTHREADS_TEMP_STREAM_READONLY || mode == PTHREADS_TEMP_STREAM_TAKE_BUFFER) {
			/* use the buffer directly */
			ms->data = buf;
			ms->fsize = length;
		} else {
			if (length) {
				assert(buf != NULL);
				pthreads_stream_write(threaded_stream, buf, length);
			}
		}
	}
	return threaded_stream;
}
/* }}} */

/* {{{ */
char *_pthreads_stream_memory_get_buffer(pthreads_stream_t *threaded_stream, size_t *length) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if(stream_lock(threaded_stream)) {
		pthreads_stream_memory_data *ms = (pthreads_stream_memory_data*)stream->abstract;

		assert(ms != NULL);
		assert(length != 0);

		*length = ms->fsize;

		stream_unlock(threaded_stream);
		return ms->data;
	}
	return NULL;
}
/* }}} */

/* }}} */

/* {{{ ------- TEMP stream implementation -------*/

typedef struct {
	pthreads_stream_t  *innerstream;
	size_t             smax;
	int			       mode;
	zval               meta;
	char*		       tmpdir;
	pthreads_object_t  *storage;
} pthreads_stream_temp_data;


/* {{{ */
static size_t pthreads_stream_temp_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;
	size_t ret = -1;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		if (!innerstream) {
			stream_unlock(threaded_stream);
			return -1;
		}

		if (pthreads_stream_is(innerstream, PTHREADS_STREAM_IS_MEMORY)) {
			size_t memsize;
			char *membuf = pthreads_stream_memory_get_buffer(innerstream, &memsize);

			if (memsize + count >= ts->smax) {
				pthreads_stream_t *threaded_file = pthreads_stream_fopen_temporary_file(ts->tmpdir, "php", NULL);
				if (threaded_file == NULL) {
					stream_unlock(threaded_stream);
					php_error_docref(NULL, E_WARNING, "Unable to create temporary file, Check permissions in temporary files directory.");
					return 0;
				}
				pthreads_stream_write(threaded_file, membuf, memsize);
				pthreads_stream_close_ignore_parent(innerstream, PTHREADS_STREAM_FREE_CLOSE);

				pthreads_set_inner_stream(threaded_stream, threaded_file);
				pthreads_stream_set_parent(innerstream, threaded_stream);

				pthreads_del_ref(innerstream);
				pthreads_del_ref(threaded_stream);
			}
		}
		ret = pthreads_stream_write(innerstream, buf, count);

		stream_unlock(threaded_stream);
	}
	return ret;
}
/* }}} */

/* {{{ */
static size_t pthreads_stream_temp_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		size_t got;

		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		if (!innerstream) {
			stream_unlock(threaded_stream);
			return -1;
		}
		got = pthreads_stream_read(innerstream, buf, count);

		stream->eof = PTHREADS_FETCH_STREAMS_STREAM(innerstream)->eof;
		stream_unlock(threaded_stream);

		return got;
	}
	return -1;
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_close(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;

	pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
	int ret;

	assert(ts != NULL);

	innerstream = pthreads_get_inner_stream(threaded_stream);

	if (innerstream) {
		ret = pthreads_stream_close_ignore_parent(innerstream, PTHREADS_STREAM_FREE_CLOSE | (close_handle ? 0 : PTHREADS_STREAM_FREE_PRESERVE_HANDLE));
	} else {
		ret = 0;
	}
	zval_ptr_dtor(&ts->meta);

	return ret;
}
/* }}} */

/* {{{ */
static void pthreads_stream_temp_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;

	assert(ts != NULL);

	if (ts->tmpdir) {
		free(ts->tmpdir);
	}
	free(ts);
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_flush(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;
	int result = -1;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);
		result = innerstream ? pthreads_stream_flush(innerstream) : -1;

		stream_unlock(threaded_stream);
	}
	return result;
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_seek(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffs) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;
	int ret = -1;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		if (!innerstream) {
			*newoffs = -1;
			stream_unlock(threaded_stream);
			return ret;
		}
		ret = pthreads_stream_seek(innerstream, offset, whence);
		*newoffs = pthreads_stream_tell(innerstream);
		stream->eof = PTHREADS_FETCH_STREAMS_STREAM(innerstream)->eof;

		stream_unlock(threaded_stream);
	}
	return ret;
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_cast(pthreads_stream_t *threaded_stream, int castas, void **ret) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		pthreads_stream_t *threaded_file;
		size_t memsize;
		char *membuf;
		zend_off_t pos;

		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		if (!innerstream) {
			stream_unlock(threaded_stream);
			return FAILURE;
		}

		if (pthreads_stream_is(innerstream, PTHREADS_STREAM_IS_STDIO)) {
			int result = pthreads_stream_cast(innerstream, castas, ret, 0);
			stream_unlock(threaded_stream);
			return result;
		}

		/* we are still using a memory based backing. If they are if we can be
		 * a FILE*, say yes because we can perform the conversion.
		 * If they actually want to perform the conversion, we need to switch
		 * the memory stream to a tmpfile stream */

		if (ret == NULL && castas == PTHREADS_STREAM_AS_STDIO) {
			return SUCCESS;
		}

		/* say "no" to other stream forms */
		if (ret == NULL) {
			return FAILURE;
		}

		threaded_file = pthreads_stream_fopen_tmpfile();
		if (threaded_file == NULL) {
			stream_unlock(threaded_stream);
			php_error_docref(NULL, E_WARNING, "Unable to create temporary file.");
			return FAILURE;
		}

		/* perform the conversion and then pass the request on to the innerstream */
		membuf = pthreads_stream_memory_get_buffer(innerstream, &memsize);
		pthreads_stream_write(threaded_file, membuf, memsize);
		pos = pthreads_stream_tell(innerstream);

		pthreads_stream_close_ignore_parent(innerstream, PTHREADS_STREAM_FREE_CLOSE);
		pthreads_set_inner_stream(threaded_stream, threaded_file);

		/* switch to new innerstream*/
		innerstream = threaded_file;

		pthreads_stream_set_parent(innerstream, threaded_stream);

		pthreads_del_ref(innerstream);
		pthreads_del_ref(threaded_stream);

		pthreads_stream_seek(innerstream, pos, SEEK_SET);

		int result = pthreads_stream_cast(innerstream, castas, ret, 1);

		stream_unlock(threaded_stream);

		return result;
	}
	return FAILURE;
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;
	int ret = -1;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		if (!ts || !innerstream) {
			stream_unlock(threaded_stream);
			return ret;
		}
		ret = pthreads_stream_stat(innerstream, ssb);
		stream_unlock(threaded_stream);
	}
	return ret;
}
/* }}} */

/* {{{ */
static int pthreads_stream_temp_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *innerstream;
	int ret = -1;

	if(stream_lock(threaded_stream)) {
		pthreads_stream_temp_data *ts = (pthreads_stream_temp_data*)stream->abstract;

		assert(ts != NULL);
		innerstream = pthreads_get_inner_stream(threaded_stream);

		switch(option) {
			case PTHREADS_STREAM_OPTION_META_DATA_API:
				if (Z_TYPE(ts->meta) != IS_UNDEF) {
					zend_hash_copy(Z_ARRVAL_P((zval*)ptrparam), Z_ARRVAL(ts->meta), zval_add_ref);
				}
				stream_unlock(threaded_stream);

				return PTHREADS_STREAM_OPTION_RETURN_OK;
			default:
				if (innerstream) {
					ret = pthreads_stream_set_option(innerstream, option, value, ptrparam);
					stream_unlock(threaded_stream);

					return ret;
				}
				stream_unlock(threaded_stream);

				return PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
		}
		stream_unlock(threaded_stream);
	}
}
/* }}} */

const pthreads_stream_ops pthreads_stream_temp_ops = {
	pthreads_stream_temp_write, pthreads_stream_temp_read,
	pthreads_stream_temp_close, pthreads_stream_temp_free,
	pthreads_stream_temp_flush,
	"TEMP",
	pthreads_stream_temp_seek,
	pthreads_stream_temp_cast,
	pthreads_stream_temp_stat,
	pthreads_stream_temp_set_option
};

/* }}} */

/* {{{ _pthreads_stream_temp_create_ex */
pthreads_stream_t *_pthreads_stream_temp_create_ex(int mode, size_t max_memory_usage, const char *tmpdir, zend_class_entry *ce) {
	pthreads_stream_temp_data *self;
	pthreads_stream_t *threaded_stream, *innerstream;

	self = calloc(1, sizeof(*self));
	self->smax = max_memory_usage;
	self->mode = mode;
	ZVAL_UNDEF(&self->meta);
	if (tmpdir) {
		self->tmpdir = strdup(tmpdir);
	}
	threaded_stream = PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_temp_ops, self, _pthreads_stream_mode_to_str(mode), ce);
	PTHREADS_FETCH_STREAMS_STREAM(threaded_stream)->flags |= PTHREADS_STREAM_FLAG_NO_BUFFER;

	innerstream = pthreads_stream_memory_create(mode, ce);

	pthreads_set_inner_stream(threaded_stream, innerstream);

	pthreads_stream_set_parent(innerstream, threaded_stream);

	pthreads_del_ref(innerstream);
	pthreads_del_ref(threaded_stream);

	return threaded_stream;
}
/* }}} */

/* {{{ _pthreads_stream_temp_create */
pthreads_stream_t *_pthreads_stream_temp_create(int mode, size_t max_memory_usage, zend_class_entry *ce) {
	if(ce == NULL) {
		ce = pthreads_file_stream_entry;
	}
	return pthreads_stream_temp_create_ex(mode, max_memory_usage, NULL, ce);
}
/* }}} */

/* {{{ _pthreads_stream_temp_open */
pthreads_stream_t *_pthreads_stream_temp_open(int mode, size_t max_memory_usage, char *buf, size_t length, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream;
	pthreads_stream_temp_data *ts;
	zend_off_t newoffs;

	if ((threaded_stream = pthreads_stream_temp_create(mode, max_memory_usage, ce)) != NULL) {
		if (length) {
			assert(buf != NULL);
			pthreads_stream_temp_write(threaded_stream, buf, length);
			pthreads_stream_temp_seek(threaded_stream, 0, SEEK_SET, &newoffs);
		}
		ts = (pthreads_stream_temp_data*)PTHREADS_FETCH_STREAMS_STREAM(threaded_stream)->abstract;
		assert(ts != NULL);
		ts->mode = mode;
	}
	return threaded_stream;
}
/* }}} */

const pthreads_stream_ops pthreads_stream_rfc2397_ops = {
	pthreads_stream_temp_write, pthreads_stream_temp_read,
	pthreads_stream_temp_close, pthreads_stream_temp_free,
	pthreads_stream_temp_flush,
	"RFC2397",
	pthreads_stream_temp_seek,
	pthreads_stream_temp_cast,
	pthreads_stream_temp_stat,
	pthreads_stream_temp_set_option
};

/* {{{ */
pthreads_stream_t * pthreads_stream_url_wrap_rfc2397(pthreads_stream_wrapper_t *threaded_wrapper, const char *path,
												const char *mode, int options, zend_string **opened_path,
												pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream *stream;
	pthreads_stream_t *threaded_stream;
	pthreads_stream_temp_data *ts;
	char *comma, *semi, *sep;
	size_t mlen, dlen, plen, vlen, ilen;
	zend_off_t newoffs;
	zval meta;
	int base64 = 0;
	zend_string *base64_comma = NULL, *base64_tmp = NULL;

	ZVAL_NULL(&meta);
	if (memcmp(path, "data:", 5)) {
		return NULL;
	}

	path += 5;
	dlen = strlen(path);

	if (dlen >= 2 && path[0] == '/' && path[1] == '/') {
		dlen -= 2;
		path += 2;
	}

	if ((comma = memchr(path, ',', dlen)) == NULL) {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: no comma in URL");
		return NULL;
	}

	if (comma != path) {
		/* meta info */
		mlen = comma - path;
		dlen -= mlen;
		semi = memchr(path, ';', mlen);
		sep = memchr(path, '/', mlen);

		if (!semi && !sep) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: illegal media type");
			return NULL;
		}

		array_init(&meta);
		if (!semi) { /* there is only a mime type */
			add_assoc_stringl(&meta, "mediatype", (char *) path, mlen);
			mlen = 0;
		} else if (sep && sep < semi) { /* there is a mime type */
			plen = semi - path;
			add_assoc_stringl(&meta, "mediatype", (char *) path, plen);
			mlen -= plen;
			path += plen;
		} else if (semi != path || mlen != sizeof(";base64")-1 || memcmp(path, ";base64", sizeof(";base64")-1)) { /* must be error since parameters are only allowed after mediatype */
			zval_ptr_dtor(&meta);
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: illegal media type");
			return NULL;
		}
		/* get parameters and potentially ';base64' */
		while(semi && (semi == path)) {
			path++;
			mlen--;
			sep = memchr(path, '=', mlen);
			semi = memchr(path, ';', mlen);
			if (!sep || (semi && semi < sep)) { /* must be ';base64' or failure */
				if (mlen != sizeof("base64")-1 || memcmp(path, "base64", sizeof("base64")-1)) {
					/* must be error since parameters are only allowed after mediatype and we have no '=' sign */
					zval_ptr_dtor(&meta);
					pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: illegal parameter");
					return NULL;
				}
				base64 = 1;
				mlen -= sizeof("base64") - 1;
				path += sizeof("base64") - 1;
				break;
			}
			/* found parameter ... the heart of cs ppl lies in +1/-1 or was it +2 this time? */
			plen = sep - path;
			vlen = (semi ? (size_t)(semi - sep) : (mlen - plen)) - 1 /* '=' */;
			if (plen != sizeof("mediatype")-1 || memcmp(path, "mediatype", sizeof("mediatype")-1)) {
				add_assoc_stringl_ex(&meta, path, plen, sep + 1, vlen);
			}
			plen += vlen + 1;
			mlen -= plen;
			path += plen;
		}
		if (mlen) {
			zval_ptr_dtor(&meta);
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: illegal URL");
			return NULL;
		}
	} else {
		array_init(&meta);
	}
	add_assoc_bool(&meta, "base64", base64);

	/* skip ',' */
	comma++;
	dlen--;

	if (base64) {
		base64_comma = php_base64_decode_ex((const unsigned char *)comma, dlen, 1);
		if (!base64_comma) {
			zval_ptr_dtor(&meta);
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "rfc2397: unable to decode");
			return NULL;
		}
		base64_tmp = zend_string_dup(base64_comma, 1);
		zend_string_free(base64_comma);
		base64_comma = base64_tmp;

		comma = ZSTR_VAL(base64_comma);
		ilen = ZSTR_LEN(base64_comma);
	} else {
		comma = strndup(comma, dlen);
		dlen = php_url_decode(comma, dlen);
		ilen = dlen;
	}

	if ((threaded_stream = pthreads_stream_temp_create(0, ~0u, ce)) != NULL) {
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
		/* store data */
		//pthreads_stream_temp_write(threaded_stream, comma, ilen);
		//pthreads_stream_temp_seek(threaded_stream, 0, SEEK_SET, &newoffs);
		/* set special stream stuff (enforce exact mode) */
		vlen = strlen(mode);
		if (vlen >= sizeof(stream->mode)) {
			vlen = sizeof(stream->mode) - 1;
		}
		memcpy(stream->mode, mode, vlen);
		stream->mode[vlen] = '\0';
		stream->ops = &pthreads_stream_rfc2397_ops;
		ts = (pthreads_stream_temp_data*)stream->abstract;
		assert(ts != NULL);
		ts->mode = mode && mode[0] == 'r' && mode[1] != '+' ? PTHREADS_TEMP_STREAM_READONLY : 0;
		ZVAL_COPY_VALUE(&ts->meta, &meta);
	}

	if (base64_comma) {
		zend_string_free(base64_comma);
	} else {
		free(comma);
	}

	return threaded_stream;
}

const pthreads_stream_wrapper_ops pthreads_stream_rfc2397_wops = {
	pthreads_stream_url_wrap_rfc2397,
	NULL, /* close */ NULL, /* fstat */
	NULL, /* stat */ NULL, /* opendir */
	"RFC2397",
	NULL, /* unlink */ NULL, /* rename */
	NULL, /* mkdir */ NULL, /* rmdir */
	NULL, /* stream_metadata */
};

#endif
