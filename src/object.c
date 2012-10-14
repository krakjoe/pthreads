#ifndef HAVE_PTHREADS_OBJECT
#define HAVE_PTHREADS_OBJECT

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_COMPAT_H
#	include <src/compat.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

/* {{{ pthreads object handlers */
extern zend_object_handlers poh; /* }}} */

/* {{{ pthreads module entry */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ Empty method entries */
zend_function_entry	pthreads_empty_methods[] = {
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ set worker flag on PTHREAD */
int pthreads_set_worker(PTHREAD thread, zend_bool flag) {
	int acquire = PTHREADS_LOCK(thread);
	int result = 0;
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->worker = flag;
		result = 1;
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else result = 0;
	return result;
} /* }}} */

/* {{{ get worker flag on PTHREAD */
int pthreads_is_worker(PTHREAD thread) {
	int result;
	int acquire = PTHREADS_LOCK(thread);
	
	if (acquire == SUCCESS || acquire == EDEADLK) {
		result = thread->worker;
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else result = -1;
	return result;
} /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
int pthreads_set_state_ex(PTHREAD thread, int mask, long timeout) {
	struct timeval now;
	struct timespec until;
	int wacquire = 0, timed = 0, result = -1;
	
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
			timed=1;
		}
	}
	
	pthreads_state_set(thread->state, mask);
	
	switch(mask){
		case PTHREADS_ST_WAITING: {
			wacquire = PTHREADS_WLOCK(thread);
			if (wacquire == SUCCESS || wacquire == EDEADLK) {
				do {
					if (timed) {
							result=!pthread_cond_timedwait(thread->sync, thread->wait, &until);
					} else result=!pthread_cond_wait(thread->sync, thread->wait);
				} while(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING));
				if (wacquire != EDEADLK)
					PTHREADS_WUNLOCK(thread);
			} else zend_error(E_WARNING, "pthreads may have expereinced an internal error, I mean, really, who knows ?");
		} break;
		
		case PTHREADS_ST_RUNNING: 	
			pthreads_globals_add(thread); 
		break;
	}
	
	return result;
} /* }}} */

/* {{{ set state bits on a thread */
int pthreads_set_state(PTHREAD thread, int mask) {
	return pthreads_set_state_ex(thread, mask, 0L);
} /* }}} */

/* {{{ unset state bits on a thread */
int pthreads_unset_state(PTHREAD thread, int mask){
	int wacquire = 0;
	
	pthreads_state_unset(thread->state, mask);
	
	switch(mask) {
		case PTHREADS_ST_WAITING: {
			wacquire = PTHREADS_WLOCK(thread);
			
			if (wacquire == SUCCESS || wacquire == EDEADLK) {
				if (pthread_cond_signal(thread->sync)!=SUCCESS) {
					zend_error(E_WARNING, "pthreads failed to notify %lu", thread->tid);
				}
					
				if (wacquire != EDEADLK) {
					PTHREADS_WUNLOCK(thread);
				}
			} else zend_error(E_WARNING, "pthreads failed to acquire wait lock");
		} break;
		
		case PTHREADS_ST_RUNNING: 
			pthreads_globals_del(thread); 
		break;
	}

	return SUCCESS;
} /* }}} */

/* {{{ pop for php */
int pthreads_stack_pop_ex(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	int acquire = 0;
	int remain = 0;
	
	acquire = PTHREADS_LOCK(thread);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		if (work) {
			zend_llist_del_element(thread->stack, &work, (int (*)(void *, void *)) pthreads_equal_func);
		} else zend_llist_destroy(thread->stack);
		remain = thread->stack->count;
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	return remain;
} /* }}} */

