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
#ifndef HAVE_PTHREADS_OBJECT
#define HAVE_PTHREADS_OBJECT

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_PREPARE_H
#	include <src/prepare.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

/* {{{ pthreads module entry */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ base ctor/clone/dtor/free */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry TSRMLS_DC);
static void pthreads_base_dtor(void *arg TSRMLS_DC); 
static void pthreads_base_free(void *arg TSRMLS_DC);
static void pthreads_base_clone(void *arg, void **pclone TSRMLS_DC); /* }}} */

/* {{{ connect objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination TSRMLS_DC); /* }}} */

/* {{{ pthreads routine */
static void * pthreads_routine(void *arg); /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
int pthreads_set_state_ex(PTHREAD thread, int mask, long timeout TSRMLS_DC) {
	int result = 0;
	
	if (mask & PTHREADS_ST_WAITING) {
		int slocked = pthreads_state_lock(thread->state TSRMLS_CC);
		int dowait = 0;
		if (slocked == SUCCESS ||slocked == EDEADLK) {
			dowait = !pthreads_state_check(thread->state, PTHREADS_ST_JOINED TSRMLS_CC);
			if (dowait)
				pthreads_state_set_locked(thread->state, PTHREADS_ST_WAITING TSRMLS_CC);
			if (slocked != EDEADLK)
				pthreads_state_unlock(thread->state TSRMLS_CC);
			if (dowait) {
				result = pthreads_synchro_wait_ex(
					thread->synchro, timeout TSRMLS_CC
				);
			} else result = 0;
		} else result = 0;
	} else result = pthreads_state_set(thread->state, mask TSRMLS_CC);
	return result;
} /* }}} */

/* {{{ set state bits on a thread */
int pthreads_set_state(PTHREAD thread, int mask TSRMLS_DC) {
	return pthreads_set_state_ex(thread, mask, 0L TSRMLS_CC);
} /* }}} */

/* {{{ unset state bits on a thread */
int pthreads_unset_state(PTHREAD thread, int mask TSRMLS_DC){
	int result = 0;
	
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_unset(thread->state, mask TSRMLS_CC)) {
			result = pthreads_synchro_notify(thread->synchro TSRMLS_CC);
		} else result = 0;
	} else result = pthreads_state_unset(thread->state, mask TSRMLS_CC);
	
	return result;
} /* }}} */

/* {{{ pop for php */
int pthreads_stack_pop(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	int acquire = 0;
	int remain = 0;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		if (work) {
			zend_llist_del_element(thread->stack, &work, (int (*)(void *, void *)) pthreads_equal_func);
		} else zend_llist_destroy(thread->stack);
		remain = thread->stack->count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else remain = -1;
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
int pthreads_stack_push(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	int acquire = 0;
	int counted = -1;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		zend_llist_add_element(thread->stack, &work);
		counted = thread->stack->count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (counted > 0) {
		if (pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC)) {
			pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
		}
	}
	
	return counted;
} /* }}} */

/* {{{ pop the next item from the work buffer */
int pthreads_stack_next(PTHREAD thread, zval *this_ptr TSRMLS_DC) {
	PTHREAD *work = NULL;
	PTHREAD current = NULL;
	int acquire = 0;
	int bubble = 0;
	zval *that_ptr;
	zend_function *run;
burst:
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		if ((bubble=thread->stack->count) > 0) {
			work = zend_llist_get_first(thread->stack);
			if (work && (current = *work) && current) {
				/*
				* Allocate a $that
				*/
				MAKE_STD_ZVAL(that_ptr);
				
				/*
				* Initialize it with the new entry
				*/
				object_init_ex(
					that_ptr, 
					pthreads_prepared_entry
					(
						thread, 
						current->std.ce TSRMLS_CC
					)
				);
				
				/*
				* Switch scope to the next stackable
				*/
				if (zend_hash_find(&Z_OBJCE_P(that_ptr)->function_table, "run", sizeof("run"), (void**) &run)==SUCCESS) {
					/*
					* Setup Executor
					*/
					EG(This)=that_ptr;
					EG(scope)=Z_OBJCE_P(that_ptr);
					EG(called_scope)=Z_OBJCE_P(that_ptr);
					
					/*
					* Setup stackable for runtime
					*/
					{
						PTHREAD stackable = PTHREADS_FETCH_FROM(that_ptr);
						
						if (stackable) {
							current->tid = thread->tid;
							current->tls = thread->tls;
							pthreads_connect(current, stackable TSRMLS_CC);
							if (zend_hash_update(
								Z_OBJPROP_P(that_ptr), "worker", sizeof("worker"), 
								(void**) &getThis(), sizeof(zval*), NULL
								)==SUCCESS) {
								Z_ADDREF_P(getThis());
							}
						}
					}
					
					/*
					* Cleanup List
					*/
					zend_llist_del_element(thread->stack, work, (int (*)(void *, void *)) pthreads_equal_func);
				}
			}
		}
		
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	}
	
	if (!bubble) {
		if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			if (pthreads_set_state(thread, PTHREADS_ST_WAITING TSRMLS_CC)) {
				goto burst;
			}
		} else return 0;
	}
	
	return bubble;
} /* }}} */

