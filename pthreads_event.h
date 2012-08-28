#ifndef HAVE_PTHREADS_EVENT_H
#define HAVE_PTHREADS_EVENT_H
PEVENT pthreads_create_event(){
	PEVENT event = (PEVENT) calloc(1, sizeof(EVENT));
	if(event){
		event->lock = (pthread_mutex_t*) 	calloc(1, sizeof(pthread_mutex_t));
		event->cond = (pthread_cond_t*)		calloc(1, sizeof(pthread_cond_t));
		event->fired = 0;
		if(event->lock && event->cond){
			pthread_mutex_init(event->lock, NULL);
			pthread_cond_init(event->cond, NULL);
		}
	}
	return event;
}

void pthreads_wait_event(PEVENT event){
	if(event){
		if(event->lock && event->cond){
			pthread_mutex_lock(event->lock);
			while(!event->fired){
				pthread_cond_wait(event->cond, event->lock);
				if(event->fired)
					break;
			}
			pthread_mutex_unlock(event->lock);
		}
	}
}

void pthreads_fire_event(PEVENT event){
	if(event){
		while(!event->fired){
			pthread_mutex_lock(event->lock);
			if(!event->fired){
				event->fired = 1;
				pthread_cond_broadcast(
					event->cond
				);
			}
			pthread_mutex_unlock(event->lock);
		}
	}
}

void pthreads_destroy_event(PEVENT event){
	if(event){
		if(!event->fired){
			pthreads_wait_event(
				event
			);
		}
		if(event->lock){
			pthread_mutex_destroy(
				event->lock
			);
			free(event->lock);
		}
		if(event->cond){
			pthread_cond_destroy(event->cond);
			free(
				event->cond
			);
		}
		free(event);
	}
}
#endif