/* {{{ pop the next item from the work buffer */
int pthreads_stack_pop(PTHREAD thread, zval *this_ptr TSRMLS_DC) {
	PTHREAD *work = NULL;
	PTHREAD current = NULL;
	int acquire = 0;
	int remain = -1;
	int bubble = 0;
	zval *that_ptr;
	zend_class_entry *pce;
	zend_function *run;
burst:
	acquire = PTHREADS_LOCK(thread);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		bubble = thread->worker;
		if (thread->stack->count > 0) {
			work = zend_llist_get_first(thread->stack);
			if (work && (current = *work) && current) {
				bubble = 0;
				
				/*
				* We create a class entry if we must
				*/
				{
					char *lcname = malloc(thread->std.ce->name_length+1);
					
					if (lcname != NULL) {
						zend_str_tolower_copy(lcname, thread->std.ce->name, thread->std.ce->name_length);
						if (!zend_hash_exists(CG(class_table), lcname, thread->std.ce->name_length+1)) {
							zend_class_entry ce;  {
								INIT_CLASS_ENTRY_EX(
									ce, 
									thread->std.ce->name, 
									thread->std.ce->name_length,
									pthreads_empty_methods
								);
								pce = zend_register_internal_class(&ce TSRMLS_CC);
								
								/*
								* Copy user declared methods
								*/
								zend_hash_copy(
									&pce->function_table,
									&current->std.ce->function_table,
									(copy_ctor_func_t) function_add_ref,
									&run, sizeof(zend_function)
								);
								
#if PHP_VERSION_ID > 50399
								/*
								* Versions above 5.4 have run_time_cache to contend with, handled in compat for now
								*/
								pce->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif
							}
						} else {
							zend_class_entry **ppce;
							if (zend_hash_find(CG(class_table), lcname, thread->std.ce->name_length+1, (void**)&ppce)==SUCCESS){
								pce = *ppce;
							}
						}
						free(lcname);
					} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
				}
				
				/*
				* Allocate a $that
				*/
				ALLOC_ZVAL(that_ptr);
				
				/*
				* Initialize it with the new entry
				*/
				object_init_ex(that_ptr, pce);
				
				/*
				* Setup the executor again
				*/
				if (zend_hash_find(&Z_OBJCE_P(that_ptr)->function_table, "run", sizeof("run"), (void**) &run)==SUCCESS) {
					/*
					* Set next op array
					*/
					EG(active_op_array)=(zend_op_array*)run;
					
					/*
					* Return what is left on stack
					*/
					remain = thread->stack->count;
				}
				
				/*
				* Cleanup List
				*/
				zend_llist_del_element(thread->stack, work, (int (*)(void *, void *)) pthreads_equal_func);
			}
		}
		
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (bubble && remain < 0 && !PTHREADS_IS_JOINED(thread)) {
		if (PTHREADS_WAIT(thread)) {
			remain = -1;
			goto burst;
		}
	}
	
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
int pthreads_stack_push(PTHREAD thread, PTHREAD work) {
	int acquire = 0;
	int counted = -1;
	
	acquire = PTHREADS_LOCK(thread);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->worker = 1;
		zend_llist_add_element(thread->stack, &work);
		counted = thread->stack->count;
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (counted > 0) {
		if (pthreads_state_isset(thread->state, PTHREADS_ST_WAITING)) {
			PTHREADS_NOTIFY(thread);
		}
	}
	
	return counted;
} /* }}} */

/* {{{ return the number of items currently stacked */
int pthreads_stack_length(PTHREAD thread) {
	int acquire = 0;
	int counted = -1;
	
	acquire = PTHREADS_LOCK(thread);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		counted = thread->stack->count;
		if (acquire != EDEADLK)
			PTHREADS_UNLOCK(thread);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return counted;
} /* }}} */

/* {{{ import a thread created in another context into the current context */
int pthreads_import(PTHREAD thread, zval** return_value TSRMLS_DC){
	int acquire = 0;
	PTHREAD imported = NULL;
	zend_class_entry *pce;
	
	if (!PTHREADS_IS_CREATOR(thread)) {
		acquire = PTHREADS_LOCK(thread);
		
		if (acquire == SUCCESS || acquire == EDEADLK) {
			/*
			* Initialize the thread in the current context, once
			*/
			{
				char *lcname = malloc(thread->std.ce->name_length+1);
				
				if (lcname != NULL) {
					zend_str_tolower_copy(lcname, thread->std.ce->name, thread->std.ce->name_length);
					if (!zend_hash_exists(CG(class_table), lcname, thread->std.ce->name_length+1)) {
						zend_class_entry ce;  {
							INIT_CLASS_ENTRY_EX(
								ce, 
								thread->std.ce->name, 
								thread->std.ce->name_length,
								pthreads_empty_methods
							);
							ce.create_object = pthreads_attach_to_other;
							ce.serialize = zend_class_serialize_deny;
							ce.unserialize = zend_class_unserialize_deny;
							pce = zend_register_internal_class(&ce TSRMLS_CC);
						}
					} else {
						zend_class_entry **ppce;
						if (zend_hash_find(CG(class_table), lcname, thread->std.ce->name_length+1, (void**)&ppce)==SUCCESS){
							pce = *ppce;
						}
					}
					free(lcname);
				} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
			}
		
			if (object_init_ex(*return_value, pce)==SUCCESS) {
				imported = PTHREADS_FETCH_FROM(*return_value);
				if (imported) {
					
					/*
					* Copy function table from user declared class
					*/
					{
						zend_function *tf;
						zend_hash_copy(
							&(Z_OBJCE_PP(return_value)->function_table), 
							&((thread->std.ce)->function_table),
							(copy_ctor_func_t) function_add_ref,
							&tf, sizeof(zend_function)
						);
					}
					
#if PHP_VERSION_ID > 50399
					/*
					* Versions above 5.4 have run_time_cache to contend with, handled in compat for now
					*/
					Z_OBJCE_PP(return_value)->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif
					/*
					* Point thread data in the correct places
					*/
					pthreads_copy(thread, imported);
					
					/*
					* Set the imports significant other
					*/
					imported->sig = thread->sig;
				} else zend_error_noreturn(E_WARNING, "pthreads has detected a failure while importing Thread %lu from Thread %lu", thread->tid, thread->cid);
			} else zend_error_noreturn(E_WARNING, "pthreads has detected a failure while trying to initialize imported Thread %lu", thread->tid);
			
			if (acquire != EDEADLK) 
				PTHREADS_UNLOCK(thread);
		}
	} else zend_error_noreturn(E_WARNING, "pthreads has detected an attempt to import a thread that was created in the current context");
	return imported != NULL ? 1 : 0;
} /* }}} */

/* }}} */

/* {{{ this constructor is used to build the original PTHREAD in the creating context and the significant other in the threading context */
zend_object_value pthreads_attach_to_context(zend_class_entry *entry TSRMLS_DC){
#if PHP_VERSION_ID < 50399
	zval *temp; 
#endif
	zend_object_value attach;
	
	/*
	* Allocate an initialize thread object for storage
	*/
	PTHREAD instance = calloc(1, sizeof(*instance));			
	
	/*
	* Set creator information
	*/
	instance->cid = pthreads_self();
	instance->cls = tsrm_ls;
	
	/*
	* State Initialization
	*/
	instance->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
	if (instance->lock) {
		pthread_mutex_init(instance->lock, &defmutex);
		instance->state = pthreads_state_alloc(0);
		instance->wait = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
		if (instance->wait) {
			pthread_mutex_init(instance->wait, &defmutex);
		}
	}
	
	/*
	* Sync Initialization
	*/
	instance->sync = (pthread_cond_t*) calloc(1, sizeof(pthread_cond_t));
	if (instance->sync)
		pthread_cond_init(instance->sync, NULL);
	
	/*
	* Allocate Work Buffer
	*/
	instance->stack = (zend_llist*) calloc(1, sizeof(zend_llist));
	
	/*
	* Initialize Work buffer
	*/
	zend_llist_init(instance->stack, sizeof(void**), NULL, 1);
	
	/*
	* Initialize standard entry
	*/ 
	zend_object_std_init(&instance->std, entry TSRMLS_CC);
	
	/*
	* Initialize instance properties
	*/
#if PHP_VERSION_ID < 50399
	{
		zend_hash_copy(															
			instance->std.properties,
			&entry->default_properties,
			ZVAL_COPY_CTOR,
			&temp, sizeof(zval*)
		);
	}
#else
	object_properties_init(&(instance->std), entry);
#endif		

	/*
	* Allocate Modifiers
	*/
	instance->modifiers = pthreads_modifiers_alloc();
	
	/*
	* Look for private/protected methods
	*/
	pthreads_modifiers_init(instance->modifiers, entry TSRMLS_CC);
	
	/*
	* Signify this is the real thread object
	*/
	instance->copy = 0;
	
	/*
	* Store the object
	*/
	attach.handle = zend_objects_store_put(
		instance,
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_context, 
		NULL TSRMLS_CC
	);
	
	/*
	* Mutex aware handlers
	*/
	attach.handlers = &poh;

	return attach;																
}
/* }}} */

/* {{{ this constructor is used to build a PTHREAD skeleton for use as a base object to represent an imported thread */
zend_object_value pthreads_attach_to_other(zend_class_entry *entry TSRMLS_DC){
#if PHP_VERSION_ID < 50399
	zval *temp;
#endif
	zend_object_value attach;
	
	/*
	* Allocate an initialize thread object for storage
	*/
	PTHREAD other = calloc(1, sizeof(*other));	
	
	/*
	* Standard Entry Initialization
	*/ 
	zend_object_std_init(&other->std, entry TSRMLS_CC);
	
	/*
	* Initialize instance properties
	*/
#if PHP_VERSION_ID < 50399
	{
		zend_hash_copy(															
			other->std.properties,
			&entry->default_properties,
			ZVAL_COPY_CTOR,
			&temp, sizeof(zval*)
		);
	}
#else
	object_properties_init(&(other->std), entry);
#endif

	/*
	* Store the object
	*/
	attach.handle = zend_objects_store_put(
		other,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_other,
		NULL TSRMLS_CC
	);
	
	/*
	* Attach pthreads handlers
	*/
	attach.handlers = &poh;

	return attach;																
}
/* }}} */

void pthreads_detach_from_other(void *arg TSRMLS_DC) {
	PTHREAD thread = (PTHREAD) arg;
	
	if (arg && thread) {
		/*
		* Cleanup function table
		*/
		if (zend_hash_num_elements(&thread->std.ce->function_table)) {
			HashPosition position;
			zend_function *function;
			for (zend_hash_internal_pointer_reset_ex(&thread->std.ce->function_table, &position);
				zend_hash_get_current_data_ex(&thread->std.ce->function_table, (void**)&function, &position)==SUCCESS;
				zend_hash_move_forward_ex(&thread->std.ce->function_table, &position)){
				if (function->type == ZEND_OVERLOADED_FUNCTION) {
					function->type = ZEND_USER_FUNCTION;
				}
			}
		}
		
		/*
		* Destroy object properties
		*/
		if (thread->std.properties) 
		{
			zend_hash_destroy(thread->std.properties);
			FREE_HASHTABLE(thread->std.properties);
			thread->std.properties = NULL;
		}
		
#if PHP_VERSION_ID > 50399
		
		/*
		* 5.4+ requires that you free default properties seperately
		*/
		if (thread->std.ce->default_properties_count > 0 && thread->std.properties_table) {
			int i=0;
			for (;i<thread->std.ce->default_properties_count;i++){
				if (thread->std.properties_table[i]) {
					zval_ptr_dtor(&thread->std.properties_table[i]);
				}
			}
			efree(thread->std.properties_table);
		}
#endif
		
		/*
		* Finally free the thread object
		*/
		free(thread);
	}
}

/* {{{ this destructor handles all types of PTHREAD gracefully */
void pthreads_detach_from_context(void * arg TSRMLS_DC){
	PTHREAD thread = (PTHREAD) arg;
	char *result;
	int worker = 0;
	if (arg && thread) {	
		worker = pthreads_is_worker(thread);
		if (pthreads_state_isset(thread->state, PTHREADS_ST_STARTED)) {
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED)) {
				if (worker && pthreads_state_isset(thread->state, PTHREADS_ST_WAITING)) {
					do {
						PTHREADS_NOTIFY(thread);
					} while(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING));
					pthreads_state_set(thread->state, PTHREADS_ST_JOINED);
				}
				
				if (!worker)
					pthreads_state_set(thread->state, PTHREADS_ST_JOINED);
				if (pthread_join(thread->thread, (void*) &result)==SUCCESS){
					/*
					* Free the result for the thread
					*/
					if (result != NULL)
						free(result);
				}
			}
		}
		
		/*
		* Free whatever is found in the serial buffer
		*/
		if (thread->serial)
			free(thread->serial);
		
		/*
		* Destroy Wait Lock
		*/
		if (thread->wait) {
			pthread_mutex_destroy(thread->wait);
			free(thread->wait);
		}
		
		/*
		* Destroy State Lock
		*/
		if (thread->lock) {
			pthread_mutex_destroy(thread->lock);
			free(thread->lock);
		}
		
		/*
		* Destroy Sync
		*/
		if (thread->sync) {
			pthread_cond_destroy(thread->sync);
			free(thread->sync);
		}
		
		/*
		* Destroy Stack
		*/
		zend_llist_destroy(thread->stack);
		
		/*
		* Free stack list
		*/
		free(thread->stack);
		
		/*
		* Free state object
		*/
		pthreads_state_free(thread->state);
			
		/*
		* Free modifiers
		*/
		pthreads_modifiers_free(thread->modifiers);
		
		/*
		* Run PHP specific detachment routine on thread
		*/
		pthreads_detach_from_other(thread TSRMLS_CC);
	}
}
/* }}} */

