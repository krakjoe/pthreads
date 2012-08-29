#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H

extern 	zend_class_entry *	pthreads_class_entry;
static 	zend_object_value 	pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
static 	void 				pthreads_detach_from_instance(void * child TSRMLS_DC);
		void * 				PHP_PTHREAD_ROUTINE(void *);

typedef struct _pthread_construct {						/* Thread internal structure */
	zend_object 			std;						/* standard entry */
	pthread_t				thread;						/* internal thread identifier */
	PEVENT					started;					/* started event */
	PEVENT					finished;					/* finished event */
	zend_function			*runnable;					/* the run method as defined by user */
	zval					result;						/* result from thread */
	void					***pls;						/* local storage of parent thread */
	int						running;					/* flag to indicate thread was started */
	int						joined;						/* flag to indicate thread was joined */
} THREAD, *PTHREAD;

#define PTHREADS_FETCH_FROM_EX(object, ls)				(PTHREAD) zend_object_store_get_object(object, ls)
#define PTHREADS_FETCH_FROM(object)						(PTHREAD) zend_object_store_get_object(object TSRMLS_CC)
#define PTHREADS_FETCH									(PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC)

static zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	zend_object_value attach;
	PTHREAD thread = calloc(1, sizeof(*thread));						/* allocate memory for thread */
	thread->joined = 0;													/* set flags */
	thread->running = 0;												
	thread->pls = tsrm_ls;												/* set local storage of parent */
	thread->started = pthreads_create_event();							/* create events */
	thread->finished = pthreads_create_event();
	zend_object_std_init(&thread->std, entry TSRMLS_CC);				/* initialize standard entry */
	/** @TODO only copy values we can use and do not allow references, heap corruption **/
	/** @TODO possibly allow closures to be passed, they !should work! if copied sizeof(zend_function*) **/
	/** @TODO possibly move this out to thread runtime **/
	zval *temp;
	zend_hash_copy(														/* copy standard properties */
		thread->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
	attach.handle = zend_objects_store_put(								/* standard stuff, store definition */
		thread,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_instance, 
		zend_objects_store_clone_obj TSRMLS_CC
	);
	attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();
	return attach;														/* return attached */
}

static void pthreads_detach_from_instance(void * child TSRMLS_DC){
	PTHREAD thread = (PTHREAD) child;
	if(thread){
		if(thread->thread){												/* be certain we reference an actual thread */
			if(thread->running){										/* if the thread is still running we will join with it */
				if(!thread->joined)										/* if it wasn't already joined */
					pthread_join(thread->thread, NULL);
			}
			if(thread->started)
				pthreads_destroy_event(thread->started);				/* destroy the started event and release it's resources */
			if(thread->finished)
				pthreads_destroy_event(thread->finished);				/* destroy the finished event and release it's resources */
		}
		if(thread->std.properties){
			zend_hash_destroy(
				thread->std.properties									/* destroy standard properties */
			);
			FREE_HASHTABLE(thread->std.properties);						/* free the table they were stored in */
		}
		free(thread);													/* free memory associated with thread */
	}
}

void * PHP_PTHREAD_ROUTINE(void *arg){
	PTHREAD 	thread = (PTHREAD) arg;
	if(thread){															/* I can't imagine a way for this condition to be false ... */
		if(thread->runnable){											/* this should always be true, can't hurt to check */
			HashTable symbols; {										/* copy symbols from class to thread scope while it is locked on event */
				zval *tz;
				zend_hash_init(&symbols, 100, NULL, ZVAL_PTR_DTOR, 0);
				zend_hash_copy(
					&symbols, thread->std.properties, NULL, &tz, sizeof(zval*)
				);
			}
			pthreads_fire_event(thread->started); {						/* allow parent thread to continue */
				void ***newctx = tsrm_new_interpreter_context(); {		/* allocate new interpreter context */
					tsrm_set_interpreter_context(newctx);				/* set context */
					TSRMLS_FETCH_FROM_CTX(newctx);						/* switch to new context local storage */
					zend_activate(TSRMLS_C);							/* activate zend for execution of Thread::run */
					zend_activate_modules(TSRMLS_C);					/* activate extensions */
					zval *returned = &thread->result;					/* vm requires a pointer to a pointer for method return var */
					zend_first_try {									/* using first_try sets a bailout address */
						zend_op_array *op_array = (zend_op_array*) thread->runnable;	/* prepare method for execution */
						EG(in_execution) = 1;											/* set execution flag */
						EG(return_value_ptr_ptr)=&returned;								/* set pointer to the pointer to the return value */
						EG(active_op_array)=op_array;									/* set active op array ( Thread::run ) */
						EG(current_execute_data)=NULL;									/* no current execute data */
						EG(active_symbol_table)=&symbols;								/* set active symbol table to scoped symbols */
						EG(called_scope)=pthreads_class_entry;							/* setting both called scope and scope allows us to define protected methods in Thread */
						EG(scope)=pthreads_class_entry;									/* so this needs setting too */
						zend_execute(EG(active_op_array) TSRMLS_CC);					/* do it */
						if(Z_TYPE_P(returned) != IS_NULL)								/* set result */
							thread->result = *returned;
					} zend_catch {														/* @TODO: report some error somewhere */
						if(Z_TYPE_P(returned) != IS_NULL)
							FREE_ZVAL(returned);
					} zend_end_try();
					zend_deactivate_modules(TSRMLS_C);									/* shutdown modules */
					zend_deactivate(TSRMLS_C);											/* shutdown zend */
					tsrm_free_interpreter_context(newctx);								/* free interpreter context */
				}
			}
		} else pthreads_fire_event(thread->started);				/* fire started in case of error */
	} else pthreads_fire_event(thread->started);					/* fire started in case of error */
	pthreads_fire_event(thread->finished);							/* fire finished event for Userland Thread::busy */
	pthread_exit((void*)0);											/* leave thread */
}
#endif
