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
#ifndef HAVE_PTHREADS_FILESTREAM_API
#define HAVE_PTHREADS_FILESTREAM_API

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "ext/standard/flock_compat.h"
#include "ext/standard/scanf.h"
#include "ext/standard/php_string.h"

#ifdef HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

static int flock_values[] = { LOCK_SH, LOCK_EX, LOCK_UN };
#define PHP_META_UNSAFE ".\\+*?[^]$() "

/**
 * FileStream API
 */

void pthreads_streams_api_filestream_lock(zval *object, int act, zend_long operation, zval *wouldblock, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (wouldblock) {
		zval_ptr_dtor(wouldblock);
		ZVAL_LONG(wouldblock, 0);
	}

	/* flock_values contains all possible actions if (operation & 4) we won't block on the lock */
	act = flock_values[act - 1] | (operation & PHP_LOCK_NB ? LOCK_NB : 0);
	if (pthreads_stream_lock(threaded_stream, act)) {
		if (operation && errno == EWOULDBLOCK && wouldblock) {
			ZVAL_LONG(wouldblock, 1);
		}
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

void pthreads_streams_api_filestream_close(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if (PTHREADS_IS_INVALID_STREAM(stream)) {
		php_error_docref(NULL, E_WARNING, "%s was already closed", ZSTR_VAL(Z_OBJCE_P(object)->name));
		RETURN_FALSE;
	}

	if ((stream->flags & PTHREADS_STREAM_FLAG_NO_FCLOSE) != 0) {
		php_error_docref(NULL, E_WARNING, "%s is not a valid stream", ZSTR_VAL(Z_OBJCE_P(object)->name));
		RETURN_FALSE;
	}
	int ret = pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);

	RETURN_LONG(ret);
}

void pthreads_streams_api_filestream_pclose(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int ret = pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);

	RETURN_LONG(ret);
}

