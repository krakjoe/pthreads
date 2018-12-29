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
#ifndef HAVE_PTHREADS_INFO
#define HAVE_PTHREADS_INFO

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HTML_H
#	include <ext/standard/html.h>
#endif

static int pthreads_info_printf(const char *fmt, ...) /* {{{ */
{
	char *buf;
	size_t len, written;
	va_list argv;

	va_start(argv, fmt);
	len = vspprintf(&buf, 0, fmt, argv);
	va_end(argv);

	written = php_output_write(buf, len);
	efree(buf);
	return written;
}
/* }}} */

static int pthreads_info_print(const char *str) /* {{{ */
{
	return php_output_write(str, strlen(str));
}

static int pthreads_info_print_html_esc(const char *str, size_t len) /* {{{ */
{
	size_t written;
	zend_string *new_str;

	new_str = php_escape_html_entities((unsigned char *) str, len, 0, ENT_QUOTES, "utf-8");
	written = php_output_write(ZSTR_VAL(new_str), ZSTR_LEN(new_str));
	zend_string_free(new_str);
	return written;
}
/* }}} */

static void pthreads_info_print_stream_hash(const char *name, HashTable *ht) /* {{{ */
{
	zend_string *key;

	if (ht) {
		if (zend_hash_num_elements(ht)) {
			int first = 1;

			if (!sapi_module.phpinfo_as_text) {
				pthreads_info_printf("<tr><td class=\"e\">Registered %s</td><td class=\"v\">", name);
			} else {
				pthreads_info_printf("\nRegistered %s => ", name);
			}

			ZEND_HASH_FOREACH_STR_KEY(ht, key) {
				if (key) {
					if (first) {
						first = 0;
					} else {
						pthreads_info_print(", ");
					}
					if (!sapi_module.phpinfo_as_text) {
						pthreads_info_print_html_esc(ZSTR_VAL(key), ZSTR_LEN(key));
					} else {
						pthreads_info_print(ZSTR_VAL(key));
					}
				}
			} ZEND_HASH_FOREACH_END();

			if (!sapi_module.phpinfo_as_text) {
				pthreads_info_print("</td></tr>\n");
			}
		} else {
			char reg_name[128];
			snprintf(reg_name, sizeof(reg_name), "\nRegistered %s", name);
			php_info_print_table_row(2, reg_name, "none registered");
		}
	} else {
		php_info_print_table_row(2, name, "disabled");
	}
}
/* }}} */

static void pthreads_info_print_table_end(void) /* {{{ */
{
	if (!sapi_module.phpinfo_as_text) {
		pthreads_info_print("</table>\n");
	} else {
		pthreads_info_print("\n");
	}
}
/* }}} */

#endif
