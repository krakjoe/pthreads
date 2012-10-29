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
 
/*
* @TODO
*	1. make errors more meaningful, where are my exceptions already !!
*/
#ifndef HAVE_PTHREADS_STATE
#define HAVE_PTHREADS_STATE

#ifndef HAVE_PTHREADS_STATE_H
#	include <src/state.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ allocate state object */
pthreads_state pthreads_state_alloc(int mask TSRMLS_DC) {
	pthreads_state state = calloc(1, sizeof(*state));
	if (state != NULL) {
		state->bits |= mask;
		if (pthread_mutex_init(&state->lock, &defmutex)==SUCCESS) {
			return state;
		} else free(state);
	}
	return NULL;
} /* }}} */

/* {{{ free state object */
void pthreads_state_free(pthreads_state state TSRMLS_DC) {
	if (state) {
		if (pthread_mutex_destroy(&state->lock)==SUCCESS) {
			free(state);
			state=NULL;
		} else zend_error(E_WARNING, "pthreads_state_free failed to destroy state lock");
	} else zend_error(E_WARNING, "pthreads_state_free failed to read state object");
} /* }}} */

/* {{{ lock state */
int pthreads_state_lock(pthreads_state state TSRMLS_DC) {
	return pthread_mutex_lock(&state->lock);
} /* }}} */

/* {{{ unlock state */
int pthreads_state_unlock(pthreads_state state TSRMLS_DC) {
	return pthread_mutex_unlock(&state->lock);
} /* }}} */

/* {{{ check state (assumes appropriate locking in place) */
int pthreads_state_check(pthreads_state state, int mask TSRMLS_DC) {
	return (state->bits & mask)==mask;
} /* }}} */

/* {{{ set state (assumes appropriate locking in place */
int pthreads_state_set_locked(pthreads_state state, int mask TSRMLS_DC) {
	return (state->bits |= mask);
} /* }}} */

/* {{{ unset state ( assumes appropriate locking in place */
int pthreads_state_unset_locked(pthreads_state state, int mask TSRMLS_DC) {
	return (state->bits &= mask);
} /* }}} */

/* {{{ set state on state object */
int pthreads_state_set(pthreads_state state, int mask TSRMLS_DC) {
	int acquired;
	int result = 1;
	if (state) {
		acquired = pthread_mutex_lock(&state->lock);
		if (acquired == SUCCESS ||acquired == EDEADLK) {
			state->bits |= mask;
			pthread_mutex_unlock(&state->lock);
		} else {
			result = 0;
			zend_error_noreturn(E_WARNING, "pthreads_state_set failed to lock state object");
		}
	} else {
		result = 0;
		zend_error_noreturn(E_WARNING, "pthreads_state_set failed to read state object");
	}
	return result;
} /* }}} */

/* {{{ check for state on state object */
int pthreads_state_isset(pthreads_state state, int mask TSRMLS_DC) {
	int acquired;
	int isset = -1;
	
	if (state) {
		acquired = pthread_mutex_lock(&state->lock);
		if (acquired == SUCCESS ||acquired == EDEADLK){
			isset = ((state->bits & mask)==mask);
			if (acquired != EDEADLK)
				pthread_mutex_unlock(&state->lock);
		} else zend_error_noreturn(E_WARNING, "pthreads_state_isset failed to lock state object");
	} else 	zend_error_noreturn(E_WARNING, "pthreads_state_isset failed to read state object");
	
	return isset;
} /* }}} */

/* {{{ unset state on state object */
int pthreads_state_unset(pthreads_state state, int mask TSRMLS_DC) {
	int acquired;
	int result = 1;
	
	if (state) {
		acquired = pthread_mutex_lock(&state->lock);
		if (acquired == SUCCESS ||acquired == EDEADLK) {
			state->bits &= ~mask;
			if (acquired != EDEADLK)
				pthread_mutex_unlock(&state->lock);
		} else {
			result = 0;
			zend_error_noreturn(E_WARNING, "pthreads_state_unset failed to lock state object");
		}
	} else {
		result = 0;
		zend_error_noreturn(E_WARNING, "pthreads_state_unset failed to read state object");
	}
	return result;
} /* }}} */

#endif