void pthreads_streams_api_filestream_eof(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_stream_eof(threaded_stream)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

void pthreads_streams_api_filestream_gets(zval *object, int argc, zend_long len, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	char *buf = NULL;
	size_t line_len = 0;
	zend_string *str;

	if (argc == 0) {
		/* ask streams to give us a buffer of an appropriate size */
		buf = pthreads_stream_get_line(threaded_stream, NULL, 0, &line_len);

		if (buf == NULL) {
			RETURN_NULL();
		}
		// TODO: avoid reallocation ???
		RETVAL_STRINGL(buf, line_len);

		free(buf);
	} else if (argc > 0) {
		if (len <= 0) {
			php_error_docref(NULL, E_WARNING, "Length parameter must be greater than 0");
			RETURN_NULL();
		}

		str = zend_string_alloc(len, 0);
		if (pthreads_stream_get_line(threaded_stream, ZSTR_VAL(str), len, &line_len) == NULL) {
			zend_string_efree(str);
			RETURN_NULL();
		}
		/* resize buffer if it's much larger than the result.
		 * Only needed if the user requested a buffer size. */
		if (line_len < (size_t)len / 2) {
			str = zend_string_truncate(str, line_len, 0);
		} else {
			ZSTR_LEN(str) = line_len;
		}
		RETURN_NEW_STR(str);
	}
}

void pthreads_streams_api_filestream_getc(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	char buf[2];
	int result;

	result = pthreads_stream_getc(threaded_stream);

	if (result == EOF) {
		RETURN_NULL();
	}
	buf[0] = result;
	buf[1] = '\0';

	RETURN_STRINGL(buf, 1);
}

void pthreads_streams_api_filestream_getss(zval *object, int argc, zend_long bytes, zend_string *allowed_tags, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	size_t len = 0;
	size_t actual_len, retval_len;
	char *buf = NULL, *retval;

	if (argc >= 1) {
		if (bytes <= 0) {
			php_error_docref(NULL, E_WARNING, "Length parameter must be greater than 0");
			RETURN_FALSE;
		}

		len = (size_t) bytes;
		buf = calloc(sizeof(char), (len + 1));
	}

	if(stream_lock(threaded_stream)) {
		if ((retval = pthreads_stream_get_line(threaded_stream, buf, len, &actual_len)) == NULL)	{
			if (buf != NULL) {
				free(buf);
			}
			stream_unlock(threaded_stream);
			RETURN_NULL();
		}

		retval_len = php_strip_tags(retval, actual_len, &stream->fgetss_state, ZSTR_VAL(allowed_tags), ZSTR_LEN(allowed_tags));

		// TODO: avoid reallocation ???
		RETVAL_STRINGL(retval, retval_len);

		stream_unlock(threaded_stream);
	}

	if(retval != NULL) {
		free(retval);
	}
}

void pthreads_streams_api_filestream_scanf(zval *object, zend_string *format, zval *args, int argc, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int result;
	size_t format_len;
	zval *file_handle;
	char *buf;
	size_t len;

	buf = pthreads_stream_get_line(threaded_stream, NULL, 0, &len);
	if (buf == NULL) {
		RETURN_FALSE;
	}

	result = php_sscanf_internal(buf, ZSTR_VAL(format), argc, args, 0, return_value);

	free(buf);

	if (SCAN_ERROR_WRONG_PARAM_COUNT == result) {
		WRONG_PARAM_COUNT;
	}
}

void pthreads_streams_api_filestream_write(zval *object, int argc, zend_string *input, zend_long maxlen, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	size_t inputlen = ZSTR_LEN(input);
	size_t ret, num_bytes;

	if (argc == 1) {
		num_bytes = inputlen;
	} else if (maxlen <= 0) {
		num_bytes = 0;
	} else {
		num_bytes = MIN((size_t) maxlen, inputlen);
	}

	if (!num_bytes) {
		RETURN_LONG(0);
	}
	ret = pthreads_stream_write(threaded_stream, ZSTR_VAL(input), num_bytes);

	RETURN_LONG(ret);
}

void pthreads_streams_api_filestream_flush(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int ret = pthreads_stream_flush(threaded_stream);
	if (ret) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

void pthreads_streams_api_filestream_rewind(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	int ret = pthreads_stream_rewind(threaded_stream);
	if (ret) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}

void pthreads_streams_api_filestream_tell(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_LONG(pthreads_stream_tell(threaded_stream));
}

void pthreads_streams_api_filestream_seek(zval *object, zend_long offset, zend_long whence, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_LONG(pthreads_stream_seek(threaded_stream, offset, (int) whence));
}

void pthreads_streams_api_filestream_passthru(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_LONG(pthreads_stream_passthru(threaded_stream));
}

void pthreads_streams_api_filestream_truncate(zval *object, zend_long size, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (!pthreads_stream_truncate_supported(threaded_stream)) {
		php_error_docref(NULL, E_WARNING, "Can't truncate this stream!");
		RETURN_FALSE;
	}

	RETURN_BOOL(0 == pthreads_stream_truncate_set_size(threaded_stream, size));
}

void pthreads_streams_api_filestream_stat(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zval stat_dev, stat_ino, stat_mode, stat_nlink, stat_uid, stat_gid, stat_rdev,
		 stat_size, stat_atime, stat_mtime, stat_ctime, stat_blksize, stat_blocks;

	pthreads_stream_statbuf stat_ssb;
	char *stat_sb_names[13] = {
		"dev", "ino", "mode", "nlink", "uid", "gid", "rdev",
		"size", "atime", "mtime", "ctime", "blksize", "blocks"
	};

	if (pthreads_stream_stat(threaded_stream, &stat_ssb)) {
		RETURN_FALSE;
	}

	array_init(return_value);

	ZVAL_LONG(&stat_dev, stat_ssb.sb.st_dev);
	ZVAL_LONG(&stat_ino, stat_ssb.sb.st_ino);
	ZVAL_LONG(&stat_mode, stat_ssb.sb.st_mode);
	ZVAL_LONG(&stat_nlink, stat_ssb.sb.st_nlink);
	ZVAL_LONG(&stat_uid, stat_ssb.sb.st_uid);
	ZVAL_LONG(&stat_gid, stat_ssb.sb.st_gid);
#ifdef HAVE_STRUCT_STAT_ST_RDEV
# ifdef PHP_WIN32
	/* It is unsigned, so if a negative came from userspace, it'll
	   convert to UINT_MAX, but we wan't to keep the userspace value.
	   Almost the same as in php_fstat. This is ugly, but otherwise
	   we would have to maintain a fully compatible struct stat. */
	if ((int)stat_ssb.sb.st_rdev < 0) {
		ZVAL_LONG(&stat_rdev, (int)stat_ssb.sb.st_rdev);
	} else {
		ZVAL_LONG(&stat_rdev, stat_ssb.sb.st_rdev);
	}
# else
	ZVAL_LONG(&stat_rdev, stat_ssb.sb.st_rdev);
# endif
#else
	ZVAL_LONG(&stat_rdev, -1);
#endif
	ZVAL_LONG(&stat_size, stat_ssb.sb.st_size);
	ZVAL_LONG(&stat_atime, stat_ssb.sb.st_atime);
	ZVAL_LONG(&stat_mtime, stat_ssb.sb.st_mtime);
	ZVAL_LONG(&stat_ctime, stat_ssb.sb.st_ctime);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	ZVAL_LONG(&stat_blksize, stat_ssb.sb.st_blksize);
#else
	ZVAL_LONG(&stat_blksize,-1);
#endif
#ifdef HAVE_ST_BLOCKS
	ZVAL_LONG(&stat_blocks, stat_ssb.sb.st_blocks);
#else
	ZVAL_LONG(&stat_blocks,-1);
#endif
	/* Store numeric indexes in proper order */
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_dev);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_ino);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_mode);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_nlink);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_uid);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_gid);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_rdev);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_size);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_atime);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_mtime);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_ctime);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_blksize);
	zend_hash_next_index_insert(Z_ARRVAL_P(return_value), &stat_blocks);

	/* Store string indexes referencing the same zval*/
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[0], strlen(stat_sb_names[0]), &stat_dev);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[1], strlen(stat_sb_names[1]), &stat_ino);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[2], strlen(stat_sb_names[2]), &stat_mode);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[3], strlen(stat_sb_names[3]), &stat_nlink);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[4], strlen(stat_sb_names[4]), &stat_uid);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[5], strlen(stat_sb_names[5]), &stat_gid);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[6], strlen(stat_sb_names[6]), &stat_rdev);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[7], strlen(stat_sb_names[7]), &stat_size);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[8], strlen(stat_sb_names[8]), &stat_atime);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[9], strlen(stat_sb_names[9]), &stat_mtime);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[10], strlen(stat_sb_names[10]), &stat_ctime);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[11], strlen(stat_sb_names[11]), &stat_blksize);
	zend_hash_str_add_new(Z_ARRVAL_P(return_value), stat_sb_names[12], strlen(stat_sb_names[12]), &stat_blocks);
}

