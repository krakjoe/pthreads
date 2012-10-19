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

extern pthread_mutexattr_t defmutex;

pthreads_state pthreads_state_alloc(int mask) {
	pthreads_state state = calloc(1, sizeof(*state));
	if (state != NULL) {
		state->bits = mask;
		if (pthread_mutex_init(&state->lock, &defmutex)==SUCCESS) {
			return state;
		} else free(state);
	}
	return NULL;
}

int pthreads_state_lock(pthreads_state state, int *acquired) {
	switch(pthread_mutex_lock(&state->lock)){
		case SUCCESS: 
			return ((*acquired)=1); 
		break;
		
		case EDEADLK:
			(*acquired)=0;
			return 1;
		break;
		
		default: {
			zend_error(E_WARNING, "pthreads has experienced an internal error and is likely to fail");
			(*acquired)=0;
			return 0;
		}
	}
}

int pthreads_state_unlock(pthreads_state state, int *acquired) {
	if ((*acquired)) {
		if (pthread_mutex_unlock(&state->lock)==SUCCESS) {
			return 1;
		} else return 0;
	} else return 1;
}

void pthreads_state_free(pthreads_state state) {
	if (state) {
		if (pthread_mutex_destroy(&state->lock)==SUCCESS) {
			free(state);
			state=NULL;
		} else zend_error(E_WARNING, "pthreads_state_free failed to destroy state lock");
	} else zend_error(E_WARNING, "pthreads_state_free failed to read state object");
}

int pthreads_state_set(pthreads_state state, int mask) {
	int acquired;
	
	if (state) {
		if (pthreads_state_lock(state, &acquired)) {
			state->bits |= mask;
			return pthreads_state_unlock(state, &acquired);
		} else {
			zend_error_noreturn(E_WARNING, "pthreads_state_set failed to lock state object");
			return 0;
		}
	} else {
		zend_error_noreturn(E_WARNING, "pthreads_state_set failed to read state object");
		return 0;
	}
}

int pthreads_state_isset(pthreads_state state, int mask) {
	int acquired;
	
	if (state) {
		if (pthreads_state_lock(state, &acquired)){
			if (PTHREADS_ST_CHECK(state->bits, mask)) {
				pthreads_state_unlock(state, &acquired);
				return 1;
			} else {
				pthreads_state_unlock(state, &acquired);
				return 0;
			}
		} else {
			zend_error(E_WARNING, "pthreads_state_isset failed to lock state object");
			return 0;
		}
	} else {
		zend_error(E_WARNING, "pthreads_state_isset failed to read state object");
		return 0;
	}
}

int pthreads_state_unset(pthreads_state state, int mask) {
	int acquired;
	
	if (state) {
		if (pthreads_state_lock(state, &acquired)) {
			state->bits &= ~mask;
			return pthreads_state_unlock(state, &acquired);
		} else {
			zend_error(E_WARNING, "pthreads_state_unset failed to lock state object");
			return 0;
		}
	} else {
		zend_error(E_WARNING, "pthreads_state_unset failed to read state object");
		return 0;
	}
}

#endif
