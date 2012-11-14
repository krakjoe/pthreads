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
#ifndef HAVE_PTHREADS_GLOBALS
#define HAVE_PTHREADS_GLOBALS

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

struct _pthreads_globals pthreads_globals;

/* {{{ pthreads_globals_init */
void pthreads_globals_init(TSRMLS_D){
	if (!PTHREADS_G(init)) {
		PTHREADS_G(init)=1;
		PTHREADS_G(lock)=pthreads_lock_alloc(TSRMLS_C);
	}
} /* }}} */

/* {{{ pthreads_globals_lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC){
	return pthreads_lock_acquire(PTHREADS_G(lock), locked TSRMLS_CC);
} /* }}} */

/* {{{ pthreads_globals_unlock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC) {
	pthreads_lock_release(PTHREADS_G(lock), locked TSRMLS_CC);
} /* }}} */
#endif
