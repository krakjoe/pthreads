/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
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
#ifndef HAVE_PTHREADS_SYNCHRO
#define HAVE_PTHREADS_SYNCHRO

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_SYNCHRO_H
#	include <src/synchro.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ allocate (and initialize) a synchronization object */
pthreads_synchro pthreads_synchro_alloc() {
	pthreads_synchro sync = (pthreads_synchro) calloc(1, sizeof(*sync));
	
	if (sync) {
		if ((sync->lock = pthreads_lock_alloc())) {
			if (pthread_cond_init(&sync->notify, NULL)==SUCCESS) {
				return sync;
			}
			pthreads_lock_free(sync->lock);
		}
		free(sync);
	}
	
	return NULL;
} /* }}} */

/* {{{ wait for notification on synchronization object */
int pthreads_synchro_wait_ex(pthreads_synchro sync, long timeout) {
	int result = FAILURE;
	struct timeval time;
	struct timespec spec;
	
	if (timeout>0L) {
		if (gettimeofday(&time, NULL)==SUCCESS) {
			time.tv_sec += (timeout / 1000000L);
			// add extra seconds where they belong
			time.tv_sec += (time.tv_usec + (timeout % 1000000L)) / 1000000L;
			// keep the remaining microseonds
			time.tv_usec = (time.tv_usec + (timeout % 1000000L)) % 1000000L;
		} else
			timeout = 0L;

		if (timeout > 0L) {
			spec.tv_sec = time.tv_sec;
			spec.tv_nsec = time.tv_usec * 1000;
		}
	}
	
	if (sync) {
		if (timeout > 0L) {
			result = pthread_cond_timedwait(&sync->notify, &sync->lock->mutex, &spec);
		} else { result = pthread_cond_wait(&sync->notify, &sync->lock->mutex); }
	} else { /* report unknown error */ }
	
	return (result == SUCCESS) ? 1 : 0;
} /* }}} */

/* {{{ wait for notification on synchronization object */
int pthreads_synchro_wait(pthreads_synchro sync) {
	return pthreads_synchro_wait_ex(sync, 0L);
} /* }}} */

/* {{{ send notification to synchronization object */
int pthreads_synchro_notify(pthreads_synchro sync) {
	int result = FAILURE;
	if (sync) {
		if ((result = pthread_cond_broadcast(&sync->notify))!=SUCCESS) {
				/* report error */
		}
	} else { /* report unknown error */ }
	return (result == SUCCESS) ? 1 : 0;
} /* }}} */

/* {{{ the ability to execute a block of code truly synchronized */
void pthreads_synchro_block(zval *this, zend_fcall_info *info, zend_fcall_info_cache *cache, uint argc, zval *argv, zval *return_value) {
	PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(this));
	zend_bool failed = 0;

	/* set argc and argv for function call */
	zend_fcall_info_argp(info, argc, argv);

	/* set local return value */
	info->retval = return_value;

	info->no_separation = 1;
	info->object = &pobject->std;
	cache->object = &pobject->std;

	/* acquire synchronization lock and execute function synchronized */
	{
		pthreads_synchro_lock(pobject->synchro);
		zend_try {
			/* call the closure */
			zend_call_function(info, cache);
		} zend_catch {
			failed = 1;
		} zend_end_try ();
		pthreads_synchro_unlock(pobject->synchro);
	}
	
	zend_fcall_info_args_clear(info, 1);
	
	/* return the result */
	if (failed) {
		ZVAL_UNDEF(return_value);
	}
} /* }}} */

/* {{{ free synchronization object */
void pthreads_synchro_free(pthreads_synchro sync) {
	pthread_cond_destroy(&sync->notify);
	pthreads_lock_free(sync->lock);
	free(sync);
} /* }}} */

/* {{{ acquire lock internally for userland synchronization */
int pthreads_synchro_lock(pthreads_synchro sync) {
	return pthread_mutex_lock(&sync->lock->mutex);
} /* }}} */

/* {{{ release lock internally for userland synchronization */
int pthreads_synchro_unlock(pthreads_synchro sync) {
	return pthread_mutex_unlock(&sync->lock->mutex);
} /* }}} */

#endif