void pthreads_streams_api_filestream_read(zval *object, zend_long len, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	ZVAL_NEW_STR(return_value, zend_string_alloc(len, 0));
	Z_STRLEN_P(return_value) = pthreads_stream_read(threaded_stream, Z_STRVAL_P(return_value), len);

	/* needed because recv/read/gzread doesn't put a null at the end*/
	Z_STRVAL_P(return_value)[Z_STRLEN_P(return_value)] = 0;

	if (Z_STRLEN_P(return_value) < len / 2) {
		Z_STR_P(return_value) = zend_string_truncate(Z_STR_P(return_value), Z_STRLEN_P(return_value), 0);
	}
}

void pthreads_streams_api_filestream_putcsv(zval *object, zval *fields, char delimiter, char enclosure, char escape_char, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_LONG(pthreads_fputcsv(threaded_stream, fields, delimiter, enclosure, escape_char));
}

void pthreads_streams_api_filestream_getcsv(zval *object, zend_long len, char delimiter, char enclosure, char escape_char, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	size_t buf_len;
	char *buf;

	if (len < 0) {
		if ((buf = pthreads_stream_get_line(threaded_stream, NULL, 0, &buf_len)) == NULL) {
			RETURN_NULL();
		}
	} else {
		buf = malloc(len + 1);
		if (pthreads_stream_get_line(threaded_stream, buf, len + 1, &buf_len) == NULL) {
			free(buf);
			RETURN_NULL();
		}
	}

	pthreads_fgetcsv(threaded_stream, delimiter, enclosure, escape_char, buf_len, buf, return_value);
}

/**
 * File API
 */

