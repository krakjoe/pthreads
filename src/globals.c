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

/* {{{ pthreads_globals_init */
void pthreads_globals_init(TSRMLS_D){
	if (!PTHREADS_G(init)) {
		PTHREADS_G(init)=1;
		PTHREADS_G(peak)=0;
		PTHREADS_G(nid)=1;
		PTHREADS_G(max)=INI_INT("pthreads.max");
		PTHREADS_G(lock)=pthreads_lock_alloc(TSRMLS_C);
		zend_llist_init(&PTHREADS_G(objects), sizeof(void**), NULL, 1);
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

/* {{{ pthreads_globals_count */
size_t pthreads_globals_count(TSRMLS_D) {
	size_t result = 0L;
	zend_bool locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		result = PTHREADS_G(objects).count;
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */

/* {{{ pthreads_globals_add */
void pthreads_globals_add(PTHREAD thread TSRMLS_DC) {
	zend_bool locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		PTHREADS_LIST_INSERT(
			&PTHREADS_G(objects), thread
		);
		if (PTHREADS_G(peak)<PTHREADS_G(objects).count) {
			PTHREADS_G(peak)=PTHREADS_G(objects).count;
		}
		thread->gid = ++PTHREADS_G(nid);
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
} /* }}} */

/* {{{ pthreads_globals_del */
void pthreads_globals_del(PTHREAD thread TSRMLS_DC) {
	zend_bool locked= 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		PTHREADS_LIST_REMOVE(&PTHREADS_G(objects), thread);
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
}  /* }}} */

/* {{{ pthreads_globals_peak */
long pthreads_globals_peak(TSRMLS_D) {
	long result = 0L;
	zend_bool locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		result = PTHREADS_G(peak);
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */


/* {{{ pthreads_globals_fetch */
PTHREAD pthreads_globals_fetch(ulong gid TSRMLS_DC) {
	PTHREAD search = NULL;
	zend_bool fetched = 0;
	zend_bool locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		PTHREADS_LIST_BEGIN_LOOP(&PTHREADS_G(objects), search)
		if (search->gid == gid) {
			fetched = 1;
			break;
		}
		PTHREADS_LIST_END_LOOP(&PTHREADS_G(objects), search)
		pthreads_globals_unlock(locked TSRMLS_CC);
	}
	
	if (fetched) {
		return search;
	} else return NULL;
} /* }}} */
#endif
