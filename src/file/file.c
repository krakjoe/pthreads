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
#ifndef HAVE_PTHREADS_FILE
#define HAVE_PTHREADS_FILE

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_FILE_H
#	include <src/file/file.h>
#endif

#include "ext/standard/flock_compat.h"
#include "zend_smart_str.h"

/* {{{ php_next_meta_token
   Tokenizes an HTML file for get_meta_tags */
php_meta_tags_token pthreads_next_meta_token(pthreads_meta_tags_data *md) {
	int ch = 0, compliment;
	char buff[PTHREADS_META_DEF_BUFSIZE + 1];

	memset((void *)buff, 0, PTHREADS_META_DEF_BUFSIZE + 1);

	while (md->ulc || (!pthreads_stream_eof(md->stream) && (ch = pthreads_stream_getc(md->stream)))) {
		if (pthreads_stream_eof(md->stream)) {
			break;
		}

		if (md->ulc) {
			ch = md->lc;
			md->ulc = 0;
		}

		switch (ch) {
			case '<':
				return TOK_OPENTAG;
				break;

			case '>':
				return TOK_CLOSETAG;
				break;

			case '=':
				return TOK_EQUAL;
				break;
			case '/':
				return TOK_SLASH;
				break;

			case '\'':
			case '"':
				compliment = ch;
				md->token_len = 0;
				while (!pthreads_stream_eof(md->stream) && (ch = pthreads_stream_getc(md->stream)) && ch != compliment && ch != '<' && ch != '>') {
					buff[(md->token_len)++] = ch;

					if (md->token_len == PTHREADS_META_DEF_BUFSIZE) {
						break;
					}
				}

				if (ch == '<' || ch == '>') {
					/* Was just an apostrohpe */
					md->ulc = 1;
					md->lc = ch;
				}

				/* We don't need to alloc unless we are in a meta tag */
				if (md->in_meta) {
					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);
				}

				return TOK_STRING;
				break;

			case '\n':
			case '\r':
			case '\t':
				break;

			case ' ':
				return TOK_SPACE;
				break;

			default:
				if (isalnum(ch)) {
					md->token_len = 0;
					buff[(md->token_len)++] = ch;
					while (!pthreads_stream_eof(md->stream) && (ch = pthreads_stream_getc(md->stream)) && (isalnum(ch) || strchr(PTHREADS_META_HTML401_CHARS, ch))) {
						buff[(md->token_len)++] = ch;

						if (md->token_len == PTHREADS_META_DEF_BUFSIZE) {
							break;
						}
					}

					/* This is ugly, but we have to replace ungetc */
					if (!isalpha(ch) && ch != '-') {
						md->ulc = 1;
						md->lc = ch;
					}

					md->token_data = (char *) emalloc(md->token_len + 1);
					memcpy(md->token_data, buff, md->token_len+1);

					return TOK_ID;
				} else {
					return TOK_OTHER;
				}
				break;
		}
	}

	return TOK_EOF;
}
/* }}} */