void pthreads_streams_api_file_get_meta_data(zend_string *filename, zend_bool use_include_path, zval *return_value) {
	int in_tag = 0, done = 0;
	int looking_for_val = 0, have_name = 0, have_content = 0;
	int saw_name = 0, saw_content = 0;
	char *name = NULL, *value = NULL, *temp = NULL;
	php_meta_tags_token tok, tok_last;
	pthreads_meta_tags_data md;

	/* Initiailize our structure */
	memset(&md, 0, sizeof(md));

	md.stream = pthreads_stream_open_wrapper(ZSTR_VAL(filename), "rb",
			(use_include_path ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS, NULL);
	if (!md.stream)	{
		RETURN_FALSE;
	}

	array_init(return_value);

	tok_last = TOK_EOF;

	if(stream_lock(md.stream)) {
		while (!done && (tok = pthreads_next_meta_token(&md)) != TOK_EOF) {
			if (tok == TOK_ID) {
				if (tok_last == TOK_OPENTAG) {
					md.in_meta = !strcasecmp("meta", md.token_data);
				} else if (tok_last == TOK_SLASH && in_tag) {
					if (strcasecmp("head", md.token_data) == 0) {
						/* We are done here! */
						done = 1;
					}
				} else if (tok_last == TOK_EQUAL && looking_for_val) {
					if (saw_name) {
						if (name) efree(name);
						/* Get the NAME attr (Single word attr, non-quoted) */
						temp = name = estrndup(md.token_data, md.token_len);

						while (temp && *temp) {
							if (strchr(PHP_META_UNSAFE, *temp)) {
								*temp = '_';
							}
							temp++;
						}

						have_name = 1;
					} else if (saw_content) {
						if (value) efree(value);
						value = estrndup(md.token_data, md.token_len);
						have_content = 1;
					}

					looking_for_val = 0;
				} else {
					if (md.in_meta) {
						if (strcasecmp("name", md.token_data) == 0) {
							saw_name = 1;
							saw_content = 0;
							looking_for_val = 1;
						} else if (strcasecmp("content", md.token_data) == 0) {
							saw_name = 0;
							saw_content = 1;
							looking_for_val = 1;
						}
					}
				}
			} else if (tok == TOK_STRING && tok_last == TOK_EQUAL && looking_for_val) {
				if (saw_name) {
					if (name) efree(name);
					/* Get the NAME attr (Quoted single/double) */
					temp = name = estrndup(md.token_data, md.token_len);

					while (temp && *temp) {
						if (strchr(PHP_META_UNSAFE, *temp)) {
							*temp = '_';
						}
						temp++;
					}

					have_name = 1;
				} else if (saw_content) {
					if (value) efree(value);
					value = estrndup(md.token_data, md.token_len);
					have_content = 1;
				}

				looking_for_val = 0;
			} else if (tok == TOK_OPENTAG) {
				if (looking_for_val) {
					looking_for_val = 0;
					have_name = saw_name = 0;
					have_content = saw_content = 0;
				}
				in_tag = 1;
			} else if (tok == TOK_CLOSETAG) {
				if (have_name) {
					/* For BC */
					php_strtolower(name, strlen(name));
					if (have_content) {
						add_assoc_string(return_value, name, value);
					} else {
						add_assoc_string(return_value, name, "");
					}

					efree(name);
					if (value) efree(value);
				} else if (have_content) {
					efree(value);
				}

				name = value = NULL;

				/* Reset all of our flags */
				in_tag = looking_for_val = 0;
				have_name = saw_name = 0;
				have_content = saw_content = 0;
				md.in_meta = 0;
			}

			tok_last = tok;

			if (md.token_data)
				efree(md.token_data);

			md.token_data = NULL;
		}
		stream_unlock(md.stream);
	}

	if (value) efree(value);
	if (name) efree(name);

	pthreads_stream_close(md.stream, PTHREADS_STREAM_FREE_CLOSE);
}

void pthreads_streams_api_file_get_contents(zend_string *filename, zend_bool use_include_path, zval *zcontext, zend_long offset, zend_long maxlen, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);
	pthreads_stream_t *threaded_stream;
	zend_string *contents;

	threaded_stream = pthreads_stream_open_wrapper_ex(ZSTR_VAL(filename), "rb",
				(use_include_path ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS,
				NULL, threaded_context, pthreads_file_stream_entry);
	if (!threaded_stream) {
		RETURN_NULL();
	}

	if (offset != 0 && pthreads_stream_seek(threaded_stream, offset, ((offset > 0) ? SEEK_SET : SEEK_END)) < 0) {
		php_error_docref(NULL, E_WARNING, "Failed to seek to position " ZEND_LONG_FMT " in the stream", offset);
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
		RETURN_FALSE;
	}

	if (maxlen > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "maxlen truncated from " ZEND_LONG_FMT " to %d bytes", maxlen, INT_MAX);
		maxlen = INT_MAX;
	}
	if ((contents = pthreads_stream_copy_to_mem(threaded_stream, maxlen, 0)) != NULL) {
		RETVAL_STR(contents);
	} else {
		RETVAL_EMPTY_STRING();
	}

	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
}

