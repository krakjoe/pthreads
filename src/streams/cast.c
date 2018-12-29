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
#ifndef HAVE_PTHREADS_STREAMS_CAST
#define HAVE_PTHREADS_STREAMS_CAST

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_MEMORY_H
#	include <src/streams/memory.h>
#endif

/* Under BSD, emulate fopencookie using funopen */
#if defined(HAVE_FUNOPEN) && !defined(HAVE_PTHREADS_FOPENCOOKIE)

/* NetBSD 6.0+ uses off_t instead of fpos_t in funopen */
# if defined(__NetBSD__) && (__NetBSD_Version__ >= 600000000)
#  define PHP_FPOS_T off_t
# else
#  define PHP_FPOS_T fpos_t
# endif

typedef struct {
	int (*reader)(void *, char *, int);
	int (*writer)(void *, const char *, int);
	PHP_FPOS_T (*seeker)(void *, PHP_FPOS_T, int);
	int (*closer)(void *);
} COOKIE_IO_FUNCTIONS_T;

FILE *fopencookie(void *cookie, const char *mode, COOKIE_IO_FUNCTIONS_T *funcs)
{
	return funopen(cookie, funcs->reader, funcs->writer, funcs->seeker, funcs->closer);
}
# define HAVE_PTHREADS_FOPENCOOKIE 1
# define EMULATE_FOPENCOOKIE 1
# define PHP_STREAM_COOKIE_FUNCTIONS	&stream_cookie_functions
#elif defined(HAVE_PTHREADS_FOPENCOOKIE)
# define PHP_STREAM_COOKIE_FUNCTIONS	stream_cookie_functions
#endif

/* {{{ STDIO with fopencookie */
#if defined(EMULATE_FOPENCOOKIE)
/* use our fopencookie emulation */
static int pthreads_stream_cookie_reader(void *cookie, char *buffer, int size)
{
	int ret;

	ret = pthreads_stream_read((pthreads_stream_t*)cookie, buffer, size);
	return ret;
}

static int pthreads_stream_cookie_writer(void *cookie, const char *buffer, int size)
{

	return pthreads_stream_write((pthreads_stream_t *)cookie, (char *)buffer, size);
}

static PHP_FPOS_T pthreads_stream_cookie_seeker(void *cookie, zend_off_t position, int whence)
{

	return (PHP_FPOS_T)pthreads_stream_seek((pthreads_stream_t *)cookie, position, whence);
}

static int pthreads_stream_cookie_closer(void *cookie)
{
	pthreads_stream_t *threaded_stream = (pthreads_stream_t*)cookie;
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	/* prevent recursion */
	stream->fclose_stdiocast = PTHREADS_STREAM_FCLOSE_NONE;
	return pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
}
#elif defined(HAVE_PTHREADS_FOPENCOOKIE)
static ssize_t pthreads_stream_cookie_reader(void *cookie, char *buffer, size_t size)
{
	ssize_t ret;

	ret = pthreads_stream_read(((pthreads_stream_t *)cookie), buffer, size);
	return ret;
}

static ssize_t pthreads_stream_cookie_writer(void *cookie, const char *buffer, size_t size)
{
	return pthreads_stream_write(((pthreads_stream_t *)cookie), (char *)buffer, size);
}

# ifdef COOKIE_SEEKER_USES_OFF64_T
static int pthreads_stream_cookie_seeker(void *cookie, __off64_t *position, int whence)
{
	*position = pthreads_stream_seek((pthreads_stream_t *)cookie, (zend_off_t)*position, whence);

	if (*position == -1) {
		return -1;
	}
	return 0;
}
# else
static int pthreads_stream_cookie_seeker(void *cookie, zend_off_t position, int whence)
{
	return pthreads_stream_seek((pthreads_stream_t *)cookie, position, whence);
}
# endif

