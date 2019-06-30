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
#ifndef HAVE_PTHREADS_URL_H
#define HAVE_PTHREADS_URL_H

typedef struct _pthreads_url {
	zend_string *scheme;
	zend_string *user;
	zend_string *pass;
	zend_string *host;
	unsigned short port;
	zend_string *path;
	zend_string *query;
	zend_string *fragment;
} pthreads_url;

void pthreads_url_free(pthreads_url *theurl);
pthreads_url *pthreads_url_parse(char const *str);
pthreads_url *pthreads_url_parse_ex(char const *str, size_t length);
char *pthreads_replace_controlchars_ex(char *str, size_t len);

#endif