void pthreads_streams_api_file_put_contents(char *filename, size_t filename_len, zval *data, zend_long flags, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, flags & PTHREADS_FILE_NO_DEFAULT_CONTEXT);
	pthreads_stream_t *threaded_stream;
	char mode[3] = "wb";
	size_t numbytes = 0;

	if (flags & PTHREADS_FILE_APPEND) {
		mode[0] = 'a';
	} else if (flags & LOCK_EX) {
		/* check to make sure we are dealing with a regular file */
		if (php_memnstr(filename, "://", sizeof("://") - 1, filename + filename_len)) {
			if (strncasecmp(filename, "file://", sizeof("file://") - 1)) {
				php_error_docref(NULL, E_WARNING, "Exclusive locks may only be set for regular files");
				RETURN_NULL();
			}
		}
		mode[0] = 'c';
	}
	mode[2] = '\0';

	threaded_stream = pthreads_stream_open_wrapper_ex(filename, mode,
			((flags & PTHREADS_FILE_USE_INCLUDE_PATH) ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS, NULL, threaded_context, pthreads_file_stream_entry);
	if (threaded_stream == NULL) {
		RETURN_NULL();
	}

	if (flags & LOCK_EX && (!pthreads_stream_supports_lock(threaded_stream) || pthreads_stream_lock(threaded_stream, LOCK_EX))) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
		php_error_docref(NULL, E_WARNING, "Exclusive locks are not supported for this stream");
		RETURN_NULL();
	}

	if (mode[0] == 'c') {
		pthreads_stream_truncate_set_size(threaded_stream, 0);
	}

	switch (Z_TYPE_P(data)) {
		case IS_NULL:
		case IS_LONG:
		case IS_DOUBLE:
		case IS_FALSE:
		case IS_TRUE:
			convert_to_string_ex(data);

		case IS_STRING:
			if (Z_STRLEN_P(data)) {
				numbytes = pthreads_stream_write(threaded_stream, Z_STRVAL_P(data), Z_STRLEN_P(data));
				if (numbytes != Z_STRLEN_P(data)) {
					php_error_docref(NULL, E_WARNING, "Only "ZEND_LONG_FMT" of %zd bytes written, possibly out of free disk space", numbytes, Z_STRLEN_P(data));
					numbytes = -1;
				}
			}
			break;

		case IS_ARRAY:
			if (zend_hash_num_elements(Z_ARRVAL_P(data))) {
				size_t bytes_written;
				zval *tmp;

				ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(data), tmp) {
					zend_string *t;
					zend_string *str = zval_get_tmp_string(tmp, &t);
					if (ZSTR_LEN(str)) {
						numbytes += ZSTR_LEN(str);
						bytes_written = pthreads_stream_write(threaded_stream, ZSTR_VAL(str), ZSTR_LEN(str));
						if (bytes_written != ZSTR_LEN(str)) {
							php_error_docref(NULL, E_WARNING, "Failed to write %zd bytes to %s", ZSTR_LEN(str), filename);
							zend_tmp_string_release(t);
							numbytes = -1;
							break;
						}
					}
					zend_tmp_string_release(t);
				} ZEND_HASH_FOREACH_END();
			}
			break;

		case IS_OBJECT:
			if(instanceof_function(Z_OBJCE_P(data), pthreads_stream_entry)) {
				size_t len;
				pthreads_stream_t *threaded_srcstream = PTHREADS_FETCH_FROM(Z_OBJ_P(data));

				if (pthreads_stream_copy_to_stream_ex(threaded_srcstream, threaded_stream, PTHREADS_STREAM_COPY_ALL, &len) != SUCCESS) {
					numbytes = -1;
				} else {
					if (len > ZEND_LONG_MAX) {
						php_error_docref(NULL, E_WARNING, "content truncated from %zu to " ZEND_LONG_FMT " bytes", len, ZEND_LONG_MAX);
						len = ZEND_LONG_MAX;
					}
					numbytes = len;
				}
				break;
			}

			if (Z_OBJ_HT_P(data) != NULL) {
				zval out;

				if (zend_std_cast_object_tostring(data, &out, IS_STRING) == SUCCESS) {
					numbytes = pthreads_stream_write(threaded_stream, Z_STRVAL(out), Z_STRLEN(out));
					if (numbytes != Z_STRLEN(out)) {
						php_error_docref(NULL, E_WARNING, "Only "ZEND_LONG_FMT" of %zd bytes written, possibly out of free disk space", numbytes, Z_STRLEN(out));
						numbytes = -1;
					}
					zval_ptr_dtor_str(&out);
					break;
				}
			}
		default:
			numbytes = -1;
			break;
	}
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);

	if (numbytes == (size_t)-1) {
		RETURN_NULL();
	}

	RETURN_LONG(numbytes);
}

