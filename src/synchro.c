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

extern pthread_mutexattr_t defmutex;

/* {{{ allocate (and initialize) a synchronization object */
pthreads_synchro pthreads_synchro_alloc(TSRMLS_D) {
	pthreads_synchro sync = (pthreads_synchro) calloc(1, sizeof(*sync));
	
	if (sync) {
		sync->waiting = 0;
		sync->hanging = 0;
		
		if (pthread_mutex_init(&sync->wait, &defmutex)==SUCCESS) {
			if (pthread_cond_init(&sync->notify, NULL)==SUCCESS &&
				pthread_cond_init(&sync->hang, NULL)==SUCCESS) {
				return sync;
			} else free(sync);
		} else free(sync);
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
		if (pthread_mutex_lock(&sync->wait) == SUCCESS) {
			if (sync->hanging) {
				sync->hanging = 0;
				if (pthread_cond_broadcast(&sync->hang)!=SUCCESS) {
					/* report error */
				}
			}
			
			sync->waiting = 1;
			do {
				if (timeout > 0L) {
					result = pthread_cond_timedwait(&sync->notify, &sync->wait, &until);
				} else { result = pthread_cond_wait(&sync->notify, &sync->wait); }
			} while(sync->waiting);
			
			pthread_mutex_unlock(&sync->wait);	
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
		if (pthread_mutex_lock(&sync->wait) == SUCCESS) {
			if (!sync->waiting) {
				sync->hanging = 1;
				do {
					if(pthread_cond_wait(&sync->hang, &sync->wait)!=SUCCESS){
						/* report error */
						break;
					}
				} while(sync->hanging);
			}
			
			sync->waiting = 0;
			if ((result = pthread_cond_broadcast(&sync->notify))!=SUCCESS) {
				/* report error */
			}
			
			pthread_mutex_unlock(&sync->wait);
		} else { /* report error */ }
	} else { /* report unknown error */ }
	return (result == SUCCESS) ? 1 : 0;
} /* }}} */

/* {{{ free synchronization object */
void pthreads_synchro_free(pthreads_synchro sync TSRMLS_DC) {
	pthread_cond_destroy(&sync->notify);
	pthread_mutex_destroy(&sync->wait);
	free(sync);
} /* }}} */

#endif
