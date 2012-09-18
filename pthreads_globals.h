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
#ifndef HAVE_PTHREADS_GLOBALS_H
#define HAVE_PTHREADS_GLOBALS_H

/*
* NOTES
* 1. pthreads cannot use the Zend implementation of globals, it makes for instability
* 2. providing a mechanism for limiting the threads a user can create may be useful to server admin in shared hosting environments
* 3. providing peak usage and current realtime statistics can help you to design and execute more efficiently
* 4. these globals are completely safe; they are protected by mutex
* 5. in some SAPI environments it may not make sense to use pthreads.max ini setting, that is for the server administrator to decide
* 6. in some SAPI environments it may make sense to disable importing Threads for security reasons, pthreads.importing should be set to 0
* 7. pthreads.* ini settings are system only for security
*
* TODO
* 1. look into the possibility of providing persistent threads that survive across SAPI requests
* 2. test read/write lock speed vs mutex
* 3. make errors more meaningful
*/

/* {{{ thread structure and functions */
#ifndef HAVE_PTHREADS_THREAD_H
#	include "pthreads_thread.h"
#endif /* }}} */

/* {{{ default mutex type */
extern pthread_mutexattr_t defmutex;
/* }}} */

/* {{{ pthreads_globals */
struct {
	/*
	* Initialized flag
	* @NOTE the first MINIT of an instance will cause this flag to be set
	*/
	zend_bool init;
	
	/*
	* Globals Mutex
	*/
	pthread_mutex_t	lock;
	
	/*
	* Peak Number of Running Threads
	*/
	size_t peak;
	
	/*
	* Running Thread List
	*/
	zend_llist threads;
} pthreads_globals; /* }}} */

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ PTHREADS_G_INIT */
static inline void pthreads_globals_init(){
	if (!PTHREADS_G(init)) {
		PTHREADS_G(init)=1;
		PTHREADS_G(peak)=0;
		pthread_mutex_init(&PTHREADS_G(lock), &defmutex);
		zend_llist_init(&PTHREADS_G(threads), sizeof(void**), NULL, 1);
	}
} /* }}} */
#define PTHREADS_G_INIT pthreads_globals_init

/* {{{ PTHREADS_G_LOCK */
static inline int pthreads_globals_lock(){
	switch (pthread_mutex_lock(&PTHREADS_G(lock))) {
		case SUCCESS:
		case EDEADLK:
			return 1;
		break;
		
		default: return 0;
	}
} /* }}} */
#define PTHREADS_G_LOCK pthreads_globals_lock

/* {{{ PTHREADS_G_UNLOCK */
static inline void pthreads_globals_unlock() {
	if (!PTHREADS_G(init)) {
		zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	}
	pthread_mutex_unlock(&PTHREADS_G(lock));
} /* }}} */
#define PTHREADS_G_UNLOCK pthreads_globals_unlock

/* {{{ PTHREADS_G_COUNT */
static inline long pthreads_globals_count() {
	long result = 0L;
	if (PTHREADS_G_LOCK()) {
		result = PTHREADS_G(threads).count;
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */
#define PTHREADS_G_COUNT pthreads_globals_count

/* {{{ PTHREADS_G_ADD */
static inline void pthreads_globals_add(PTHREAD thread) {
	if (PTHREADS_G_LOCK()) {
		PTHREADS_LIST_INSERT(&PTHREADS_G(threads), thread);
		if (PTHREADS_G(peak)<PTHREADS_G(threads).count) {
			PTHREADS_G(peak)=PTHREADS_G(threads).count;
		}
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
} /* }}} */
#define PTHREADS_G_ADD(t) pthreads_globals_add(t)

/* {{{ PTHREADS_G_DEL */
static inline void pthreads_globals_del(PTHREAD thread) {
	if (PTHREADS_G_LOCK()) {
		PTHREADS_LIST_REMOVE(&PTHREADS_G(threads), thread);
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
}  /* }}} */
#define PTHREADS_G_DEL(t) pthreads_globals_del(t)

/* {{{ PTHREADS_G_PEAK */
static inline long pthreads_globals_peak() {
	long result = 0L;
	if (PTHREADS_G_LOCK()) {
		result = PTHREADS_G(peak);
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
} /* }}} */
#define PTHREADS_G_PEAK pthreads_globals_peak

/* {{{ PTHREADS_FIND */
static inline PTHREAD pthreads_find(unsigned long tid) {
	PTHREAD search = NULL;
	zend_bool found = 0;
	
	if (PTHREADS_G_LOCK()) {
		PTHREADS_LIST_BEGIN_LOOP(&PTHREADS_G(threads), search)
		if (search->tid == tid) {
			found = 1;
			break;
		}
		PTHREADS_LIST_END_LOOP(&PTHREADS_G(threads), search)
		PTHREADS_G_UNLOCK();
	}
	
	if (found) {
		return search;
	} else return NULL;
	
} /* }}} */
#define PTHREADS_FIND(tid) pthreads_find(tid)

#endif /* HAVE_PTHREADS_GLOBAL_H */
