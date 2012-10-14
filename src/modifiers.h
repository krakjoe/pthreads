/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                                		 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                         |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_MODIFIERS_H
#define HAVE_PTHREADS_MODIFIERS_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#define PTHREADS_PROTECTION_ERROR 0x1200

/* {{{ access modification management */
void pthreads_modifiers_init(PTHREAD thread, zval *this_ptr TSRMLS_DC);
int pthreads_modifiers_set(PTHREAD thread, const char *method, zend_uint modify TSRMLS_DC);
zend_uint pthreads_modifiers_get(PTHREAD thread, const char *method TSRMLS_DC);
int pthreads_modifiers_protect(PTHREAD thread);
int pthreads_modifiers_unprotect(PTHREAD thread);
void pthreads_modifiers_destroy(void **element);
/* }}} */

#endif
