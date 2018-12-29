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
#ifndef HAVE_PTHREADS_FILE_H
#define HAVE_PTHREADS_FILE_H

#define PTHREADS_META_DEF_BUFSIZE 8192

#define PTHREADS_FILE_USE_INCLUDE_PATH 1
#define PTHREADS_FILE_IGNORE_NEW_LINES 2
#define PTHREADS_FILE_SKIP_EMPTY_LINES 4
#define PTHREADS_FILE_APPEND 8
#define PTHREADS_FILE_NO_DEFAULT_CONTEXT 16

/* See http://www.w3.org/TR/html4/intro/sgmltut.html#h-3.2.2 */
#define PTHREADS_META_HTML401_CHARS "-_.:"

typedef struct _pthreads_meta_tags_data {
	pthreads_stream_t *stream;
	int ulc;
	int lc;
	char *input_buffer;
	char *token_data;
	int token_len;
	int in_meta;
} pthreads_meta_tags_data;

php_meta_tags_token pthreads_next_meta_token(pthreads_meta_tags_data *);


int pthreads_copy_file_ctx(const char *src, const char *dest, int src_flg, pthreads_stream_context_t *threaded_ctx);
size_t pthreads_fputcsv(pthreads_stream_t *threaded_stream, zval *fields, char delimiter, char enclosure, char escape_char);
void pthreads_fgetcsv(pthreads_stream_t *threaded_stream, char delimiter, char enclosure, char escape_char, size_t buf_len, char *buf, zval *return_value);

#endif
