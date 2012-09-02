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
#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H

extern 	zend_class_entry *	pthreads_class_entry;		/* needed here because it is assigned statically to the scope in thread */
static 	zend_object_value 	pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
static 	void 				pthreads_detach_from_instance(void * child TSRMLS_DC);
		char *				pthreads_serialize(zval *unserial TSRMLS_DC);
		zval *				pthreads_unserialize(char *serial TSRMLS_DC);
		int 				pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC);
		void * 				PHP_PTHREAD_ROUTINE(void *);

typedef struct _pthread_construct {						/* Thread internal structure */
	zend_object 			std;						/* standard entry */
	pthread_t				thread;						/* internal thread identifier */
	PEVENT					started;					/* started event */
	PEVENT					finished;					/* finished event */
	zend_function			*prepare;					/* the prepare method as defined by user */
	zend_function			*runnable;					/* the run method as defined by user */
	void					***pls;						/* local storage of parent thread */
	void					***ls;						/* local storage for thread */
	int						running;					/* flag to indicate thread was started */
	int						joined;						/* flag to indicate thread was joined */
	char					*serial;					/* serialized symbols */
} THREAD, *PTHREAD;

#define PTHREADS_FETCH_FROM_EX(object, ls)				(PTHREAD) zend_object_store_get_object(object, ls)
#define PTHREADS_FETCH_FROM(object)						(PTHREAD) zend_object_store_get_object(object TSRMLS_CC)
#define PTHREADS_FETCH									(PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC)

static zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	zend_object_value attach;
	PTHREAD thread = calloc(1, sizeof(*thread));								/* allocate memory for thread */
	
	thread->joined = 		0;													/* set flags */
	thread->running = 		0;													
	/** @TODO both contexts exist during runtime and are both readable, possibly implement some sharing here */
	thread->ls	=			NULL;												/* context for new thread */
	thread->pls = 			tsrm_ls;											/* context for parent */
	thread->started = 		pthreads_create_event();							/* fired when the parent thread can continue */
	/** @NOTE this fires before the zend engine shuts down **/
	thread->finished = 		pthreads_create_event();							/* fired when the method has returned a result */
	
	zend_object_std_init(&thread->std, entry TSRMLS_CC);						/* initialize standard entry */
	
	zval *temp;
	zend_hash_copy(																/* copy standard properties, as yet unused */
		thread->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
	
	attach.handle = zend_objects_store_put(										/* standard stuff, store definition */
		thread,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_instance, 
		zend_objects_store_clone_obj TSRMLS_CC
	);
	
	attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();
	
	return attach;																/* return attached */
}

static void pthreads_detach_from_instance(void * child TSRMLS_DC){
	PTHREAD thread = (PTHREAD) child;
	char *result = NULL;												/* we need a reference to this to free it for the child */
	
	if (thread) {
		if (thread->thread) {												/* be certain we reference an actual thread */
			if (thread->running) {										/* if the thread is still running we will join with it */
				if (!thread->joined) {									/* if it wasn't already joined */
					pthread_join(thread->thread, (void*) &result);
					if (result)											/* check for result and free it's memory */
						free(result);
				}
			}
			
			if (thread->started) {
				pthreads_destroy_event(thread->started); 				/* destroy the started event and release it's resources */
			}
			
			if (thread->finished) {
				pthreads_destroy_event(thread->finished);				/* destroy the finished event and release it's resources */
			}
		}
		
		if (thread->serial){												
			free(thread->serial);										/* free serialized parameters */
		}
		
		if (thread->std.properties){
			zend_hash_destroy(thread->std.properties);
			FREE_HASHTABLE(thread->std.properties);						/* free the table they were stored in */
		}
		
		free(thread);													/* free memory associated with thread */
	}
}

char *	pthreads_serialize(zval *unserial TSRMLS_DC){					/* will return newly allocate buffer with serial zval contained */
	char 					*result = NULL;
	
	if (unserial && Z_TYPE_P(unserial) != IS_NULL) {
		smart_str 				*output;
		php_serialize_data_t 	vars;
		
		PHP_VAR_SERIALIZE_INIT(vars);				
		output = (smart_str*) calloc(1, sizeof(smart_str));
		php_var_serialize(							
			output, 
			&unserial, 
			&vars TSRMLS_CC
		);
		PHP_VAR_SERIALIZE_DESTROY(vars);			
		result = (char*) calloc(1, output->len+1);	
		memcpy(result, output->c, output->len);
		smart_str_free(output);						
		free(output);	
	}							
	return result;														/* remember to free this when you're done with it */
}

int pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC){	/* will unserialize a zval into the existing zval */
	if (serial) {
		const unsigned char *pointer = (const unsigned char *)serial;
		php_unserialize_data_t vars;
		
		PHP_VAR_UNSERIALIZE_INIT(vars);
		if (!php_var_unserialize(&result, &pointer, pointer+strlen(serial), &vars TSRMLS_CC)) {
			
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
			zval_dtor(result);
			zend_error(E_WARNING, "The thread attempted to use symbols that could not be unserialized");
			return FAILURE;
		} else { 
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		}
		
		return SUCCESS;														/* will only destroy the zval if there's an error in deserialization */
	} else return SUCCESS;
}

zval *	pthreads_unserialize(char *serial TSRMLS_DC){					/* will allocate a zval to contain the unserialized data */
	zval *result;
	ALLOC_ZVAL(result);
	
	if (pthreads_unserialize_into(serial, result TSRMLS_CC)==SUCCESS) {
			return result;												/* don't forget to free the variable when you're done with it */
	} else return NULL;													/* will return NULL on failure, maybe should return NULL zval ? */
}

void * PHP_PTHREAD_ROUTINE(void *arg){
	PTHREAD 	thread = (PTHREAD) arg;
	char		*result = NULL;
	
	if (thread) {															/* I can't imagine a way for this condition to be false ... */
		if (thread->runnable) {											/* this should always be true, can't hurt to check */
			pthreads_fire_event(thread->started); {						/* allow parent thread to continue */
				void ***ctx = thread->ls = tsrm_new_interpreter_context(); { /* allocate new interpreter context */
					tsrm_set_interpreter_context(ctx);					/* set context */
					TSRMLS_FETCH_FROM_CTX(ctx);							/* switch to new context local storage */
					zend_activate(TSRMLS_C);							/* activate zend for execution of Thread::run */
					zend_activate_modules(TSRMLS_C);					/* activate extensions */
					zval *retval;										/* pointer to return value */
					zval *symbols;
					
					zend_first_try {									/* using first_try sets a bailout address */
						EG(in_execution) = 1;							/* set execution flag */
						EG(current_execute_data)=NULL;					/* no current execute data */
						EG(called_scope)=pthreads_class_entry;			/* setting both called scope and scope allows us to define protected methods in Thread */
						EG(scope)=pthreads_class_entry;					/* so this needs setting too */
						if(thread->prepare){							/* found Thread::__prepare */
							zval *discard;
							
							EG(return_value_ptr_ptr)=&discard;			/* ignore result of this call */
							EG(active_op_array)=(zend_op_array*) thread->prepare; /* cast method for execution */
							
							zend_execute(EG(active_op_array) TSRMLS_CC);
							
							if(discard && Z_TYPE_P(discard) != IS_NULL)
								FREE_ZVAL(discard);
						}
						
						zend_op_array *op_array = (zend_op_array*) thread->runnable; /* cast method for execution */
						
						EG(active_op_array)=op_array;					/* set active op array ( Thread::run ) */
						if (thread->serial)	{							/* check for symbols */
							symbols = pthreads_unserialize(thread->serial TSRMLS_CC); /* unserialized symbols */
							if (symbols) {
								EG(active_symbol_table)=Z_ARRVAL_P(symbols);
							}
						}
						
						if (!EG(active_symbol_table)) {
							EG(active_symbol_table)=&EG(symbol_table);
						}
						
						EG(return_value_ptr_ptr)=&retval;				/* set pointer to the pointer to the return value */
						zend_execute(EG(active_op_array) TSRMLS_CC);	/** @TODO: set compiled_filename from thread id and something else before here **/
						if (retval && Z_TYPE_P(retval) != IS_NULL) {		/* check for a result */
							result = pthreads_serialize(retval TSRMLS_CC);
							pthreads_fire_event(thread->finished);		/* inform parent we are done, almost */
						} else {
							pthreads_fire_event(thread->finished);		/* no result, method returned null */
						}
						if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
							FREE_ZVAL(symbols);							/* free unserialized symbols */
						}
					} zend_catch {										/** @TODO: report some error somewhere **/
						if (retval && Z_TYPE_P(retval) != IS_NULL) {
							FREE_ZVAL(retval);							/* free return value */
						}
													
						if (symbols && Z_TYPE_P(symbols) != IS_NULL) {	/* free unserialized symbols */
							FREE_ZVAL(symbols);
						}
					} zend_end_try();
					
					zend_deactivate_modules(TSRMLS_C);					/* shutdown modules */
					zend_deactivate(TSRMLS_C);							/* shutdown zend */
					tsrm_free_interpreter_context(ctx);					/* free interpreter context */
					
					thread->ls = NULL;									/* set this to null to stop injection */
				}
			}
		}
	}
	
	if (thread) {
		if (thread->started && !thread->started->fired)					/* firing started even in case of failure */
			pthreads_fire_event(thread->started);
		if (thread->finished && !thread->finished->fired)				/* same with finished */
			pthreads_fire_event(thread->finished);
	}
	
	pthread_exit((void*)result);										/* leave thread */
} 
#endif