static int pthreads_stream_cookie_closer(void *cookie)
{
	pthreads_stream_t *threaded_stream = (pthreads_stream_t *)cookie;
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	/* prevent recursion */
	stream->fclose_stdiocast = PTHREADS_STREAM_FCLOSE_NONE;
	return pthreads_stream_close_unsafe(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
}
#endif /* elif defined(HAVE_PTHREADS_FOPENCOOKIE) */

#if HAVE_PTHREADS_FOPENCOOKIE
static COOKIE_IO_FUNCTIONS_T stream_cookie_functions =
{
	pthreads_stream_cookie_reader, pthreads_stream_cookie_writer,
	pthreads_stream_cookie_seeker, pthreads_stream_cookie_closer
};
#else
/* TODO: use socketpair() to emulate fopencookie, as suggested by Hartmut ? */
#endif
/* }}} */

/* {{{ pthreads_stream_mode_sanitize_fdopen_fopencookie
 * Result should have at least size 5, e.g. to write wbx+\0 */
void pthreads_stream_mode_sanitize_fdopen_fopencookie(pthreads_stream_t *threaded_stream, char *result) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	/* replace modes not supported by fdopen and fopencookie, but supported
	 * by PHP's fread(), so that their calls won't fail */
	const char *cur_mode = stream->mode;
	int         has_plus = 0,
		        has_bin  = 0,
				i,
				res_curs = 0;

	if (cur_mode[0] == 'r' || cur_mode[0] == 'w' || cur_mode[0] == 'a') {
		result[res_curs++] = cur_mode[0];
	} else {
		/* assume cur_mode[0] is 'c' or 'x'; substitute by 'w', which should not
		 * truncate anything in fdopen/fopencookie */
		result[res_curs++] = 'w';

		/* x is allowed (at least by glibc & compat), but not as the 1st mode
		 * as in PHP and in any case is (at best) ignored by fdopen and fopencookie */
	}

	/* assume current mode has at most length 4 (e.g. wbn+) */
	for (i = 1; i < 4 && cur_mode[i] != '\0'; i++) {
		if (cur_mode[i] == 'b') {
			has_bin = 1;
		} else if (cur_mode[i] == '+') {
			has_plus = 1;
		}
		/* ignore 'n', 't' or other stuff */
	}

	if (has_bin) {
		result[res_curs++] = 'b';
	}
	if (has_plus) {
		result[res_curs++] = '+';
	}

	result[res_curs] = '\0';
}
/* }}} */

/* {{{ pthreads_stream_cast */
int _pthreads_stream_cast(pthreads_stream_t *threaded_stream, int castas, void **ret, int show_err) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	int flags = castas & PTHREADS_STREAM_CAST_MASK;
	castas &= ~PTHREADS_STREAM_CAST_MASK;

	if(stream_lock(threaded_stream)) {
		/* synchronize our buffer (if possible) */
		if (ret && castas != PTHREADS_STREAM_AS_FD_FOR_SELECT) {
			pthreads_stream_flush(threaded_stream);
			if (stream->ops->seek && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0) {
				zend_off_t dummy;

				stream->ops->seek(threaded_stream, stream->position, SEEK_SET, &dummy);
				stream->readpos = stream->writepos = 0;
			}
		}

		/* filtered streams can only be cast as stdio, and only when fopencookie is present */

		if (castas == PTHREADS_STREAM_AS_STDIO) {
			if (stream->stdiocast) {
				if (ret) {
					*(FILE**)ret = stream->stdiocast;
				}
				goto exit_success;
			}

			/* if the stream is a stdio stream let's give it a chance to respond
			 * first, to avoid doubling up the layers of stdio with an fopencookie */
			if (pthreads_stream_is(threaded_stream, PTHREADS_STREAM_IS_STDIO) &&
				stream->ops->cast &&
				!pthreads_stream_is_filtered(threaded_stream) &&
				stream->ops->cast(threaded_stream, castas, ret) == SUCCESS
			) {
				goto exit_success;
			}

#if HAVE_PTHREADS_FOPENCOOKIE
			/* if just checking, say yes we can be a FILE*, but don't actually create it yet */
			if (ret == NULL) {
				goto exit_success;
			}

			{
				char fixed_mode[5];
				pthreads_stream_mode_sanitize_fdopen_fopencookie(threaded_stream, fixed_mode);
				*(FILE**)ret = fopencookie(threaded_stream, fixed_mode, PHP_STREAM_COOKIE_FUNCTIONS);
			}

			if (*ret != NULL) {
				zend_off_t pos;

				stream->fclose_stdiocast = PTHREADS_STREAM_FCLOSE_FOPENCOOKIE;

				/* If the stream position is not at the start, we need to force
				 * the stdio layer to believe it's real location. */
				pos = pthreads_stream_tell(threaded_stream);
				if (pos > 0) {
					zend_fseek(*ret, pos, SEEK_SET);
				}

				goto exit_success;
			}
			stream_unlock(threaded_stream);

			/* must be either:
				a) programmer error
				b) no memory
				-> lets bail
			*/
			php_error_docref(NULL, E_ERROR, "fopencookie failed");
			return FAILURE;
#endif

			if (!pthreads_stream_is_filtered(threaded_stream) && stream->ops->cast && stream->ops->cast(threaded_stream, castas, NULL) == SUCCESS) {
				if (FAILURE == stream->ops->cast(threaded_stream, castas, ret)) {
					stream_unlock(threaded_stream);
					return FAILURE;
				}
				goto exit_success;
			} else if (flags & PTHREADS_STREAM_CAST_TRY_HARD) {
				pthreads_stream_t *threaded_newstream;

				threaded_newstream = pthreads_stream_fopen_tmpfile();
				if (threaded_newstream) {
					int retcopy = pthreads_stream_copy_to_stream_ex(threaded_stream, threaded_newstream, PTHREADS_STREAM_COPY_ALL, NULL);

					if (retcopy != SUCCESS) {
						pthreads_stream_close(threaded_newstream, PTHREADS_STREAM_FREE_CLOSE);
					} else {
						int retcast = pthreads_stream_cast(threaded_newstream, castas | flags, (void **)ret, show_err);

						if (retcast == SUCCESS) {
							rewind(*(FILE**)ret);
						}

						/* do some specialized cleanup */
						if ((flags & PTHREADS_STREAM_CAST_RELEASE)) {
							pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE_CASTED);
						}
						stream_unlock(threaded_stream);

						/* TODO: we probably should be setting .stdiocast and .fclose_stdiocast or
						 * we may be leaking the FILE*. Needs investigation, though. */
						return retcast;
					}
				}
			}
		}

		if (pthreads_stream_is_filtered(threaded_stream)) {
			stream_unlock(threaded_stream);
			php_error_docref(NULL, E_WARNING, "cannot cast a filtered stream on this system");
			return FAILURE;
		} else if (stream->ops->cast && stream->ops->cast(threaded_stream, castas, ret) == SUCCESS) {
			goto exit_success;
		}
		stream_unlock(threaded_stream);
	}

	if (show_err) {
		/* these names depend on the values of the PTHREADS_STREAM_AS_XXX defines in streams.h */
		static const char *cast_names[4] = {
			"STDIO FILE*",
			"File Descriptor",
			"Socket Descriptor",
			"select()able descriptor"
		};

		php_error_docref(NULL, E_WARNING, "cannot represent a stream of type %s as a %s", stream->ops->label, cast_names[castas]);
	}

	return FAILURE;

