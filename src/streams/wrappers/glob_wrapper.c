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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_GLOB_WRAPPER
#define HAVE_PTHREADS_STREAMS_WRAPPERS_GLOB_WRAPPER

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_INTERNAL_H
#	include <src/streams/internal.h>
#endif

#ifdef HAVE_GLOB
# ifndef PHP_WIN32
#  include <glob.h>
# else
#  include "win32/glob.h"
# endif

#ifndef PTHREADS_GLOB_ONLYDIR
#define PTHREADS_GLOB_ONLYDIR (1<<30)
#define PTHREADS_GLOB_FLAGMASK (~PTHREADS_GLOB_ONLYDIR)
#else
#define PTHREADS_GLOB_FLAGMASK (~0)
#endif

typedef struct {
	glob_t   glob;
	size_t   index;
	int      flags;
	char     *path;
	size_t   path_len;
	char     *pattern;
	size_t   pattern_len;
} pthreads_glob_s_t;

/* {{{ */
char* _pthreads_glob_stream_get_path(pthreads_stream_t *threaded_stream, int copy, size_t *plen) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;
	char *ret = NULL;

	if(stream_lock(threaded_stream)) {
		if (pglob && pglob->path) {
			if (plen) {
				*plen = pglob->path_len;
			}
			if (copy) {
				ret = strndup(pglob->path, pglob->path_len);
			} else {
				ret = pglob->path;
			}
			stream_unlock(threaded_stream);
			return ret;
		} else {
			if (plen) {
				*plen = 0;
			}
			stream_unlock(threaded_stream);
			return NULL;
		}
	}
	return ret;
}
/* }}} */

/* {{{ */
char* _pthreads_glob_stream_get_pattern(pthreads_stream_t *threaded_stream, int copy, size_t *plen) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;
	char *ret = NULL;

	if(stream_lock(threaded_stream)) {
		if (pglob && pglob->pattern) {
			if (plen) {
				*plen = pglob->pattern_len;
			}
			if (copy) {
				ret = strndup(pglob->pattern, pglob->pattern_len);
			} else {
				ret = pglob->pattern;
			}
			stream_unlock(threaded_stream);
			return ret;
		} else {
			if (plen) {
				*plen = 0;
			}
			stream_unlock(threaded_stream);
			return NULL;
		}
	}
	return ret;
}
/* }}} */

/* {{{ */
int _pthreads_glob_stream_get_count(pthreads_stream_t *threaded_stream, int *pflags) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;

	if(stream_lock(threaded_stream)) {
		if (pglob) {
			if (pflags) {
				*pflags = pglob->flags;
			}
			int ret = pglob->glob.gl_pathc;
			stream_unlock(threaded_stream);
			return ret;
		} else {
			if (pflags) {
				*pflags = 0;
			}
			stream_unlock(threaded_stream);
			return 0;
		}
	}
	return 0;
}
/* }}} */

/* {{{ */
static void pthreads_glob_stream_path_split(pthreads_glob_s_t *pglob, const char *path, int get_path, const char **p_file) {
	const char *pos, *gpath = path;

	if ((pos = strrchr(path, '/')) != NULL) {
		path = pos+1;
	}
#ifdef PHP_WIN32
	if ((pos = strrchr(path, '\\')) != NULL) {
		path = pos+1;
	}
#endif

	*p_file = path;

	if (get_path) {
		if (pglob->path) {
			free(pglob->path);
		}
		if (path != gpath) {
			path--;
		}
		pglob->path_len = path - gpath;
		pglob->path = strndup(gpath, pglob->path_len);
	}
}
/* }}} */

