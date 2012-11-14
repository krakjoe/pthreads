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
#ifndef HAVE_PTHREADS_LOCK_H
#define HAVE_PTHREADS_LOCK_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct {
	pthread_mutex_t mutex;
	void*** 		owner;
	
} *pthreads_lock;

pthreads_lock pthreads_lock_alloc(TSRMLS_D);
zend_bool pthreads_lock_acquire(pthreads_lock lock, zend_bool *acquired TSRMLS_DC);
zend_bool pthreads_lock_release(pthreads_lock lock, zend_bool acquired TSRMLS_DC);
void pthreads_lock_free(pthreads_lock lock TSRMLS_DC);
#endif
