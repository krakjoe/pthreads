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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_PLAIN_WRAPPER_H
#define HAVE_PTHREADS_STREAMS_WRAPPERS_PLAIN_WRAPPER_H

/* definitions for the plain files wrapper */

extern pthreads_stream_ops pthreads_stream_stdio_ops;
extern const pthreads_stream_wrapper_ops pthreads_plain_files_wrapper_ops;

/* like fopen, but returns a stream */
pthreads_stream_t *_pthreads_stream_fopen(const char *filename, const char *mode, zend_string **opened_path, int options, zend_class_entry *ce);
#define pthreads_stream_fopen(filename, mode, opened, options)	_pthreads_stream_fopen((filename), (mode), (opened), (options), NULL)

pthreads_stream_t *_pthreads_stream_fopen_with_path(const char *filename, const char *mode, const char *path, zend_string **opened_path, int options);
#define pthreads_stream_fopen_with_path(filename, mode, path, opened)	_pthreads_stream_fopen_with_path((filename), (mode), (path), (opened))

pthreads_stream_t *_pthreads_stream_fopen_from_file(FILE *file, const char *mode);
#define pthreads_stream_fopen_from_file(file, mode)	_pthreads_stream_fopen_from_file((file), (mode))

pthreads_stream_t *_pthreads_stream_fopen_from_fd(int fd, const char *mode);
#define pthreads_stream_fopen_from_fd(fd, mode)	_pthreads_stream_fopen_from_fd((fd), (mode))

pthreads_stream_t *_pthreads_stream_fopen_from_pipe(FILE *file, const char *mode, zend_class_entry *ce);
#define pthreads_stream_fopen_from_pipe(file, mode)	_pthreads_stream_fopen_from_pipe((file), (mode), NULL)

pthreads_stream_t *_pthreads_stream_fopen_tmpfile(int dummy);
#define pthreads_stream_fopen_tmpfile()	_pthreads_stream_fopen_tmpfile(0)

pthreads_stream_t *_pthreads_stream_fopen_temporary_file(const char *dir, const char *pfx, zend_string **opened_path);
#define pthreads_stream_fopen_temporary_file(dir, pfx, opened_path)	_pthreads_stream_fopen_temporary_file((dir), (pfx), (opened_path))

/* This is a utility API for extensions that are opening a stream, converting it
 * to a FILE* and then closing it again.  Be warned that fileno() on the result
 * will most likely fail on systems with fopencookie. */
FILE * _pthreads_stream_open_wrapper_as_file(char * path, char * mode, int options, zend_string **opened_path);
#define pthreads_stream_open_wrapper_as_file(path, mode, options, opened_path) _pthreads_stream_open_wrapper_as_file((path), (mode), (options), (opened_path))

/* parse standard "fopen" modes into open() flags */
int pthreads_stream_parse_fopen_modes(const char *mode, int *open_flags);

#endif