/* {{{ */
static size_t pthreads_glob_stream_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;
	pthreads_stream_dirent *ent = (pthreads_stream_dirent*)buf;
	const char *path;

	/* avoid problems if someone mis-uses the stream */
	if (count == sizeof(pthreads_stream_dirent) && pglob) {
		if(stream_lock(threaded_stream)) {
			if (pglob->index < (size_t)pglob->glob.gl_pathc) {
				pthreads_glob_stream_path_split(pglob, pglob->glob.gl_pathv[pglob->index++], pglob->flags & GLOB_APPEND, &path);
				PHP_STRLCPY(ent->d_name, path, sizeof(ent->d_name), strlen(path));
				stream_unlock(threaded_stream);
				return sizeof(pthreads_stream_dirent);
			}
			pglob->index = pglob->glob.gl_pathc;

			if (pglob->path) {
				free(pglob->path);
				pglob->path = NULL;
			}
			stream_unlock(threaded_stream);
		}
	}

	return 0;
}
/* }}} */

/* {{{ */
static int pthreads_glob_stream_close(pthreads_stream_t *threaded_stream, int close_handle) {
	return 0;
}
/* }}} */

/* {{{ */
static void pthreads_glob_stream_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;

	if (pglob) {
		pglob->index = 0;
		globfree(&pglob->glob);
		if (pglob->path) {
			free(pglob->path);
		}
		if (pglob->pattern) {
			free(pglob->pattern);
		}
	}
	free(stream->abstract);
}
/* }}} */

/* {{{ */
static int pthreads_glob_stream_rewind(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffs) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_glob_s_t *pglob = (pthreads_glob_s_t *)stream->abstract;

	if (pglob) {
		if(stream_lock(threaded_stream)) {
			pglob->index = 0;
			if (pglob->path) {
				free(pglob->path);
				pglob->path = NULL;
			}
			stream_unlock(threaded_stream);
		}
	}
	return 0;
}
/* }}} */

const pthreads_stream_ops pthreads_glob_stream_ops = {
	NULL, pthreads_glob_stream_read,
	pthreads_glob_stream_close,
	pthreads_glob_stream_free,
	NULL,
	"glob",
	pthreads_glob_stream_rewind,
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

 /* {{{ pthreads_glob_stream_opener */
static pthreads_stream_t *pthreads_glob_stream_opener(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode,
		int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce)
{
	pthreads_glob_s_t *pglob;
	int ret;
	const char *tmp, *pos;

	if (!strncmp(path, "glob://", sizeof("glob://")-1)) {
		path += sizeof("glob://")-1;
		if (opened_path) {
			*opened_path = zend_string_init(path, strlen(path), 1);
		}
	}

	if (((options & PTHREADS_STREAM_DISABLE_OPEN_BASEDIR) == 0) && php_check_open_basedir(path)) {
		return NULL;
	}

	pglob = calloc(sizeof(*pglob), 1);

	if (0 != (ret = glob(path, pglob->flags & PTHREADS_GLOB_FLAGMASK, NULL, &pglob->glob))) {
#ifdef GLOB_NOMATCH
		if (GLOB_NOMATCH != ret)
#endif
		{
			free(pglob);
			return NULL;
		}
	}

	pos = path;
	if ((tmp = strrchr(pos, '/')) != NULL) {
		pos = tmp+1;
	}
#ifdef PHP_WIN32
	if ((tmp = strrchr(pos, '\\')) != NULL) {
		pos = tmp+1;
	}
#endif

	pglob->pattern_len = strlen(pos);
	pglob->pattern = strndup(pos, pglob->pattern_len);

	pglob->flags |= GLOB_APPEND;

	if (pglob->glob.gl_pathc) {
		pthreads_glob_stream_path_split(pglob, pglob->glob.gl_pathv[0], 1, &tmp);
	} else {
		pthreads_glob_stream_path_split(pglob, path, 1, &tmp);
	}

	return PTHREADS_STREAM_CLASS_NEW(&pthreads_glob_stream_ops, pglob, mode, ce);
}
/* }}} */

const pthreads_stream_wrapper_ops  pthreads_glob_stream_wrapper_ops = {
	NULL,
	NULL,
	NULL,
	NULL,
	pthreads_glob_stream_opener,
	"glob",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};

#endif /* HAVE_GLOB */

#endif
