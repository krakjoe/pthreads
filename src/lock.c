/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ proto pthreads_lock pthreads_lock_alloc(TSRMLS_D)
	shall allocate and initialize a pthreads lock */
pthreads_lock pthreads_lock_alloc(TSRMLS_D) {
	pthreads_lock lock = calloc(1, sizeof(*lock));
	if (lock) {	
		pthread_mutexattr_t attr;

		pthread_mutexattr_init(&attr);

#ifdef PTHREAD_MUTEX_RECURSIVE
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#else
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#endif

		/* whatever the default type will do */
		if (pthread_mutex_init(&lock->mutex, &attr)==SUCCESS) {
			lock->owner = NULL;
			lock->locks = 0;
			return lock;
		}
		free(lock);
	}
	return NULL;
} /* }}} */

/* {{{ proto zend_bool pthreads_lock_acquire(pthreads_lock lock, zend_bool *acquired TSRMLS_DC)
	shall attempt to acquire the referenced lock, setting acquired and returning true on success 
	if the lock is already held by the caller, the lock count is increased, acquired is not set,
	and true is still returned ( since the caller still owns the lock, and has the privilege to
	carry out whatever action they were acqiuring the lock for ) */
zend_bool pthreads_lock_acquire(pthreads_lock lock, zend_bool *acquired TSRMLS_DC) {
	zend_bool locked = 0;
	if (lock) {
		lock->locks++;
		switch (pthread_mutex_lock(&lock->mutex)) {
			case SUCCESS:
				locked = (((*acquired)=1)==1);
				lock->owner = TSRMLS_C;
			break;
			
			default: {
				locked = (((*acquired)=0)==1);
			}
		}
	} else locked = (((*acquired)=0)==1);
	
	return locked;
} /* }}} */

/* {{{ proto zend_bool pthreads_lock_release(pthreads_lock lock, zend_bool acquired TSRMLS_DC)
	acquired should have been set by the accompanying call to pthreads_lock_acquire
	if acquired is set true then the lock shall be released, resetting the owner and decremeneting the count 
	if acquired is set false then the counter shall be decremented without unlocking taking place */
zend_bool pthreads_lock_release(pthreads_lock lock, zend_bool acquired TSRMLS_DC) {
	zend_bool released = 1;
	if (lock) {
		switch (pthread_mutex_unlock(&lock->mutex)) {
			case SUCCESS: 	
				released = 1;
				lock->locks--;
			break;
			
			default: {
				released = 0;
			}
		}
	} else released = 0;

	return released;
} /* }}} */

/* {{{ proto void pthreads_lock_free(pthreads_lock lock TSRMLS_DC) 
	shall deinitialize and free the memory associated with the referenced lock */
void pthreads_lock_free(pthreads_lock lock TSRMLS_DC) {
	if (lock) {
		pthread_mutex_destroy(&lock->mutex);
		
		free(lock);

		lock = NULL;
	}
} /* }}} */
#endif
