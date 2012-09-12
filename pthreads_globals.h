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
*/

/* {{{ externs */
extern pthread_mutexattr_t defmutex;
/* }}} */

/* {{{ true globals struct */
struct {
	int init;
	long count;
	long peak;
	pthread_mutex_t	lock;
} pthreads_globals;
/* }}} */

/* {{{ this macro emulates the zend behaviour */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ global macros and supporting inline functions */
static inline void pthreads_globals_init(){
	if (!PTHREADS_G(init)) {
		PTHREADS_G(init)=1;
		pthread_mutex_init(
			&PTHREADS_G(lock), &defmutex
		);
	}
}
#define PTHREADS_G_INIT pthreads_globals_init
static inline int pthreads_globals_lock(){
	switch (pthread_mutex_lock(&PTHREADS_G(lock))) {
		case SUCCESS:
		case EDEADLK:
			return 1;
		break;
		
		default: return 0;
	}
}
#define PTHREADS_G_LOCK pthreads_globals_lock
static inline void pthreads_globals_unlock() {
	if (!PTHREADS_G(init)) {
		zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	}
	pthread_mutex_unlock(&PTHREADS_G(lock));
}
#define PTHREADS_G_UNLOCK pthreads_globals_unlock
static inline long pthreads_globals_count() {
	long result = 0L;
	if (PTHREADS_G_LOCK()) {
		result = PTHREADS_G(count);
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
}
#define PTHREADS_G_COUNT pthreads_globals_count
static inline void pthreads_globals_add() {
	if (PTHREADS_G_LOCK()) {
		PTHREADS_G(count)++;
		if (PTHREADS_G(peak)<PTHREADS_G(count))
			PTHREADS_G(peak)=PTHREADS_G(count);
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
}
#define PTHREADS_G_ADD pthreads_globals_add
static inline void pthreads_globals_del() {
	if (PTHREADS_G_LOCK()) {
		PTHREADS_G(count)--;
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
}
#define PTHREADS_G_DEL pthreads_globals_del
static inline long pthreads_globals_peak() {
	long result = 0L;
	if (PTHREADS_G_LOCK()) {
		result = PTHREADS_G(peak);
		PTHREADS_G_UNLOCK();
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return result;
}
#define PTHREADS_G_PEAK pthreads_globals_peak
#endif /* HAVE_PTHREADS_GLOBAL_H */
