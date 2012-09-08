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
#ifndef HAVE_PTHREADS_EVENT_H
#define HAVE_PTHREADS_EVENT_H

/* {{{ structs */
typedef struct _pthread_event {
	pthread_mutex_t			*lock;
	pthread_cond_t			*cond;
	int						fired;
} EVENT, *PEVENT;
/* }}} */

/* {{{ prototypes */
PEVENT 	pthreads_create_event();
int 	pthreads_wait_event(PEVENT event);
void 	pthreads_fire_event(PEVENT event);
void 	pthreads_destroy_event(PEVENT event);
/* }}} */

/* {{{ macros */
#define PTHREADS_STARTED								0
#define PTHREADS_FINISHED								1
#define PTHREADS_WAKE									2
#define PTHREADS_E(t, e)								t->events[e]
#define PTHREADS_E_STARTED(t)							PTHREADS_E(t, PTHREADS_STARTED)
#define PTHREADS_E_FINISHED(t)							PTHREADS_E(t, PTHREADS_FINISHED)
#define PTHREADS_E_WAKE(t)								PTHREADS_E(t, PTHREADS_WAKE)
#define PTHREADS_E_EXISTS(t, e)							(PTHREADS_E(t, e)!=NULL)
#define PTHREADS_E_CREATE(t, e)							PTHREADS_E(t, e)=pthreads_create_event()
#define PTHREADS_E_FIRED(t, e)							(PTHREADS_E(t, e)->fired==1)
#define PTHREADS_E_FIRE(t, e)							pthreads_fire_event(PTHREADS_E(t, e))
#define PTHREADS_E_WAIT(t, e)							pthreads_wait_event(PTHREADS_E(t, e))
#define PTHREADS_E_WAIT_EX(t, e, u)						pthreads_wait_event_ex(PTHREADS_E(t, e), u);
#define PTHREADS_E_DESTROY(t, e)						pthreads_destroy_event(PTHREADS_E(t, e))
#define PTHREADS_E_RESET(t, e)							PTHREADS_E(t, e)->fired=0
#define PTHREADS_IS_RUNNING(t)							(t->running && !t->joined)
#define PTHREADS_IS_JOINED(t)							(t->running && t->joined)
#define PTHREADS_IS_STARTED(t)							PTHREADS_E_STARTED(t)->fired
#define PTHREADS_IS_FINISHED(t)							PTHREADS_E_FINISHED(t)->fired
#define PTHREADS_SET_JOINED(t)							t->joined=1
#define PTHREADS_SET_RUNNING(t)							t->running=1
/* }}} */

/* {{{ Will allocate and initialize a new event */
PEVENT pthreads_create_event(){							
	PEVENT event = (PEVENT) calloc(1, sizeof(EVENT));
	
	if (event) {
		event->lock = (pthread_mutex_t*) 	calloc(1, sizeof(pthread_mutex_t));
		event->cond = (pthread_cond_t*)		calloc(1, sizeof(pthread_cond_t));
		event->fired = 0;								
		
		if (event->lock && event->cond) {
			pthread_mutex_init(event->lock, NULL);
			pthread_cond_init(event->cond, NULL);
		}
	}
	return event;
}
/* }}} */

/* {{{ Will cause the calling thread to block until the event is fired */
#define pthreads_wait_event(e)	pthreads_wait_event_ex(e, 0L);
/* }}} */

/* {{{ Will cause the calling thread to block until the event is fired or the optional timeout has passed */
int pthreads_wait_event_ex(PEVENT event, long timeout) {
	if (event) {
		if (event->lock && event->cond) {
			pthread_mutex_lock(event->lock);
			while (!event->fired) {						
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
						int rc=pthread_cond_timedwait(event->cond, event->lock, &until);
						switch(rc){
							case SUCCESS: return 1;
							
							case ETIMEDOUT: 
								zend_error(E_WARNING, "The implementation detected a timeout while waiting for event"); 
								return 0;
							break;
							
							default:
								zend_error(E_WARNING, "The implementation detected an error while waiting for event: %d", rc);
								return 0;
						}
					} else return 0;
				} else pthread_cond_wait(event->cond, event->lock);
				if (event->fired)
					break;
			}
			
			pthread_mutex_unlock(event->lock);			
			return 1;
		} else return 0;
	} else return 0;
}
/* }}} */

/* {{{ Firing an event causing all waiting threads to continue */
void pthreads_fire_event(PEVENT event){
	if (event) {
		while(!event->fired) {
			event->fired = 1;
			pthread_cond_broadcast(
				event->cond
			);
		}
	}
}
/* }}} */

/* {{{ Will destroy an event, if the event is not fired we wait for it */
void pthreads_destroy_event(PEVENT event){
	if (event) {
		if (pthread_mutex_trylock(event->lock)==EBUSY) {
			pthreads_fire_event(event);
		} else pthread_mutex_unlock(event->lock);
		
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
}
/* }}} */
#endif
