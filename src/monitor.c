/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2015                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_MONITOR
#define HAVE_PTHREADS_MONITOR

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

struct _pthreads_monitor_t {
	pthreads_monitor_state_t state;	
	pthread_mutex_t			 mutex;	
	pthread_cond_t			 cond;
};

pthreads_monitor_t* pthreads_monitor_alloc() {
	pthread_mutexattr_t at;
	pthreads_monitor_t *m = 
		(pthreads_monitor_t*) calloc(1, sizeof(pthreads_monitor_t));

	pthread_mutexattr_init(&at);
#if defined(PTHREAD_MUTEX_RECURSIVE) || defined(__FreeBSD__)
	pthread_mutexattr_settype(&at, PTHREAD_MUTEX_RECURSIVE);
#else
	pthread_mutexattr_settype(&at, PTHREAD_MUTEX_RECURSIVE_NP);
#endif
	if (pthread_mutex_init(&m->mutex, &at) != 0) {
		free(m);
		return NULL;
	}

	if (pthread_cond_init(&m->cond, NULL) != 0) {
		pthread_mutex_destroy(&m->mutex);
		free(m);
		return NULL;
	}

	return m;
}

zend_bool pthreads_monitor_lock(pthreads_monitor_t *m) {
	return pthread_mutex_lock(&m->mutex) == 0;
}

zend_bool pthreads_monitor_unlock(pthreads_monitor_t *m) {
	return pthread_mutex_unlock(&m->mutex) == 0;
}

pthreads_monitor_state_t pthreads_monitor_check(pthreads_monitor_t *m, pthreads_monitor_state_t state) {
	return (m->state & state);
}

int pthreads_monitor_wait(pthreads_monitor_t *m, long timeout) {
	struct timeval time;
	struct timespec spec;
	
	if (timeout == 0) {
		return pthread_cond_wait(&m->cond, &m->mutex);
	}
	
	if (gettimeofday(&time, NULL) != 0) {
		return -1;
	}

	time.tv_sec += (timeout / 1000000L);
	time.tv_sec += (time.tv_usec + (timeout % 1000000L)) / 1000000L;
	time.tv_usec = (time.tv_usec + (timeout % 1000000L)) % 1000000L;

	spec.tv_sec = time.tv_sec;
	spec.tv_nsec = time.tv_usec * 1000;

	return pthread_cond_timedwait(&m->cond, &m->mutex, &spec);
}

int pthreads_monitor_notify(pthreads_monitor_t *m) {
	return pthread_cond_broadcast(&m->cond);
}

void pthreads_monitor_wait_until(pthreads_monitor_t *m, pthreads_monitor_state_t state) {
	if (pthreads_monitor_lock(m)) {
		while (!pthreads_monitor_check(m, state)) {
			if (pthreads_monitor_wait(m, 0) != 0) {
				break;
			}
		}
		pthreads_monitor_unlock(m);
	}
}

void pthreads_monitor_set(pthreads_monitor_t *m, pthreads_monitor_state_t state) {
	m->state |= state;
}

void pthreads_monitor_add(pthreads_monitor_t *m, pthreads_monitor_state_t state) {
	if (pthreads_monitor_lock(m)) {
		m->state |= state;
		pthreads_monitor_notify(m);
		pthreads_monitor_unlock(m);
	}
}

void pthreads_monitor_remove(pthreads_monitor_t *m, pthreads_monitor_state_t state) {
	if (pthreads_monitor_lock(m)) {
		m->state &= ~state;
		pthreads_monitor_notify(m);
		pthreads_monitor_unlock(m);
	}
}

void pthreads_monitor_free(pthreads_monitor_t *m) {
	if (pthreads_monitor_lock(m)) {
		pthread_cond_destroy(&m->cond);
		pthread_mutex_destroy(&m->mutex);
	}
	free(m);
}
#endif