/* {{{ this is aptly named ... */
void * PHP_PTHREAD_ROUTINE(void *arg){

	/*
	* The thread in the context where it was created
	*/
	PTHREAD thread = (PTHREAD) arg;
	
	/*
	* The thread in this context
	*/
	PTHREAD	self = NULL;
	
	/*
	* A pointer to the exit value of the thread
	*/
	char *result = NULL;
	
	/*
	* Allocating a new string for the thread class entry in this context ensures a clean, error free shutdown
	*/
	char *rename = NULL;
	
	/*
	* As always, a pointer to $this
	*/
	zval *this_ptr;
	
	/*
	* As always, the return value pointer
	*/
	zval *return_value = NULL;
	
	/*
	* Thread safe property store
	*/
	zval *symbols = NULL;
	
	/*
	* Reference to original properties
	*/
	HashTable *properties = NULL;
	
	/*
	* Class Entry for the Thread in this context
	*/
	zend_class_entry ce;
	
	/*
	* Temporary function pointer
	*/
	zend_function 			*tf;
	
	/*
	* Will signify if the user has declared __prepare magic
	*/
	int preparation = 0;
	
	/*
	* Pointer to __prepare megic implementation
	*/
	zend_function *prepare;				

	/*
	* Pointer to run implementation
	*/
	zend_function *run;

	/*
	* Thread safe storage
	*/
	void ***tsrm_ls;
	
	if (thread) {
		/*
		* Allocate and Initialize an interpreter context for this Thread
		*/
		pthreads_globals_lock();
		tsrm_ls = tsrm_new_interpreter_context(); 
		pthreads_globals_unlock(); 
		
		/*
		* Set Context/TSRMLS
		*/
		tsrm_set_interpreter_context(tsrm_ls);
		
		/*
		* Set Thread Identifier
		*/
		thread->tid = pthreads_self();
		
		/*
		* Activate Zend
		*/
#if COMPILE_DL_PTHREADS
		/*
		* Same server context as parent
		*/
		SG(server_context)=PTHREADS_SG(thread->cls, server_context);
		
		/*
		* Stops PHP from attempting to add headers to null structs
		*/
		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;
		
		/*
		* Startup the request ...
		*/
		php_request_startup(TSRMLS_C);
		
		/*
		* Stop any pointless output from threads
		*/
		SG(headers_sent)=1;
		SG(request_info).no_headers = 1;
#else
		/*
		* Not nearly as much work !!
		*/
		zend_activate(TSRMLS_C);							
		zend_activate_modules(TSRMLS_C);
#endif			
		
		/*
		* Release global lock
		*/
		pthreads_globals_unlock();
		
		zend_first_try {	
			/*
			* A new reference to $this for the current context
			*/ 
			MAKE_STD_ZVAL(this_ptr);
		
			/*
			* First some basic executor setup, using zend_first_try sets a sensible bailout address incase of error
			*/
			EG(in_execution) = 1;							
			EG(current_execute_data)=NULL;					
			EG(current_module)=&pthreads_module_entry;
			CG(compiled_filename)=rename;
			
			/*
			* Get a copy of the name of the users implementation of Thread
			*/
			rename = (char*) calloc(1, thread->std.ce->name_length+1);
			if (rename) {
				memcpy(
					rename, 
					thread->std.ce->name, 
					thread->std.ce->name_length
				);
			}
			
			/*
			* Initialize the thread in the current context
			*/
			INIT_CLASS_ENTRY_EX(
				ce,				
				rename,
				thread->std.ce->name_length,
				pthreads_empty_methods
			);
			ce.create_object = pthreads_attach_to_other;
			ce.serialize = zend_class_serialize_deny;
			ce.unserialize = zend_class_unserialize_deny;
			object_init_ex(this_ptr, zend_register_internal_class(
				&ce TSRMLS_CC
			));
			
			/*
			* Fetches a reference to thread from the current context
			*/
			PTHREADS_SET_SELF(getThis());
			
			/*
			* We can now set the executor and scope to reference a thread safe version of $this
			*/
			EG(called_scope) = Z_OBJCE_P(getThis());
			EG(scope) = Z_OBJCE_P(getThis());
			EG(This) = getThis();
			
			/*
			* Import the users object definition
			*/
			zend_hash_copy(
				&Z_OBJCE_P(getThis())->function_table, 
				&thread->std.ce->function_table,
				(copy_ctor_func_t) function_add_ref,
				&tf, sizeof(zend_function)
			);
			
			/*
			* Allow the creating thread to continue where appropriate
			*/
			if (!thread->synchronized)
				PTHREADS_NOTIFY(thread);
			
			/*
			* Find methods for execution
			*/
			if(zend_hash_find(								
				&Z_OBJCE_P(getThis())->function_table, 
				"__prepare", sizeof("__prepare"), 
				(void**) &prepare
			)==SUCCESS) {
				preparation=1;
			}
			
			zend_hash_find(
				&Z_OBJCE_P(getThis())->function_table, 
				"run", sizeof("run"), 
				(void**) &run
			);
			
#if PHP_VERSION_ID > 50399
			/*
			* Versions above 5.4 have run_time_cache to contend with, handled in compat for now
			*/
			Z_OBJCE_P(getThis())->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif
			
			/*
			* Even if the user does not supply symbols, they may create some
			*/
			if (!EG(active_symbol_table)) {
				EG(active_symbol_table)=&EG(symbol_table);	
			}
			
			/*
			* Test for a preparation method
			*/
			if (preparation) {				
				/*
				* Preparation is run out of context and the return value is ignored
				*/
				zval *discard;
				EG(return_value_ptr_ptr)=&discard;				
				EG(active_op_array)=(zend_op_array*)prepare;
				EG(active_op_array)->filename = rename;
				zend_try {
					zend_execute(EG(active_op_array) TSRMLS_CC);
				} zend_end_try();
				
				if (discard && Z_TYPE_P(discard) != IS_NULL) {
					zval_ptr_dtor(&discard);
				}
			}
			
			/*
			* Setup executor for Thread::run
			*/
			EG(return_value_ptr_ptr)=&return_value;
			EG(active_op_array)=(zend_op_array*)run;
			EG(active_op_array)->filename = rename;
			EG(in_execution) = 1;
			
			/*
			* Unserialize symbols from parent context
			*/
			if (thread->serial)	{											
				symbols = pthreads_unserialize(thread->serial TSRMLS_CC); 	
				if (symbols) {			
					/*
					* Point the properties of the runnable at the thread safe symbols
					*/
					properties = self->std.properties;
					self->std.properties=Z_ARRVAL_P(
						symbols
					);				
				}
				/*
				* Safe to free, no longer required
				*/
				free(thread->serial);
				
				/*
				* Avoid double free
				*/
				thread->serial = NULL;
			}
			
			/*
			* Now time to execute Thread::run
			*/
			zend_try {
				do {
					if (EG(active_op_array)) {
						EG(active_op_array)->filename = rename;
						EG(in_execution) = 1;
						zend_execute(
							EG(active_op_array) TSRMLS_CC
						);
					}
				} while(pthreads_stack_pop(thread, getThis() TSRMLS_CC)>-1);
			} zend_end_try();
			
			/*
			* Serialize and destroy return value
			*/
			if (return_value && Z_TYPE_P(return_value) != IS_NULL) {
				result = pthreads_serialize(
					return_value TSRMLS_CC
				);
				zval_ptr_dtor(&return_value);
			}
			
			/*
			* Serialize and destroy symbols
			*/
			if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
				thread->serial = pthreads_serialize(
					symbols TSRMLS_CC
				);
				zval_ptr_dtor(&symbols);
			}
			
			/*
			* Free zval allocated for $this
			*/
			FREE_ZVAL(this_ptr);
		} zend_catch {	
			if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
				zval_ptr_dtor(&symbols);
			}
			
			if (return_value && Z_TYPE_P(return_value) != IS_NULL){
				zval_ptr_dtor(&return_value);
			}
			
			/*
			* Free zval allocated for $this
			*/
			FREE_ZVAL(this_ptr);
		} zend_end_try();
		
		/*
		* Restoring the pointer to original property table ensures they are free'd
		*/
		if (self)
			self->std.properties = properties;
		
		/*
		* Free the name of the thread
		*/
		if (rename) {
			free(rename);
		}
		
		/*
		* Acquire global lock
		*/
		pthreads_globals_lock();
		
		/*
		* Prepare for shutdown
		*/
		preparation = PTHREADS_LOCK(thread);

		/*
		* Deactivate Zend
		*/
#if COMPILE_DL_PTHREADS
		php_request_shutdown(TSRMLS_C);
#else
		zend_deactivate_modules(TSRMLS_C);
		zend_deactivate(TSRMLS_C);
#endif
		if (preparation != EDEADLK)
			PTHREADS_UNLOCK(thread);
		
		tsrm_free_interpreter_context(tsrm_ls);	
		
		/*
		* Release global lock
		*/
		pthreads_globals_unlock();
	}

	/*
	* Remove from global list and set flags
	*/
	pthreads_unset_state(thread, PTHREADS_ST_RUNNING);


	/*
	* Passing serialized symbols back to thread
	*/
	pthread_exit((void*)result);
	
#ifdef _WIN32
	/*
	* Silence MSVC Compiler
	*/
	return NULL;
#endif
} 
/* }}} */

#endif
