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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER_H
#define HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER_H

#ifndef HAVE_PTHREADS_URL_H
#	include <src/url.h>
#endif

pthreads_stream_t *pthreads_stream_url_wrap_http(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
		zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
pthreads_stream_t *pthreads_stream_url_wrap_ftp(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
		zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
pthreads_stream_t *pthreads_stream_url_wrap_php(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
		zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);

extern const pthreads_stream_wrapper_ops pthreads_stdio_wops;
extern const pthreads_stream_wrapper_ops pthreads_http_stream_wops;
extern const pthreads_stream_wrapper_ops pthreads_ftp_stream_wops;

#endif
