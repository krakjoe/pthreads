/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2011                                		 |
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

typedef struct _pthread_event {							/* Event internal structure */
	pthread_mutex_t			*lock;						/* mutex for condition */
	pthread_cond_t			*cond;						/* event condition */
	int						fired;						/* flag fired for destruction */
} EVENT, *PEVENT;

PEVENT 	pthreads_create_event();						/* Creates an Event for Thread */
int 	pthreads_wait_event(PEVENT event);				/* Waits for Event to be fired */
void 	pthreads_fire_event(PEVENT event);				/* Fires an Event */
void 	pthreads_destroy_event(PEVENT event);			/* Destroys an Event */

PEVENT pthreads_create_event(){							/* Allocate and initialize mutex and condition for event */
	PEVENT event = (PEVENT) calloc(1, sizeof(EVENT));
	if(event){											/* Out of memory if this fails */
		event->lock = (pthread_mutex_t*) 	calloc(1, sizeof(pthread_mutex_t));
		event->cond = (pthread_cond_t*)		calloc(1, sizeof(pthread_cond_t));
		event->fired = 0;								/* Initialize to 0 for completeness */
		if(event->lock && event->cond){					/* Make sure we got the memory we asked for */
			pthread_mutex_init(event->lock, NULL);		/* And finally initialize the mutex */
			pthread_cond_init(event->cond, NULL);		/* and the condition for event */
		}
	}
	return event;
}

int pthreads_wait_event(PEVENT event){					/* Wait for an event to occur, returns 1 on success */
	if(event){
		if(event->lock && event->cond){
			pthread_mutex_lock(event->lock);			/* While waiting we have to acquire the lock */
			while(!event->fired){						/* We use this opportunity to check that the event still hasn't been fired */
				pthread_cond_wait(event->cond, event->lock); /* Before waiting for it, indefinitely */
				if(event->fired)						/* One last check before we bail */
					break;
			}
			pthread_mutex_unlock(event->lock);			/* Release the mutex for event */
			return 1;									/* An return all good */
		} else return 0;								/* Possibly mid destruction */
	} else return 0;									/* Some sort of internal error */
}

void pthreads_fire_event(PEVENT event){					/* Fire an event, all threads waiting will now unblock and continue executing */
	if(event){
		while(!event->fired){
			pthread_mutex_lock(event->lock);			/* this may seem verbose, but is reliable */
			if(!event->fired){
				event->fired = 1;
				pthread_cond_broadcast(					
					event->cond							/* The bit that tells everyone the event was fired */
				);
			}
			pthread_mutex_unlock(event->lock);
		}
	}
}

void pthreads_destroy_event(PEVENT event){				/* Destroy mutex and condition for event object and free memory associated */
	if(event){
		if(!event->fired){
			pthreads_wait_event(
				event									/* wait for un-fired event to occur */
			);
		}
		if(event->lock){
			pthread_mutex_destroy(
				event->lock								/* destroy the mutex */
			);
			free(event->lock);							/* and free it's memory */
		}
		if(event->cond){
			pthread_cond_destroy(event->cond);			/* destroy the condition */
			free(
				event->cond								/* and free it's memory */
			);
		}
		free(event);									/* free memory associated with event */
	}
}
#endif
