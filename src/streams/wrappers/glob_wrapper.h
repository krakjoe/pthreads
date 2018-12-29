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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_GLOB_WRAPPER_H
#define HAVE_PTHREADS_STREAMS_WRAPPERS_GLOB_WRAPPER_H

extern const pthreads_stream_ops pthreads_glob_stream_ops;
extern const pthreads_stream_wrapper_ops  pthreads_glob_stream_wrapper_ops;

char* _pthreads_glob_stream_get_path(pthreads_stream_t *threaded_stream, int copy, size_t *plen);
#define pthreads_glob_stream_get_path(threaded_stream, copy, plen)	_pthreads_glob_stream_get_path((threaded_stream), (copy), (plen))

char* _pthreads_glob_stream_get_pattern(pthreads_stream_t *threaded_stream, int copy, size_t *plen);
#define pthreads_glob_stream_get_pattern(threaded_stream, copy, plen)	_pthreads_glob_stream_get_pattern((threaded_stream), (copy), (plen))

int   _pthreads_glob_stream_get_count(pthreads_stream_t *threaded_stream, int *pflags);
#define pthreads_glob_stream_get_count(threaded_stream, pflags)	_pthreads_glob_stream_get_count((threaded_stream), (pflags))

#endif