void pthreads_streams_api_file_file(char *filename, zend_long flags, zval *context, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(context, flags & PTHREADS_FILE_NO_DEFAULT_CONTEXT);
	pthreads_stream_t *threaded_stream;
	pthreads_stream *stream;
	char *p, *s, *e;
	register int i = 0;
	char eol_marker = '\n';
	zend_bool use_include_path;
	zend_bool include_new_line;
	zend_bool skip_blank_lines;
	zend_string *target_buf;

	if (flags < 0 || flags > (PTHREADS_FILE_USE_INCLUDE_PATH | PTHREADS_FILE_IGNORE_NEW_LINES | PTHREADS_FILE_SKIP_EMPTY_LINES | PTHREADS_FILE_NO_DEFAULT_CONTEXT)) {
		php_error_docref(NULL, E_WARNING, "'" ZEND_LONG_FMT "' flag is not supported", flags);
		RETURN_NULL();
	}

	use_include_path = flags & PTHREADS_FILE_USE_INCLUDE_PATH;
	include_new_line = !(flags & PTHREADS_FILE_IGNORE_NEW_LINES);
	skip_blank_lines = flags & PTHREADS_FILE_SKIP_EMPTY_LINES;

	threaded_stream = pthreads_stream_open_wrapper_ex(filename, "rb",
			(use_include_path ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS, NULL, threaded_context, pthreads_file_stream_entry);
	if (!threaded_stream) {
		RETURN_NULL();
	}
	stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	/* Initialize return array */
	array_init(return_value);

	if ((target_buf = pthreads_stream_copy_to_mem(threaded_stream, PTHREADS_STREAM_COPY_ALL, 0)) != NULL) {
		s = ZSTR_VAL(target_buf);
		e = ZSTR_VAL(target_buf) + ZSTR_LEN(target_buf);

		if (!(p = (char*)pthreads_stream_locate_eol(threaded_stream, target_buf))) {
			p = e;
			goto parse_eol;
		}

		if (stream->flags & PTHREADS_STREAM_FLAG_EOL_MAC) {
			eol_marker = '\r';
		}

		/* for performance reasons the code is duplicated, so that the if (include_new_line)
		 * will not need to be done for every single line in the file. */
		if (include_new_line) {
			do {
				p++;
parse_eol:
				add_index_stringl(return_value, i++, s, p-s);
				s = p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		} else {
			do {
				int windows_eol = 0;
				if (p != ZSTR_VAL(target_buf) && eol_marker == '\n' && *(p - 1) == '\r') {
					windows_eol++;
				}
				if (skip_blank_lines && !(p-s-windows_eol)) {
					s = ++p;
					continue;
				}
				add_index_stringl(return_value, i++, s, p-s-windows_eol);
				s = ++p;
			} while ((p = memchr(p, eol_marker, (e-p))));
		}

		/* handle any left overs of files without new lines */
		if (s != e) {
			p = e;
			goto parse_eol;
		}
	}

	if (target_buf) {
		zend_string_free(target_buf);
	}
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
}

void pthreads_streams_api_file_temp_name(char *dir, char *prefix, size_t prefix_len, zval *return_value) {
	zend_string *opened_path;
	int fd;
	zend_string *p;

	if (php_check_open_basedir(dir)) {
		RETURN_NULL();
	}

	p = php_basename(prefix, prefix_len, NULL, 0);
	if (ZSTR_LEN(p) > 64) {
		ZSTR_VAL(p)[63] = '\0';
	}

	RETVAL_NULL();

	if ((fd = pthreads_open_temporary_fd_ex(dir, ZSTR_VAL(p), &opened_path, 1)) >= 0) {
		close(fd);
		RETVAL_STR(opened_path);
	}
	zend_string_release(p);
}

void pthreads_streams_api_file_temp_file(zval *return_value) {
	pthreads_stream_t *threaded_stream = pthreads_stream_fopen_tmpfile();

	if (threaded_stream) {
		pthreads_stream_to_zval(threaded_stream, return_value);
	} else {
		RETURN_NULL();
	}
}

void pthreads_streams_api_file_open(zend_string *filename, zend_string *mode, zend_bool use_include_path, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);
	pthreads_stream_t *threaded_stream;

	threaded_stream = pthreads_stream_open_wrapper_ex(ZSTR_VAL(filename), ZSTR_VAL(mode),
			(use_include_path ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS, NULL, threaded_context, pthreads_file_stream_entry);

	if (threaded_stream == NULL) {
		RETURN_NULL();
	}

	pthreads_stream_to_zval(threaded_stream, return_value);
}

void pthreads_streams_api_file_popen(char *command, char *mode, int mode_len, zval *return_value) {
	pthreads_stream_t *threaded_stream;
	FILE *fp;
	char *posix_mode;

	posix_mode = estrndup(mode, mode_len);
#ifndef PHP_WIN32
	{
		char *z = memchr(posix_mode, 'b', mode_len);
		if (z) {
			memmove(z, z + 1, mode_len - (z - posix_mode));
		}
	}
#endif

	fp = VCWD_POPEN(command, posix_mode);
	if (!fp) {
		php_error_docref2(NULL, command, posix_mode, E_WARNING, "%s", strerror(errno));
		efree(posix_mode);
		RETURN_NULL();
	}

	threaded_stream = _pthreads_stream_fopen_from_pipe(fp, mode, pthreads_file_stream_entry);

	if (threaded_stream == NULL)	{
		php_error_docref2(NULL, command, mode, E_WARNING, "%s", strerror(errno));
		RETVAL_NULL();
	} else {
		pthreads_stream_to_zval(threaded_stream, return_value);
	}

	efree(posix_mode);
}

void pthreads_streams_api_file_mkdir(zend_string *dir, zend_long mode, zend_bool recursive, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(pthreads_stream_mkdir(ZSTR_VAL(dir), (int)mode, (recursive ? PTHREADS_STREAM_MKDIR_RECURSIVE : 0) | PTHREADS_REPORT_ERRORS, threaded_context));
}

void pthreads_streams_api_file_rmdir(zend_string *dir, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);

	RETURN_BOOL(pthreads_stream_rmdir(ZSTR_VAL(dir), PTHREADS_REPORT_ERRORS, threaded_context));
}

void pthreads_streams_api_file_readfile(zend_string *filename, zend_bool use_include_path, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);
	pthreads_stream_t *threaded_stream;
	size_t size = 0;

	threaded_stream = pthreads_stream_open_wrapper_ex(ZSTR_VAL(filename), "rb", (use_include_path ? PTHREADS_USE_PATH : 0) | PTHREADS_REPORT_ERRORS, NULL, threaded_context, NULL);
	if (threaded_stream) {
		size = pthreads_stream_passthru(threaded_stream);
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
		RETURN_LONG(size);
	}

	RETURN_LONG(-1);
}

