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
/*
* This API is NOT ready to be exposed to userland.
*
* This header provides pthreads with a recursive locking mechanism that
* will eventually be independant of threading implementation that pthreads
* is using, it will use the default mutex type on the system, but will manage
* the lock count using TSRMLS_C as a reference to the owner
*/
#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct {
	pthread_mutex_t mutex;
	void*** 		owner;
	ulong			locks;
} *pthreads_lock;

/* {{{ allocate a lock */
pthreads_lock pthreads_lock_alloc(TSRMLS_D); /* }}} */

/* {{{ acquire a lock */
zend_bool pthreads_lock_acquire(pthreads_lock lock, zend_bool *acquired TSRMLS_DC); /* }}} */

/* {{{ release a lock */
zend_bool pthreads_lock_release(pthreads_lock lock, zend_bool acquired TSRMLS_DC); /* }}} */

/* {{{ free a lock */
void pthreads_lock_free(pthreads_lock lock TSRMLS_DC); /* }}} */
#endif
