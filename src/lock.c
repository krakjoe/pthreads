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
#ifndef HAVE_PTHREADS_LOCK
#define HAVE_PTHREADS_LOCK

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

pthreads_lock pthreads_lock_alloc(TSRMLS_D) {
	pthreads_lock lock = calloc(1, sizeof(*lock));
	if (lock) {
		pthread_mutexattr_t mutype;
		if (pthread_mutexattr_init(&mutype)==SUCCESS) {
			if (pthread_mutexattr_settype(&mutype, PTHREADS_LOCK_TYPE)==SUCCESS) {
				if (pthread_mutex_init(&lock->mutex, &mutype)==SUCCESS) {
					pthread_mutexattr_destroy(&mutype);
					lock->owner = NULL;
					return lock;
				}
			}
			pthread_mutexattr_destroy(&mutype);
		}
		free(lock);
	}
	return NULL;
}

zend_bool pthreads_lock_acquire(pthreads_lock lock, zend_bool *acquired TSRMLS_DC) {
	if (lock) {
		if (!TSRMLS_C || lock->owner != TSRMLS_C){
			int locked, result = pthread_mutex_lock(&lock->mutex);
			switch (result) {
				case SUCCESS: locked = (((*acquired)=1)==1); break;
				case EDEADLK: locked = (((*acquired)=0)==0); break;
				
				default: {
					zend_error_noreturn(
						E_ERROR, "pthreads has experienced an internal error while acquiring lock @ %p and cannot continue", lock
					);
					locked = (((*acquired)=0)==1);
				}
			}
			
			if (locked)
				lock->owner = tsrm_ls;
			return locked;
		} else return (((*acquired)=0)==0);
	} else return (((*acquired)=0)==1);
}

zend_bool pthreads_lock_release(pthreads_lock lock, zend_bool acquired TSRMLS_DC) {
	if (lock) {
		if (acquired) {
			int result = pthread_mutex_unlock(&lock->mutex);
			switch (result) {
				case SUCCESS: 
					lock->owner = NULL;
				return 1;
				
				default: {
					zend_error_noreturn(
						E_ERROR, 
						"pthreads has experienced an internal error while releasing lock @ %p and cannot continue", 
						lock
					);
					return 0;
				}
			}
		} else return 1;
	} else return 0;
}

void pthreads_lock_free(pthreads_lock lock TSRMLS_DC) {
	if (lock) {
		free(lock);
	}
}
#endif