/* {{{ return the number of items currently stacked */
int pthreads_stack_length(PTHREAD thread TSRMLS_DC) {
	int acquire = 0;
	int counted = -1;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		counted = thread->stack->count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else counted = -1;
	return counted;
} /* }}} */

/* {{{ import a thread created in another context into the current context */
int pthreads_import(PTHREAD thread, zval** return_value TSRMLS_DC){
	int acquire = 0;
	int imported = 1;
	
	if (!PTHREADS_IN_CREATOR(thread)) {
		acquire = pthread_mutex_lock(thread->lock);

		if (acquire == SUCCESS || acquire == EDEADLK) {
			/*
			* Initialize the object in this context
			*/
			if (object_init_ex(
					*return_value, 
					pthreads_prepared_entry
					(
						thread,
						thread->std.ce TSRMLS_CC
					)
				)==SUCCESS) {
				pthreads_connect(thread, PTHREADS_FETCH_FROM(*return_value) TSRMLS_CC);
			} else { imported = 0; }
			
			if (acquire != EDEADLK) {
				pthread_mutex_unlock(thread->lock);
			}
		} else imported = 0;
	} else imported = 0;
	
	return imported;
} /* }}} */

/* {{{ thread object constructor */
zend_object_value pthreads_thread_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD thread = calloc(1, sizeof(*thread));
	if (thread) {
		thread->scope = PTHREADS_SCOPE_THREAD;
		pthreads_base_ctor(thread, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			thread,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ worker object constructor */
zend_object_value pthreads_worker_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD worker = calloc(1, sizeof(*worker));
	if (worker) {
		worker->scope = PTHREADS_SCOPE_WORKER;
		pthreads_base_ctor(worker, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			worker,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ stackable object constructor */
zend_object_value pthreads_stackable_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD stackable = calloc(1, sizeof(*stackable));
	if (stackable) {
		stackable->scope = PTHREADS_SCOPE_STACKABLE;
		pthreads_base_ctor(stackable, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			stackable,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ connect pthread objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination TSRMLS_DC) {
	if (source && destination) {
		if (PTHREADS_IS_NOT_CONNECTION(destination)) {	
			pthreads_base_dtor(destination TSRMLS_CC);
			destination->scope |= PTHREADS_SCOPE_CONNECTION;
			pthreads_base_ctor(destination, destination->std.ce TSRMLS_CC);
			return pthreads_connect
			(
				source,
				destination TSRMLS_CC
			);
		}
		
		destination->thread = source->thread;
		destination->tid = source->tid;
		destination->tls = source->tls;
		destination->cid = source->cid;
		
		destination->lock = source->lock;
		destination->state = source->state;
		destination->synchro = source->synchro;
		destination->modifiers = source->modifiers;
		destination->store = source->store;
		destination->stack = source->stack;
		
		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ pthreads base constructor */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry TSRMLS_DC) {
	if (base) {
		zend_object_std_init(&base->std, entry TSRMLS_CC);
#if PHP_VERSION_ID < 50400
		{
			zval *temp;

			zend_hash_copy(															
				base->std.properties,
				&entry->default_properties,
				(copy_ctor_func_t) zval_add_ref,
				&temp, sizeof(zval*)
			);
		}
#else
		object_properties_init(&(base->std), entry);
#endif	
		if (PTHREADS_IS_CONNECTION(base)) {
			base->tid = pthreads_self();
			base->tls = tsrm_ls;
		} else {
			base->cid = pthreads_self();
			base->cls = tsrm_ls;
			if ((base->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t)))!=NULL) {
				pthread_mutex_init(base->lock, &defmutex);
			}
			base->state = pthreads_state_alloc(0 TSRMLS_CC);
			base->synchro = pthreads_synchro_alloc(TSRMLS_C);
			base->modifiers = pthreads_modifiers_alloc(TSRMLS_C);
			base->store = pthreads_store_alloc(TSRMLS_C);
			
			pthreads_modifiers_init(base->modifiers, entry TSRMLS_CC);
			if (PTHREADS_IS_WORKER(base)) {
				base->stack = (zend_llist*) calloc(1, sizeof(zend_llist));
				zend_llist_init(base->stack, sizeof(void**), NULL, 1);
			}
			
			if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
				pthreads_globals_add(base);
			}
		}
		
		pthreads_prepare_classes_init(base TSRMLS_CC);
	}
} /* }}} */

/* {{{ pthreads base destructor */
static void pthreads_base_dtor(void *arg TSRMLS_DC) {
	PTHREAD base = (PTHREAD) arg;
	
	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
			pthreads_join(base TSRMLS_CC);
			pthreads_globals_del(base);
		}

		if (base->lock) {
			pthread_mutex_destroy(base->lock);
			free(base->lock);
		}
		
		pthreads_state_free(base->state  TSRMLS_CC);
		pthreads_modifiers_free(base->modifiers TSRMLS_CC);
		pthreads_store_free(base->store TSRMLS_CC);
		pthreads_synchro_free(base->synchro TSRMLS_CC);
		if (PTHREADS_IS_WORKER(base)) {
			zend_llist_destroy(base->stack);
			free(base->stack);
		}
	}
	
#if PHP_VERSION_ID > 50399
	{
		zend_object *object = &base->std;
		
		if (object->properties_table) {
			int i;
			for (i = 0; i < object->ce->default_properties_count; i++) {
				if (object->properties_table[i]) {
					zval_ptr_dtor(&object->properties_table[i]);
				}
			}
			efree(object->properties_table);
		}
		
		if (object->properties) {
			zend_hash_destroy(object->properties);
			FREE_HASHTABLE(object->properties);
		}
	}
#else
	zend_object_std_dtor(&(base->std) TSRMLS_CC);
#endif
	
	pthreads_prepare_classes_free(
		base TSRMLS_CC
	);
} /* }}} */

/* {{{ free object */
static void pthreads_base_free(void *arg TSRMLS_DC) {
	PTHREAD base = (PTHREAD) arg;
	if (base) {
		free(base);
	}
} /* }}} */

/* {{{ clone object */
static void pthreads_base_clone(void *arg, void **pclone TSRMLS_DC) {
	printf("pthreads_base_clone: executing ...\n");
} /* }}} */

/* {{{ start a pthread */
int pthreads_start(PTHREAD thread TSRMLS_DC) {
	int dostart = 0;
	int started = FAILURE;
	int tlocked = FAILURE;
	int slocked = pthreads_state_lock(thread->state TSRMLS_CC);
	
	if (slocked == SUCCESS||slocked == EDEADLK) {
		if (!pthreads_state_check(thread->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_STARTED TSRMLS_CC);
			dostart = 1;
		} else started = PTHREADS_ST_STARTED;
		if (slocked != EDEADLK)
			pthreads_state_unlock(thread->state TSRMLS_CC);
	}
	
	if (dostart) {
		tlocked = pthread_mutex_lock(thread->lock);
		
		if (tlocked == SUCCESS||tlocked == EDEADLK) {
			started = pthread_create(&thread->thread, NULL, pthreads_routine, (void*)thread);
			if (started == SUCCESS) 
				pthreads_state_wait(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC);
			if (tlocked != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		}
	}
	
	return started;
} /* }}} */

/* {{{ gracefully join a pthread object */
int pthreads_join(PTHREAD thread TSRMLS_DC) {
	int dojoin = 0;
	int donotify = 0;
	int slocked = pthreads_state_lock(thread->state TSRMLS_CC);
	
	if (slocked == SUCCESS || slocked == EDEADLK) {
		if (pthreads_state_check(thread->state, PTHREADS_ST_STARTED TSRMLS_CC) && 
			!pthreads_state_check(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_JOINED TSRMLS_CC);
			if (PTHREADS_IS_WORKER(thread))
				donotify = pthreads_state_check(thread->state, PTHREADS_ST_WAITING TSRMLS_CC);
			dojoin = 1;
		}
		if (slocked != EDEADLK)
			pthreads_state_unlock(thread->state TSRMLS_CC);
	}
	
	if (donotify) do {
		pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
	} while(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
		
	return dojoin ? pthread_join(thread->thread, NULL) 	: FAILURE;
} /* }}} */

/* {{{ this is aptly named ... */
static void * pthreads_routine(void *arg) {

	/*
	* The thread object
	*/
	PTHREAD thread = (PTHREAD) arg;

	if (thread) {
		/*
		* Thread safe storage
		*/
		void ***tsrm_ls;
		
		/*
		* Global lock acquisition indicator
		*/
		int glocked;
		
		/*
		* Startup Block
		*/
		{
			/*
			* Acquire Global Lock
			*/
			pthreads_globals_lock(&glocked);
			
			/*
			* Allocate Context
			*/
			tsrm_ls = tsrm_new_interpreter_context();
			
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
			* Prepare for Execution
			*/
			pthreads_prepare(thread TSRMLS_CC);
			
			/*
			* Release global lock
			*/
			pthreads_globals_unlock(&glocked);
		}
		
		/*
		* Thread Block
		*/
		{
			/*
			* Pointers to the usual
			*/
			zval *this_ptr = NULL;
			
			/*
			* A new reference to $this for the current context
			*/ 
			MAKE_STD_ZVAL(this_ptr);
				
			/*
			* Set bailout address
			*/
			zend_first_try {
			
				/*
				* First some basic executor setup
				*/
				EG(in_execution) = 1;							
				EG(current_execute_data)=NULL;					
				EG(current_module)=&pthreads_module_entry;
				
				/*
				* Initialize thread and connection
				*/
				{
					/*
					* Initialize $this
					*/
					object_init_ex
					(
						EG(This)=getThis(), 
						EG(scope) = pthreads_prepared_entry
						(
							thread,
							thread->std.ce TSRMLS_CC
						)
					);
					
					/*
					* Make connection
					*/
					pthreads_connect(thread, PTHREADS_FETCH_FROM(EG(This)) TSRMLS_CC);
				}

				/*
				* Runtime ...
				*/
				{	
					/* always the same no point recreating this for every execution */
					zval zmethod;
					
					ZVAL_STRINGL(&zmethod, "run", sizeof("run"), 0);
					
					/*
					* Now time to execute ::run
					*/
					do {	
						zend_function *zrun;
						/* find zrun method */
						if (zend_hash_find(&EG(scope)->function_table, "run", sizeof("run"), (void**) &zrun)==SUCCESS) {
							zval *zresult;
							zend_fcall_info info;
							zend_fcall_info_cache cache;
							
							/* populate a cache and call the run method */
							{
								/* set if not set explicitly elsewhere */
								if (!EG(called_scope)) {
									EG(called_scope)=EG(scope);
								}
								
								/* initialize info object */
								info.size = sizeof(info);
								info.object_ptr = EG(This);
								info.function_name = &zmethod;
								info.retval_ptr_ptr = &zresult;
								info.no_separation = 1;
								info.symbol_table = NULL;
								info.param_count = 0;
								info.params = NULL;
								
								/* initialize cache object */
								cache.initialized = 1;
								cache.function_handler = zrun;
								cache.calling_scope = EG(scope);
								cache.called_scope = EG(called_scope);
								cache.object_ptr = EG(This);
																																																																										
								/* call the function */
								pthreads_state_set((PTHREADS_FETCH_FROM(EG(This)))->state, PTHREADS_ST_RUNNING TSRMLS_CC);
								if (zend_call_function(&info, &cache TSRMLS_CC)!=SUCCESS) {
									pthreads_state_unset((PTHREADS_FETCH_FROM(EG(This)))->state, PTHREADS_ST_RUNNING TSRMLS_CC);
									zend_error_noreturn(E_ERROR, "pthreads has experienced an internal error while calling %s::run", EG(scope)->name);
									break;
								} else pthreads_state_unset((PTHREADS_FETCH_FROM(EG(This)))->state, PTHREADS_ST_RUNNING TSRMLS_CC);
								
#if PHP_VERSION_ID > 50399
								{
									zend_op_array *ops = &zrun->op_array;
								
									if (ops) {
										if (ops->run_time_cache) {
											efree(ops->run_time_cache);
											ops->run_time_cache = NULL;
										}
									}
								}
#endif

								/* deal with zresult (ignored) */
								if (zresult) {
									zval_ptr_dtor(&zresult);
								}
							}
						} else zend_error_noreturn(E_ERROR, "pthreads has experienced an internal error while trying to execute %s::run", EG(scope)->name);
					} while(PTHREADS_IS_WORKER(thread) && pthreads_stack_next(thread, getThis() TSRMLS_CC));
				}
			} zend_catch {
				/* do something, it's all gone wrong */
			} zend_end_try();
		}
		
		/*
		* Shutdown Block
		*/
		{
			/*
			* Acquire global lock
			*/
			pthreads_globals_lock(&glocked);
		
			/*
			* Deactivate Zend
			*/
#if COMPILE_DL_PTHREADS
			php_request_shutdown(TSRMLS_C);
#else
			zend_deactivate_modules(TSRMLS_C);
			zend_deactivate(TSRMLS_C);
#endif

			/*
			* Free Context
			*/
			tsrm_free_interpreter_context(tsrm_ls);	
				
			/*
			* Release global lock
			*/
			pthreads_globals_unlock(&glocked);
		}
	}
	
	/*
	* Donetime ...
	*/
	pthread_exit(NULL);
	
#ifdef _WIN32
	/*
	* Silence MSVC Compiler
	*/
	return NULL;
#endif
} 
/* }}} */

#endif