void pthreads_streams_api_file_rename(zend_string *old_name, zend_string *new_name, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);
	pthreads_stream_wrapper_t *threaded_wrapper;
	pthreads_stream_wrapper *wrapper;

	// todo: threadsafety

	threaded_wrapper = pthreads_stream_locate_url_wrapper(ZSTR_VAL(old_name), NULL, 0);

	if (!threaded_wrapper || !PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper)->wops) {
		php_error_docref(NULL, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}
	wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

	if (!wrapper->wops->rename) {
		php_error_docref(NULL, E_WARNING, "%s wrapper does not support renaming", wrapper->wops->label ? wrapper->wops->label : "Source");
		RETURN_FALSE;
	}

	if (threaded_wrapper != pthreads_stream_locate_url_wrapper(ZSTR_VAL(new_name), NULL, 0)) {
		php_error_docref(NULL, E_WARNING, "Cannot rename a file across wrapper types");
		RETURN_FALSE;
	}
	RETURN_BOOL(wrapper->wops->rename(threaded_wrapper, ZSTR_VAL(old_name), ZSTR_VAL(new_name), 0, threaded_context));
}

void pthreads_streams_api_file_unlink(zend_string *filename, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);
	pthreads_stream_wrapper_t *threaded_wrapper;
	pthreads_stream_wrapper *wrapper;

	threaded_wrapper = pthreads_stream_locate_url_wrapper(ZSTR_VAL(filename), NULL, 0);

	if (!threaded_wrapper || !PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper)->wops) {
		php_error_docref(NULL, E_WARNING, "Unable to locate stream wrapper");
		RETURN_FALSE;
	}
	wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

	if (!wrapper->wops->unlink) {
		php_error_docref(NULL, E_WARNING, "%s does not allow unlinking", wrapper->wops->label ? wrapper->wops->label : "Wrapper");
		RETURN_FALSE;
	}
	RETURN_BOOL(wrapper->wops->unlink(threaded_wrapper, ZSTR_VAL(filename), PTHREADS_REPORT_ERRORS, threaded_context));
}

