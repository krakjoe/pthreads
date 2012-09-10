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
* NOTE: This is likely to be refactored into pthreads_sync.h
*/
#ifndef HAVE_PTHREADS_EVENT_H
#define HAVE_PTHREADS_EVENT_H

/* {{{ defaults */
extern pthread_mutexattr_t defmutex;
/* }}} */

/* {{{ structs */
typedef struct _pthread_event {
	pthread_mutex_t *change;
	pthread_mutex_t *lock;
	pthread_cond_t *cond;
	int wait;
	int fire;
} EVENT, *PEVENT;
/* }}} */

/* {{{ prototypes */
PEVENT pthreads_create_event();
int pthreads_wait_event(PEVENT event);
int pthreads_fire_event(PEVENT event);
void pthreads_destroy_event(PEVENT event);

/* }}} */

/* {{{ macros */
#define PTHREADS_E(t) t->sync
#define PTHREADS_E_CREATE(t) t->sync=pthreads_create_event()
#define PTHREADS_E_FIRE(t) pthreads_fire_event(t->sync)
#define PTHREADS_E_WAIT(t) pthreads_wait_event(t->sync)
#define PTHREADS_E_WAIT_EX(t, u) pthreads_wait_event_ex(t->sync, u)
#define PTHREADS_E_DESTROY(t) pthreads_destroy_event(t->sync)
#define PTHREADS_IS_BLOCKING(t) (PTHREADS_E(t)->wait > PTHREADS_E(t)->fire)
/* }}} */

/* {{{ Will allocate and initialize a new event */
PEVENT pthreads_create_event(){							
	PEVENT event = (PEVENT) calloc(1, sizeof(EVENT));
	
	if (event) {
		event->change = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
		event->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
		event->cond = (pthread_cond_t*) calloc(1, sizeof(pthread_cond_t));								
		event->wait = 0;
		event->fire = 0;
		
		if (event->lock && event->cond) {
			pthread_mutex_init(event->change, &defmutex);
			pthread_mutex_init(event->lock, &defmutex);
			pthread_cond_init(event->cond, NULL);
		}
	}
	return event;
}
/* }}} */

/* {{{ Will cause the calling thread to block until the event is fired */
#define pthreads_wait_event(e)	pthreads_wait_event_ex(e, 0L);
/* }}} */

/* {{{ Will cause the calling thread to block until the event is fired or the optional timeout has passed 
	If event was fired before the waiter got here this call will return after recording wait */
int pthreads_wait_event_ex(PEVENT event, long timeout) {
	int result = 0;
	int fires;
	
	if (event != NULL) {
		pthread_mutex_lock(event->change);
		if (event->fire < ++event->wait) {
			fires = event->fire;
			pthread_mutex_unlock(event->change);
			pthread_mutex_lock(event->lock);
			if (timeout > 0L) {
				struct timeval now;
				if (gettimeofday(&now, NULL)==SUCCESS) {
					struct timespec until;
					long	nsec = timeout * 1000;
					if (nsec > 1000000000L) {
						until.tv_sec = now.tv_sec + (nsec / 1000000000L);
						until.tv_nsec = (now.tv_usec * 1000) + (nsec % 1000000000L);
					} else {
						until.tv_sec = now.tv_sec;
						until.tv_nsec = (now.tv_usec * 1000) + timeout;	
					}
wait_ex:
					switch(pthread_cond_timedwait(event->cond, event->lock, &until)){
						case SUCCESS:
							/*
							* Protection from spurious wakeups
							*/
							if (pthread_mutex_lock(event->change)==SUCCESS) {
								if (event->fire <= fires) {
									goto wait_ex;
								} else --event->wait;
								result = 1; 
								pthread_mutex_unlock(event->change);
							} else zend_error(E_ERROR, "The implementation detected an internal error and cannot continue");
						break;
						
						case ETIMEDOUT: 
							zend_error(E_WARNING, "The implementation detected a timeout while waiting for event"); 
							if (pthread_mutex_lock(event->change)==SUCCESS){
								--event->wait;
								pthread_mutex_unlock(event->change);
							} else zend_error(E_ERROR, "The implementation detected an internal error and cannot continue");
							result = 0;
						break;
						
						default: result = 0;
					}
				}
			} else {
wait:
				switch(pthread_cond_wait(event->cond, event->lock)){
					case SUCCESS:
						/*
						* Protection from spurious wakeups
						*/
						if (pthread_mutex_lock(event->change)==SUCCESS) {
							if (event->fire <= fires) {
								goto wait;
							} else --event->wait;
							result = 1; 
							pthread_mutex_unlock(event->change);
						} else zend_error(E_ERROR, "The implementation detected an internal error and cannot continue");
					break;
					
					default: result = 0;
				}
			}
			pthread_mutex_unlock(event->lock);
		} else {
			--event->wait;
			pthread_mutex_unlock(event->change);
			result = 1;
		}
	}
	
	return result;
}
/* }}} */

/* {{{ Firing an event causing all waiting threads to continue 
		If there is nobody waiting on event this call will return immediately after recording fire */
int pthreads_fire_event(PEVENT event){
	int result = 0;
	if (event) {
		pthread_mutex_lock(event->change);
		if (event->wait >= ++event->fire){
			do{
				pthread_mutex_unlock(event->change);
				pthread_mutex_lock(event->lock);
				pthread_cond_signal(event->cond);
				pthread_mutex_unlock(event->lock);
				pthread_mutex_lock(event->change);
			} while(event->wait);
			result = 1;
		}
		--event->fire;
		pthread_mutex_unlock(event->change);
	}
	return result;
}
/* }}} */

/* {{{ Will destroy an event, if the event is not fired and there are waiting threads we fire it */
void pthreads_destroy_event(PEVENT event){
	if (event) {	
		if (event->wait > event->fire) 
			pthreads_fire_event(event);
		
		if (event->change) {
			pthread_mutex_destroy(event->change);
			free(event->change);
		}
		
		if (event->lock) {
			pthread_mutex_destroy(event->lock);
			free(event->lock);
		}
		
		if (event->cond) {
			pthread_cond_destroy(event->cond);
			free(event->cond);
		}
		
		free(event);
	}
	event = NULL;
}
/* }}} */
#endif
