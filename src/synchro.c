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
#ifndef HAVE_PTHREADS_SYNCHRO
#define HAVE_PTHREADS_SYNCHRO

#ifndef HAVE_PTHREADS_SYNCHRO_H
#	include <src/synchro.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ allocate (and initialize) a synchronization object */
pthreads_synchro pthreads_synchro_alloc(TSRMLS_D) {
	pthreads_synchro sync = (pthreads_synchro) emalloc(sizeof(*sync));
	
	if (sync) {
		if ((sync->lock = pthreads_lock_alloc(TSRMLS_C))) {
			if (pthread_cond_init(&sync->notify, NULL)==SUCCESS) {
				return sync;
			}
			pthreads_lock_free(sync->lock TSRMLS_CC);
		}
		efree(sync);
	}
	
	return NULL;
} /* }}} */

/* {{{ wait for notification on synchronization object */
int pthreads_synchro_wait_ex(pthreads_synchro sync, long timeout TSRMLS_DC) {
	int result = FAILURE;
	struct timeval now;
	struct timespec until;
	
	if (timeout>0L) {
		if (gettimeofday(&now, NULL)==SUCCESS) {
			long nsec = timeout * 1000;
			if (nsec > 1000000000L) {
				until.tv_sec = now.tv_sec + (nsec / 1000000000L);
				until.tv_nsec = (now.tv_usec * 1000) + (nsec % 1000000000L);
			} else {
				until.tv_sec = now.tv_sec;
				until.tv_nsec = (now.tv_usec * 1000) + timeout;	
			}
		} else timeout = 0L;
	}
	
	if (sync) {
		if (pthread_mutex_lock(&sync->lock->mutex) == SUCCESS) {
			if (timeout > 0L) {
				result = pthread_cond_timedwait(&sync->notify, &sync->lock->mutex, &until);
			} else { result = pthread_cond_wait(&sync->notify, &sync->lock->mutex); }
			
			pthread_mutex_unlock(&sync->lock->mutex);	
		} else { /* report fatality */ }
	} else { /* report unknown error */ }
	
	return (result == SUCCESS) ? 1 : 0;
} /* }}} */

/* {{{ wait for notification on synchronization object */
int pthreads_synchro_wait(pthreads_synchro sync TSRMLS_DC) {
	return pthreads_synchro_wait_ex(sync, 0L TSRMLS_CC);
} /* }}} */

/* {{{ send notification to synchronization object */
int pthreads_synchro_notify(pthreads_synchro sync TSRMLS_DC) {
	int result = FAILURE;
	
	if (sync) {
		if (pthread_mutex_lock(&sync->lock->mutex) == SUCCESS) {
			if ((result = pthread_cond_broadcast(&sync->notify))!=SUCCESS) {
				/* report error */
			}
			
			pthread_mutex_unlock(&sync->lock->mutex);
		} else { /* report error */ }
	} else { /* report unknown error */ }
	return (result == SUCCESS) ? 1 : 0;
} /* }}} */

/* {{{ free synchronization object */
void pthreads_synchro_free(pthreads_synchro sync TSRMLS_DC) {
	pthread_cond_destroy(&sync->notify);
	pthreads_lock_free(sync->lock TSRMLS_CC);
	efree(sync);
} /* }}} */

#endif
