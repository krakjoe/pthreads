/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_PREPARE_H
#define HAVE_PTHREADS_PREPARE_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(pthreads_object_t* thread, zend_class_entry *candidate); /* }}} */

/* {{{ */
void pthreads_prepare_parent(pthreads_object_t *thread); /* }}} */

/* {{{ */
int pthreads_prepared_startup(pthreads_object_t* thread, pthreads_monitor_t *ready); /* }}} */

/* {{{ */
int pthreads_prepared_shutdown(void); /* }}} */
#endif
