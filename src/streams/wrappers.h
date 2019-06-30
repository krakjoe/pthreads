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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_H
#define HAVE_PTHREADS_STREAMS_WRAPPERS_H

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

int pthreads_init_stream_wrappers();
int pthreads_shutdown_stream_wrappers();

/* pushes an error message onto the stack for a wrapper instance */
void pthreads_stream_wrapper_log_error(const pthreads_stream_wrapper_t *threaded_wrapper, int options, const char *fmt, ...) PHP_ATTRIBUTE_FORMAT(printf, 3, 4);

void pthreads_stream_wrapper_free(pthreads_stream_wrapper *wrapper);
pthreads_stream_wrapper *pthreads_stream_wrapper_alloc(void);


int pthreads_register_url_stream_wrapper(const char *protocol, pthreads_stream_wrapper_t *threaded_wrapper);
int pthreads_unregister_url_stream_wrapper(const char *protocol);

pthreads_stream_t *_pthreads_stream_open_wrapper_ex(const char *path, const char *mode, int options, zend_string **opened_path,
		pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
pthreads_stream_wrapper_t *pthreads_stream_locate_url_wrapper(const char *path, const char **path_for_open, int options);

int _pthreads_stream_mkdir(const char *path, int mode, int options, pthreads_stream_context_t *threaded_context);
#define pthreads_stream_mkdir(path, mode, options, threaded_context)	_pthreads_stream_mkdir(path, mode, options, threaded_context)

int _pthreads_stream_rmdir(const char *path, int options, pthreads_stream_context_t *threaded_context);
#define pthreads_stream_rmdir(path, options, threaded_context)	_pthreads_stream_rmdir(path, options, threaded_context)

int _pthreads_stream_stat_path(const char *path, int flags, pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context);
#define pthreads_stream_stat_path(path, ssb)	_pthreads_stream_stat_path((path), 0, (ssb), NULL)
#define pthreads_stream_stat_path_ex(path, flags, ssb, threaded_context)	_pthreads_stream_stat_path((path), (flags), (ssb), (threaded_context))

pthreads_stream_t *_pthreads_stream_opendir(const char *path, int options, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
#define pthreads_stream_opendir(path, options, context)	_pthreads_stream_opendir((path), (options), (threaded_context), NULL)

pthreads_stream_dirent *_pthreads_stream_readdir(pthreads_stream_t *threaded_dirstream, pthreads_stream_dirent *ent);
#define pthreads_stream_readdir(threaded_dirstream, dirent)	_pthreads_stream_readdir((threaded_dirstream), (dirent))

#define pthreads_stream_open_wrapper(path, mode, options, opened)	\
	_pthreads_stream_open_wrapper_ex((path), (mode), (options), (opened), NULL, NULL)
#define pthreads_stream_open_wrapper_ex(path, mode, options, opened, threaded_context, ce)	\
	_pthreads_stream_open_wrapper_ex((path), (mode), (options), (opened), (threaded_context), (ce))

#endif