/* {{{ php_copy_file_ctx */
int pthreads_copy_file_ctx(const char *src, const char *dest, int src_flg, pthreads_stream_context_t *threaded_ctx) {
	pthreads_stream_t *srcstream = NULL, *deststream = NULL;
	int ret = FAILURE;
	pthreads_stream_statbuf src_s, dest_s;

	switch (pthreads_stream_stat_path_ex(src, 0, &src_s, threaded_ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(src_s.sb.st_mode)) {
		php_error_docref(NULL, E_WARNING, "The first argument to copy() function cannot be a directory");
		return FAILURE;
	}

	switch (pthreads_stream_stat_path_ex(dest, PTHREADS_STREAM_URL_STAT_QUIET | PTHREADS_STREAM_URL_STAT_NOCACHE, &dest_s, threaded_ctx)) {
		case -1:
			/* non-statable stream */
			goto safe_to_copy;
			break;
		case 0:
			break;
		default: /* failed to stat file, does not exist? */
			return ret;
	}
	if (S_ISDIR(dest_s.sb.st_mode)) {
		php_error_docref(NULL, E_WARNING, "The second argument to copy() function cannot be a directory");
		return FAILURE;
	}
	if (!src_s.sb.st_ino || !dest_s.sb.st_ino) {
		goto no_stat;
	}
	if (src_s.sb.st_ino == dest_s.sb.st_ino && src_s.sb.st_dev == dest_s.sb.st_dev) {
		return ret;
	} else {
		goto safe_to_copy;
	}
no_stat:
	{
		char *sp, *dp;
		int res;

		if ((sp = expand_filepath(src, NULL)) == NULL) {
			return ret;
		}
		if ((dp = expand_filepath(dest, NULL)) == NULL) {
			efree(sp);
			goto safe_to_copy;
		}

		res =
#ifndef PHP_WIN32
			!strcmp(sp, dp);
#else
			!strcasecmp(sp, dp);
#endif

		efree(sp);
		efree(dp);
		if (res) {
			return ret;
		}
	}
safe_to_copy:

	srcstream = pthreads_stream_open_wrapper_ex(src, "rb", src_flg | PTHREADS_REPORT_ERRORS, NULL, threaded_ctx, NULL);

	if (!srcstream) {
		return ret;
	}
	deststream = pthreads_stream_open_wrapper_ex(dest, "wb", PTHREADS_REPORT_ERRORS, NULL, threaded_ctx, NULL);

	if (srcstream && deststream) {
		ret = pthreads_stream_copy_to_stream_ex(srcstream, deststream, PTHREADS_STREAM_COPY_ALL, NULL);
	}

	if (srcstream) {
		pthreads_stream_close(srcstream, PTHREADS_STREAM_FREE_CLOSE);
	}

	if (deststream) {
		pthreads_stream_close(deststream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return ret;
}
/* }}} */


#define FPUTCSV_FLD_CHK(c) memchr(ZSTR_VAL(field_str), c, ZSTR_LEN(field_str))

/* {{{ size_t pthreads_fputcsv(pthreads_stream_t *threaded_stream, zval *fields, char delimiter, char enclosure, char escape_char) */
size_t pthreads_fputcsv(pthreads_stream_t *threaded_stream, zval *fields, char delimiter, char enclosure, char escape_char)
{
	int count, i = 0;
	size_t ret;
	zval *field_tmp;
	smart_str csvline = {0};

	count = zend_hash_num_elements(Z_ARRVAL_P(fields));
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(fields), field_tmp) {
		zend_string *tmp_field_str;
		zend_string *field_str = zval_get_tmp_string(field_tmp, &tmp_field_str);

		/* enclose a field that contains a delimiter, an enclosure character, or a newline */
		if (FPUTCSV_FLD_CHK(delimiter) ||
			FPUTCSV_FLD_CHK(enclosure) ||
			FPUTCSV_FLD_CHK(escape_char) ||
			FPUTCSV_FLD_CHK('\n') ||
			FPUTCSV_FLD_CHK('\r') ||
			FPUTCSV_FLD_CHK('\t') ||
			FPUTCSV_FLD_CHK(' ')
		) {
			char *ch = ZSTR_VAL(field_str);
			char *end = ch + ZSTR_LEN(field_str);
			int escaped = 0;

			smart_str_appendc(&csvline, enclosure);
			while (ch < end) {
				if (*ch == escape_char) {
					escaped = 1;
				} else if (!escaped && *ch == enclosure) {
					smart_str_appendc(&csvline, enclosure);
				} else {
					escaped = 0;
				}
				smart_str_appendc(&csvline, *ch);
				ch++;
			}
			smart_str_appendc(&csvline, enclosure);
		} else {
			smart_str_append(&csvline, field_str);
		}

		if (++i != count) {
			smart_str_appendl(&csvline, &delimiter, 1);
		}
		zend_tmp_string_release(tmp_field_str);
	} ZEND_HASH_FOREACH_END();

	smart_str_appendc(&csvline, '\n');
	smart_str_0(&csvline);

	ret = pthreads_stream_write(threaded_stream, ZSTR_VAL(csvline.s), ZSTR_LEN(csvline.s));

	smart_str_free(&csvline);

	return ret;
}
/* }}} */

static const char *pthreads_fgetcsv_lookup_trailing_spaces(const char *ptr, size_t len, const char delimiter) /* {{{ */
{
	int inc_len;
	unsigned char last_chars[2] = { 0, 0 };

	while (len > 0) {
		inc_len = (*ptr == '\0' ? 1 : php_mblen(ptr, len));
		switch (inc_len) {
			case -2:
			case -1:
				inc_len = 1;
				php_mb_reset();
				break;
			case 0:
				goto quit_loop;
			case 1:
			default:
				last_chars[0] = last_chars[1];
				last_chars[1] = *ptr;
				break;
		}
		ptr += inc_len;
		len -= inc_len;
	}
quit_loop:
	switch (last_chars[1]) {
		case '\n':
			if (last_chars[0] == '\r') {
				return ptr - 2;
			}
			/* break is omitted intentionally */
		case '\r':
			return ptr - 1;
	}
	return ptr;
}
/* }}} */

/* {{{ */
void pthreads_fgetcsv(pthreads_stream_t *threaded_stream, char delimiter, char enclosure, char escape_char, size_t buf_len, char *buf, zval *return_value) {
	char *temp, *tptr, *bptr, *line_end, *limit;
	size_t temp_len, line_end_len;
	int inc_len;
	zend_bool first_field = 1;

	/* initialize internal state */
	php_mb_reset();

	/* Now into new section that parses buf for delimiter/enclosure fields */

	/* Strip trailing space from buf, saving end of line in case required for enclosure field */

	bptr = buf;
	tptr = (char *)pthreads_fgetcsv_lookup_trailing_spaces(buf, buf_len, delimiter);
	line_end_len = buf_len - (size_t)(tptr - buf);
	line_end = limit = tptr;

	/* reserve workspace for building each individual field */
	temp_len = buf_len;
	temp = malloc(temp_len + line_end_len + 1);

	/* Initialize return array */
	array_init(return_value);

	/* Main loop to read CSV fields */
	/* NB this routine will return a single null entry for a blank line */

	do {
		char *comp_end, *hunk_begin;

		tptr = temp;

		inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : php_mblen(bptr, limit - bptr)): 0);
		if (inc_len == 1) {
			char *tmp = bptr;
			while ((*tmp != delimiter) && isspace((int)*(unsigned char *)tmp)) {
				tmp++;
  			}
			if (*tmp == enclosure) {
				bptr = tmp;
			}
  		}

		if (first_field && bptr == line_end) {
			add_next_index_null(return_value);
			break;
		}
		first_field = 0;
		/* 2. Read field, leaving bptr pointing at start of next field */
		if (inc_len != 0 && *bptr == enclosure) {
			int state = 0;

			bptr++;	/* move on to first character in field */
			hunk_begin = bptr;

			/* 2A. handle enclosure delimited field */
			for (;;) {
				switch (inc_len) {
					case 0:
						switch (state) {
							case 2:
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;

							case 1:
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								/* break is omitted intentionally */

							case 0: {
								char *new_buf;
								size_t new_len;
								char *new_temp;

								if (hunk_begin != line_end) {
									memcpy(tptr, hunk_begin, bptr - hunk_begin);
									tptr += (bptr - hunk_begin);
									hunk_begin = bptr;
								}

								/* add the embedded line end to the field */
								memcpy(tptr, line_end, line_end_len);
								tptr += line_end_len;

								if (threaded_stream == NULL) {
									goto quit_loop_2;
								} else if ((new_buf = pthreads_stream_get_line(threaded_stream, NULL, 0, &new_len)) == NULL) {
									/* we've got an unterminated enclosure,
									 * assign all the data from the start of
									 * the enclosure to end of data to the
									 * last element */
									if ((size_t)temp_len > (size_t)(limit - buf)) {
										goto quit_loop_2;
									}
									zend_array_destroy(Z_ARR_P(return_value));
									RETVAL_NULL();
									goto out;
								}
								temp_len += new_len;
								new_temp = realloc(temp, temp_len);
								tptr = new_temp + (size_t)(tptr - temp);
								temp = new_temp;

								free(buf);
								buf_len = new_len;
								bptr = buf = new_buf;
								hunk_begin = buf;

								line_end = limit = (char *)pthreads_fgetcsv_lookup_trailing_spaces(buf, buf_len, delimiter);
								line_end_len = buf_len - (size_t)(limit - buf);

								state = 0;
							} break;
						}
						break;

					case -2:
					case -1:
						php_mb_reset();
						/* break is omitted intentionally */
					case 1:
						/* we need to determine if the enclosure is
						 * 'real' or is it escaped */
						switch (state) {
							case 1: /* escaped */
								bptr++;
								state = 0;
								break;
							case 2: /* embedded enclosure ? let's check it */
								if (*bptr != enclosure) {
									/* real enclosure */
									memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
									tptr += (bptr - hunk_begin - 1);
									hunk_begin = bptr;
									goto quit_loop_2;
								}
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								bptr++;
								hunk_begin = bptr;
								state = 0;
								break;
							default:
								if (*bptr == enclosure) {
									state = 2;
								} else if (*bptr == escape_char) {
									state = 1;
								}
								bptr++;
								break;
						}
						break;

					default:
						switch (state) {
							case 2:
								/* real enclosure */
								memcpy(tptr, hunk_begin, bptr - hunk_begin - 1);
								tptr += (bptr - hunk_begin - 1);
								hunk_begin = bptr;
								goto quit_loop_2;
							case 1:
								bptr += inc_len;
								memcpy(tptr, hunk_begin, bptr - hunk_begin);
								tptr += (bptr - hunk_begin);
								hunk_begin = bptr;
								state = 0;
								break;
							default:
								bptr += inc_len;
								break;
						}
						break;
				}
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : php_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_2:
			/* look up for a delimiter */
			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_3;

					case -2:
					case -1:
						inc_len = 1;
						php_mb_reset();
						/* break is omitted intentionally */
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_3;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : php_mblen(bptr, limit - bptr)): 0);
			}

		quit_loop_3:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);
			bptr += inc_len;
			comp_end = tptr;
		} else {
			/* 2B. Handle non-enclosure field */

			hunk_begin = bptr;

			for (;;) {
				switch (inc_len) {
					case 0:
						goto quit_loop_4;
					case -2:
					case -1:
						inc_len = 1;
						php_mb_reset();
						/* break is omitted intentionally */
					case 1:
						if (*bptr == delimiter) {
							goto quit_loop_4;
						}
						break;
					default:
						break;
				}
				bptr += inc_len;
				inc_len = (bptr < limit ? (*bptr == '\0' ? 1 : php_mblen(bptr, limit - bptr)): 0);
			}
		quit_loop_4:
			memcpy(tptr, hunk_begin, bptr - hunk_begin);
			tptr += (bptr - hunk_begin);

			comp_end = (char *)pthreads_fgetcsv_lookup_trailing_spaces(temp, tptr - temp, delimiter);
			if (*bptr == delimiter) {
				bptr++;
			}
		}

		/* 3. Now pass our field back to php */
		*comp_end = '\0';
		add_next_index_stringl(return_value, temp, comp_end - temp);
	} while (inc_len > 0);

out:
	free(temp);
	if (threaded_stream) {
		free(buf);
	}
}
/* }}} */

#ifndef HAVE_PTHREADS_FILESTREAM_API
#	include "src/file/filestream_api.c"
#endif

#endif
