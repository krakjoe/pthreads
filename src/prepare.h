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
#ifndef HAVE_PTHREADS_PREPARE_H
#define HAVE_PTHREADS_PREPARE_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(PTHREAD thread, zend_class_entry *candidate TSRMLS_DC); /* }}} */

/* {{{ prepare for execution in context */
void pthreads_prepare(PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ initialize prepared class storage */
void pthreads_prepare_classes_init(PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ free prepared classes */
void pthreads_prepare_classes_free(PTHREAD thread TSRMLS_DC); /* }}} */

#endif