exit_success:

	if ((stream->writepos - stream->readpos) > 0 &&
		stream->fclose_stdiocast != PTHREADS_STREAM_FCLOSE_FOPENCOOKIE &&
		(flags & PTHREADS_STREAM_CAST_INTERNAL) == 0
	) {
		/* the data we have buffered will be lost to the third party library that
		 * will be accessing the stream.  Emit a warning so that the end-user will
		 * know that they should try something else */

		php_error_docref(NULL, E_WARNING, ZEND_LONG_FMT " bytes of buffered data lost during stream conversion!", (zend_long)(stream->writepos - stream->readpos));
	}

	if (castas == PTHREADS_STREAM_AS_STDIO && ret) {
		stream->stdiocast = *(FILE**)ret;
	}

	if (flags & PTHREADS_STREAM_CAST_RELEASE) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE_CASTED);
	}
	stream_unlock(threaded_stream);

	return SUCCESS;

}
/* }}} */

/* {{{ pthreads_stream_open_wrapper_as_file */
FILE * _pthreads_stream_open_wrapper_as_file(char *path, char *mode, int options, zend_string **opened_path)
{
	FILE *fp = NULL;
	pthreads_stream_t *threaded_stream = NULL;

	threaded_stream = pthreads_stream_open_wrapper(path, mode, options | PTHREADS_STREAM_WILL_CAST, opened_path);

	if (threaded_stream == NULL) {
		return NULL;
	}

	if (pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_STDIO|PTHREADS_STREAM_CAST_TRY_HARD|PTHREADS_STREAM_CAST_RELEASE, (void**)&fp, PTHREADS_REPORT_ERRORS) == FAILURE) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
		if (opened_path && *opened_path) {
			zend_string_release(*opened_path);
		}
		return NULL;
	}
	return fp;
}
/* }}} */

/* {{{ pthreads_stream_make_seekable */
int _pthreads_stream_make_seekable(pthreads_stream_t *threaded_origstream, pthreads_stream_t **threaded_newstream, int flags) {
	pthreads_stream *origstream = PTHREADS_FETCH_STREAMS_STREAM(threaded_origstream),
			*newstream = PTHREADS_FETCH_STREAMS_STREAM(*threaded_newstream);

	if (threaded_newstream == NULL) {
		return PTHREADS_STREAM_FAILED;
	}
	*threaded_newstream = NULL;

	if (((flags & PTHREADS_STREAM_FORCE_CONVERSION) == 0) && origstream->ops->seek != NULL) {
		*threaded_newstream = threaded_origstream;
		return PTHREADS_STREAM_UNCHANGED;
	}

	/* Use a tmpfile and copy the old streams contents into it */

	if (flags & PTHREADS_STREAM_PREFER_STDIO) {
		*threaded_newstream = pthreads_stream_fopen_tmpfile();
	} else {
		*threaded_newstream = pthreads_stream_temp_new();
	}

	if (*threaded_newstream == NULL) {
		return PTHREADS_STREAM_FAILED;
	}

	if (pthreads_stream_copy_to_stream_ex(threaded_origstream, *threaded_newstream, PTHREADS_STREAM_COPY_ALL, NULL) != SUCCESS) {
		pthreads_stream_close(*threaded_newstream, PTHREADS_STREAM_FREE_CLOSE);
		*threaded_newstream = NULL;
		return PTHREADS_STREAM_CRITICAL;
	}

	pthreads_stream_close(threaded_origstream, PTHREADS_STREAM_FREE_CLOSE);
	pthreads_stream_seek(*threaded_newstream, 0, SEEK_SET);

	return PTHREADS_STREAM_RELEASED;
}
/* }}} */

#endif
