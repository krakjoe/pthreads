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
		PTHREADS_G(peak)=0;
		PTHREADS_G(nid)=1;
		PTHREADS_G(max)=INI_INT("pthreads.max");
		PTHREADS_G(lock)=pthreads_lock_alloc(TSRMLS_C);
		/* trying something with global lists, work in progress */
		zend_ts_hash_init(
			&PTHREADS_G(objects), 10, NULL, NULL, 1
		);
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
		result = zend_ts_hash_num_elements(&PTHREADS_G(objects));
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */

/* {{{ pthreads_globals_add */
void pthreads_globals_add(PTHREAD pobject TSRMLS_DC) {
	zend_bool locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		pobject->gid = ++PTHREADS_G(nid);
		if (zend_ts_hash_index_update(&PTHREADS_G(objects), pobject->gid, pobject, sizeof(*pobject), NULL)==SUCCESS) {
			ulong count = zend_ts_hash_num_elements(&PTHREADS_G(objects));
			if (PTHREADS_G(peak)<count)
				PTHREADS_G(peak)=count;
		}
		pthreads_globals_unlock(locked TSRMLS_CC);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
} /* }}} */

/* {{{ pthreads_globals_del */
void pthreads_globals_del(PTHREAD pobject TSRMLS_DC) {
	zend_bool locked= 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		zend_ts_hash_index_del(&PTHREADS_G(objects), pobject->gid);
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
	PTHREAD pobject = NULL;
	zend_bool fetched = 0, locked = 0;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		if (zend_ts_hash_index_find(&PTHREADS_G(objects), gid, (void**)&pobject)==SUCCESS) {
			fetched = 1;
		}
		pthreads_globals_unlock(locked TSRMLS_CC);
	}
	
	if (fetched) {
		return pobject;
	} else return NULL;
} /* }}} */
#endif
