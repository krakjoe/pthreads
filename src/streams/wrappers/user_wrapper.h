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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER_H
#define HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER_H

struct pthreads_user_stream_wrapper {
	char * protoname;
	zend_string *classname;
	pthreads_stream_wrapper_t *threaded_wrapper;
};

struct _pthreads_userstream_data {
	struct pthreads_user_stream_wrapper *wrapper;
	zval object;
};
typedef struct _pthreads_userstream_data pthreads_userstream_data_t;

extern const pthreads_stream_wrapper_ops pthreads_user_stream_wops;
extern const pthreads_stream_ops pthreads_stream_userspace_ops;
extern const pthreads_stream_ops pthreads_stream_userspace_dir_ops;
#define PTHREADS_STREAM_IS_USERSPACE &pthreads_stream_userspace_ops
#define PTHREADS_STREAM_IS_USERSPACE_DIR &pthreads_stream_userspace_dir_ops

#endif
