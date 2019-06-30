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
#ifndef HAVE_PTHREADS_STREAMS_INTERNAL_H
#define HAVE_PTHREADS_STREAMS_INTERNAL_H

#define PTHREADS_STREAM_DEBUG 0

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

#define CHUNK_SIZE	8192

#ifdef PHP_WIN32
# ifdef EWOULDBLOCK
#  undef EWOULDBLOCK
# endif
# define EWOULDBLOCK WSAEWOULDBLOCK
# ifdef EMSGSIZE
#  undef EMSGSIZE
# endif
# define EMSGSIZE WSAEMSGSIZE
#endif

/* This functions transforms the first char to 'w' if it's not 'r', 'a' or 'w'
 * and strips any subsequent chars except '+' and 'b'.
 * Use this to sanitize stream->mode if you call e.g. fdopen, fopencookie or
 * any other function that expects standard modes and you allow non-standard
 * ones. result should be a char[5]. */
void pthreads_stream_mode_sanitize_fdopen_fopencookie(pthreads_stream_t *threaded_stream, char *result);

void pthreads_stream_tidy_wrapper_error_log(pthreads_stream_wrapper_t *threaded_wrapper);
void pthreads_stream_display_wrapper_errors(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *caption);

#endif
