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

extern pthread_mutexattr_t defmutex;

pthreads_synchro pthreads_synchro_alloc() {
	pthreads_synchro sync = (pthreads_synchro) calloc(1, sizeof(*sync));
	
	if (sync) {
		sync->waiting = 0;
		if (pthread_mutex_init(&sync->wait, &defmutex)==SUCCESS &&
			pthread_mutex_init(&sync->notify, &defmutex)==SUCCESS) {
			if (pthread_cond_init(&sync->cond, NULL)==SUCCESS) {
				return sync;
			} else free(sync);
		} else free(sync);
	}
	
	return NULL;
}

int pthreads_synchro_wait_ex(pthreads_synchro sync, long timeout) {
	int result = FAILURE, notified = 0;
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
		if (pthread_mutex_lock(&sync->notify) == SUCCESS) {
			notified = sync->notified;
			sync->waiting = !notified;
			pthread_mutex_unlock(&sync->notify);

			if (pthread_mutex_lock(&sync->wait) == SUCCESS) {
				if (!notified) {
					do {
						if (timeout > 0L) {
							result = pthread_cond_timedwait(&sync->cond, &sync->wait, &until);
						} else { result = pthread_cond_wait(&sync->cond, &sync->wait); }
					} while(sync->waiting);
				} else result = SUCCESS;
				pthread_mutex_unlock(&sync->wait);	
			} else { /* report fatality */ }
		} else { /* report fatality */ }
	} else { /* report unknown error */ }
	
	return (result == SUCCESS) ? 1 : 0;
}

int pthreads_synchro_wait(pthreads_synchro sync) {
	return pthreads_synchro_wait_ex(sync, 0L);
}

int pthreads_synchro_notify(pthreads_synchro sync) {
	int result = FAILURE, notify = 0;
	
	if (sync) {
		if (pthread_mutex_lock(&sync->notify) == SUCCESS) {
			notify = sync->waiting;
			sync->notified = !notify;
			pthread_mutex_unlock(&sync->notify);
			
			if (notify) {
				if (pthread_mutex_lock(&sync->wait) == SUCCESS) {
					sync->waiting = 0;
					if ((result = pthread_cond_broadcast(&sync->cond))!=SUCCESS) {
						/* report error */
					}
					pthread_mutex_unlock(&sync->wait);
				} else { /* report error */ }
			} else result = SUCCESS;
		} else { /* report fatality */ }
	} else { /* report unknown error */ }
	return (result == SUCCESS) ? 1 : 0;
}

void pthreads_synchro_free(pthreads_synchro sync) {
	pthread_cond_destroy(&sync->cond);
	pthread_mutex_destroy(&sync->wait);
	pthread_mutex_destroy(&sync->notify);
	free(sync);
}

#endif
