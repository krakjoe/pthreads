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
#ifndef HAVE_PTHREADS_HANDLERS
#define HAVE_PTHREADS_HANDLERS

#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <ext/pthreads/src/handlers.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <ext/pthreads/src/thread.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <ext/pthreads/src/object.h>
#endif

/* {{ read_property */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	int oacquire = 0;
	
	zval **lookup = NULL;
	zval *property = NULL;
	char *serial = NULL;
	
	if (thread) {
		acquire = pthread_mutex_lock(PTHREADS_IS_IMPORT(thread) ? thread->sig->lock : thread->lock);
		
		if (acquire == SUCCESS || acquire == EDEADLK) {
			if (!PTHREADS_IS_SELF(thread) && !PTHREADS_IS_IMPORT(thread)) {
				if (pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING)) {
					oacquire = pthread_mutex_lock(thread->sig->lock);

					if (oacquire == SUCCESS || oacquire == EDEADLK) {
						PTHREADS_IMPORT_PROPERTY(thread->sig, member, lookup, property);
						if (oacquire != EDEADLK)
							pthread_mutex_unlock(thread->sig->lock);
					} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
				}
			} else if (PTHREADS_IS_IMPORT(thread) && pthreads_state_isset(thread->sig->state, PTHREADS_ST_RUNNING)) {
				oacquire = pthread_mutex_lock(thread->sig->sig->lock);
				if (oacquire == SUCCESS || oacquire == EDEADLK) {
					PTHREADS_IMPORT_PROPERTY(thread->sig->sig, member, lookup, property);
					if (oacquire != EDEADLK)
						pthread_mutex_unlock(thread->sig->sig->lock);
				}
			}
			
			if (property == NULL)
				property = zsh->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(PTHREADS_IS_IMPORT(thread) ? thread->sig->lock : thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return property;
} /* }}} */

/* {{{ write_property will never attempt to write any other object other than itself */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = PTHREADS_LOCK(thread);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				PTHREADS_UNLOCK(thread);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ has_property */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	int result = 0;
	
	if (thread) {
		acquire = PTHREADS_LOCK(thread);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			result = zsh->has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				PTHREADS_UNLOCK(thread);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return result;
} /* }}} */

/* {{{ unset_property will never attempt to write any object other than itself */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = PTHREADS_LOCK(thread);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				PTHREADS_UNLOCK(thread);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

#endif
