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

/* {{{ pthreads_globals_init */
void pthreads_globals_init(){
	if (!PTHREADS_G(init)) {
		PTHREADS_G(init)=1;
		PTHREADS_G(peak)=0;
		PTHREADS_G(importing)=INI_BOOL("pthreads.importing");
		PTHREADS_G(max)=INI_INT("pthreads.max");
		pthread_mutex_init(&PTHREADS_G(lock), &defmutex);
		zend_llist_init(&PTHREADS_G(threads), sizeof(void**), NULL, 1);
	}
} /* }}} */

/* {{{ pthreads_globals_lock */
int pthreads_globals_lock(int *acquired){
	switch (pthread_mutex_lock(&PTHREADS_G(lock))) {
		case SUCCESS:
			return (((*acquired)=1)==1);	
		
		case EDEADLK:
			return (((*acquired)=0)==0);
		
		default: return (((*acquired)=0)==1);
	}
} /* }}} */

/* {{{ pthreads_globals_unlock */
void pthreads_globals_unlock(int *acquired) {
	if ((*acquired)==1) {
		pthread_mutex_unlock(&PTHREADS_G(lock));
	}
} /* }}} */

/* {{{ pthreads_globals_count */
size_t pthreads_globals_count() {
	size_t result = 0L;
	int locked;
	
	if (pthreads_globals_lock(&locked)) {
		result = PTHREADS_G(threads).count;
		pthreads_globals_unlock(&locked);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */

/* {{{ pthreads_globals_add */
void pthreads_globals_add(PTHREAD thread) {
	int locked;
	
	if (pthreads_globals_lock(&locked)) {
		PTHREADS_LIST_INSERT(&PTHREADS_G(threads), thread);
		if (PTHREADS_G(peak)<PTHREADS_G(threads).count) {
			PTHREADS_G(peak)=PTHREADS_G(threads).count;
		}
		pthreads_globals_unlock(&locked);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
} /* }}} */

/* {{{ pthreads_globals_del */
void pthreads_globals_del(PTHREAD thread) {
	int locked;
	
	if (pthreads_globals_lock(&locked)) {
		PTHREADS_LIST_REMOVE(&PTHREADS_G(threads), thread);
		pthreads_globals_unlock(&locked);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
}  /* }}} */

/* {{{ pthreads_globals_peak */
long pthreads_globals_peak() {
	long result = 0L;
	int locked;
	
	if (pthreads_globals_lock(&locked)) {
		result = PTHREADS_G(peak);
		pthreads_globals_unlock(&locked);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */

/* {{{ pthreads_globals_find */
PTHREAD pthreads_globals_find(ulong tid) {
	PTHREAD search = NULL;
	zend_bool found = 0;
	int locked = 0;
	
	if (pthreads_globals_lock(&locked)) {
		PTHREADS_LIST_BEGIN_LOOP(&PTHREADS_G(threads), search)
		if (search->tid == tid) {
			found = 1;
			break;
		}
		PTHREADS_LIST_END_LOOP(&PTHREADS_G(threads), search)
		pthreads_globals_unlock(&locked);
	}
	
	if (found) {
		return search;
	} else return NULL;
	
} /* }}} */
#endif
