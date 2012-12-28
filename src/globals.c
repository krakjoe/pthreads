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

struct _pthreads_globals pthreads_globals;

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef PTHREADS_GTSRMLS_C
#	define PTHREADS_GTSRMLS_C (TSRMLS_C) ? TSRMLS_C : (void***) &pthreads_globals
#endif

#ifndef PTHREADS_GLOBAL_LOCK_ARGS
#	define PTHREADS_GLOBAL_LOCK_ARGS \
		locked, PTHREADS_GTSRMLS_C
#endif


/* {{{ pthreads_globals_init */
zend_bool pthreads_globals_init(TSRMLS_D){
	if (!PTHREADS_G(init)&&!PTHREADS_G(failed)) {
		PTHREADS_G(init)=1;
		if (!(PTHREADS_G(lock)=pthreads_lock_alloc(PTHREADS_GTSRMLS_C)))
			PTHREADS_G(failed)=1;
		if (PTHREADS_G(failed))
			PTHREADS_G(init)=0;
		if (PTHREADS_G(init)) {
			PTHREADS_G(default_resource_dtor)=(EG(regular_list).pDestructor);
		}
		return PTHREADS_G(init);
	} else return 0;
} /* }}} */

/* {{{ pthreads_globals_lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC){
	return pthreads_lock_acquire(PTHREADS_G(lock), PTHREADS_GLOBAL_LOCK_ARGS);
} /* }}} */

/* {{{ pthreads_globals_unlock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC) {
	pthreads_lock_release(PTHREADS_G(lock), PTHREADS_GLOBAL_LOCK_ARGS);
} /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(TSRMLS_D) {
	if (PTHREADS_G(init)) {
		pthreads_lock_free(
			PTHREADS_G(lock) TSRMLS_CC
		);
	}
} /* }}} */
#endif
