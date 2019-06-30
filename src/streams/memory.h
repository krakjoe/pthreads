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
#ifndef HAVE_PTHREADS_STREAMS_MEMORY_H
#define HAVE_PTHREADS_STREAMS_MEMORY_H

#define PTHREADS_STREAM_MAX_MEM	2 * 1024 * 1024

#define PTHREADS_TEMP_STREAM_DEFAULT     0x0
#define PTHREADS_TEMP_STREAM_READONLY    0x1
#define PTHREADS_TEMP_STREAM_TAKE_BUFFER 0x2
#define PTHREADS_TEMP_STREAM_APPEND      0x4

#define pthreads_stream_memory_create(mode, ce) _pthreads_stream_memory_create((mode), (ce))
#define pthreads_stream_memory_open(mode, buf, length, ce) _pthreads_stream_memory_open((mode), (buf), (length), (ce))
#define pthreads_stream_memory_get_buffer(stream, length) _pthreads_stream_memory_get_buffer((stream), (length))

#define pthreads_stream_temp_new() pthreads_stream_temp_create(PTHREADS_TEMP_STREAM_DEFAULT, PTHREADS_STREAM_MAX_MEM, NULL)
#define pthreads_stream_temp_create(mode, max_memory_usage, ce) _pthreads_stream_temp_create((mode), (max_memory_usage), (ce))
#define pthreads_stream_temp_create_ex(mode, max_memory_usage, tmpdir, ce) _pthreads_stream_temp_create_ex((mode), (max_memory_usage), (tmpdir), (ce))
#define pthreads_stream_temp_open(mode, max_memory_usage, buf, length, ce) _pthreads_stream_temp_open((mode), (max_memory_usage), (buf), (length), (ce))

pthreads_stream_t *_pthreads_stream_memory_create(int mode, zend_class_entry *ce);
pthreads_stream_t *_pthreads_stream_memory_open(int mode, char *buf, size_t length, zend_class_entry *ce);
char *_pthreads_stream_memory_get_buffer(pthreads_stream_t *threaded_stream, size_t *length);

pthreads_stream_t *_pthreads_stream_temp_create(int mode, size_t max_memory_usage, zend_class_entry *ce);
pthreads_stream_t *_pthreads_stream_temp_create_ex(int mode, size_t max_memory_usage, const char *tmpdir, zend_class_entry *ce);
pthreads_stream_t *_pthreads_stream_temp_open(int mode, size_t max_memory_usage, char *buf, size_t length, zend_class_entry *ce);

pthreads_stream_t * pthreads_stream_url_wrap_rfc2397(pthreads_stream_wrapper_t *threaded_wrapper, const char *path,
												const char *mode, int options, zend_string **opened_path,
												pthreads_stream_context_t *threaded_context, zend_class_entry *ce);

int pthreads_stream_mode_from_str(const char *mode);
const char *_pthreads_stream_mode_to_str(int mode);

extern const pthreads_stream_ops pthreads_stream_memory_ops;
extern const pthreads_stream_ops pthreads_stream_temp_ops;
extern const pthreads_stream_ops pthreads_stream_rfc2397_ops;
extern const pthreads_stream_wrapper_ops pthreads_stream_rfc2397_wops;

#define PTHREADS_STREAM_IS_MEMORY &pthreads_stream_memory_ops
#define PTHREADS_STREAM_IS_TEMP   &pthreads_stream_temp_ops

#endif