void pthreads_streams_api_file_copy(char *source, char *target, zval *zcontext, zval *return_value) {
	pthreads_stream_context_t *threaded_context = pthreads_stream_context_from_zval(zcontext, 0);

	if (php_check_open_basedir(source)) {
		RETURN_FALSE;
	}

	if (pthreads_copy_file_ctx(source, target, 0, threaded_context) == SUCCESS) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

void pthreads_streams_api_file_sockopen(zend_string *host, zend_long port, zval *zerrno, zval *zerrstr, double timeout, int persistent, zval *return_value) {
	struct timeval tv;
	pthreads_stream_t *threaded_stream = NULL;
#ifndef PHP_WIN32
	time_t conv;
#else
	long conv;
#endif
	int err;
	char *hostname = NULL;
	size_t hostname_len;
	zend_string *errstr = NULL;

	RETVAL_NULL();


	if (port > 0) {
		hostname_len = spprintf(&hostname, 0, "%s:" ZEND_LONG_FMT, ZSTR_VAL(host), port);
	} else {
		hostname_len = ZSTR_LEN(host);
		hostname = ZSTR_VAL(host);
	}

	/* prepare the timeout value for use */
#ifndef PHP_WIN32
	conv = (time_t) (timeout * 1000000.0);
	tv.tv_sec = conv / 1000000;
#else
	conv = (long) (timeout * 1000000.0);
	tv.tv_sec = conv / 1000000;
#endif
	tv.tv_usec = conv % 1000000;

	if (zerrno)	{
		zval_ptr_dtor(zerrno);
		ZVAL_LONG(zerrno, 0);
	}
	if (zerrstr) {
		zval_ptr_dtor(zerrstr);
		ZVAL_EMPTY_STRING(zerrstr);
	}

	threaded_stream = pthreads_stream_xport_create(hostname, hostname_len, PTHREADS_REPORT_ERRORS,
			PTHREADS_STREAM_XPORT_CLIENT | PTHREADS_STREAM_XPORT_CONNECT, &tv, NULL, &errstr, &err);

	if (port > 0) {
		efree(hostname);
	}
	if (threaded_stream == NULL) {
		php_error_docref(NULL, E_WARNING, "unable to connect to %s:" ZEND_LONG_FMT " (%s)", ZSTR_VAL(host), port, errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
	}


	if (threaded_stream == NULL) {
		if (zerrno) {
			zval_ptr_dtor(zerrno);
			ZVAL_LONG(zerrno, err);
		}

		if (zerrstr && errstr) {
			/* no need to dup; we need to efree buf anyway */
			zval_ptr_dtor(zerrstr);
			ZVAL_STR(zerrstr, errstr);
		} else if (!zerrstr && errstr) {
			zend_string_release(errstr);
		}

		RETURN_NULL();
	}

	if (errstr) {
		zend_string_release(errstr);
	}

	pthreads_stream_to_zval(threaded_stream, return_value);
}

#endif
