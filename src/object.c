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

/* {{{ connect objects */
static void pthreads_connect(PTHREAD source, PTHREAD destination); /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
int pthreads_set_state_ex(PTHREAD thread, int mask, long timeout TSRMLS_DC) {
	int result = 0;
	
	switch(mask){
		case PTHREADS_ST_WAITING: {
			if (pthreads_state_set(thread->state, PTHREADS_ST_WAITING TSRMLS_CC)) {
				result = pthreads_synchro_wait_ex(
					thread->synchro, timeout TSRMLS_CC
				);
			}
		} break;
		
		default: result = pthreads_state_set(thread->state, mask TSRMLS_CC);
	}
	
	return result;
} /* }}} */

/* {{{ set state bits on a thread */
int pthreads_set_state(PTHREAD thread, int mask TSRMLS_DC) {
	return pthreads_set_state_ex(thread, mask, 0L TSRMLS_CC);
} /* }}} */

/* {{{ unset state bits on a thread */
int pthreads_unset_state(PTHREAD thread, int mask TSRMLS_DC){
	int result = 0;
	
	switch(mask) {
		case PTHREADS_ST_WAITING: {
			if (pthreads_state_unset(thread->state, mask TSRMLS_CC)) {
				result = pthreads_synchro_notify(thread->synchro TSRMLS_CC);
			} else result = 0;
		} break;
		
		default: result = pthreads_state_unset(thread->state, mask TSRMLS_CC);
	}
	
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
int pthreads_stack_next(PTHREAD thread TSRMLS_DC) {
	PTHREAD *work = NULL;
	PTHREAD current = NULL;
	int acquire = 0;
	int bubble = 0;
	zval *that_ptr;
	zend_class_entry *scope;
	zend_function *run;
burst:
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		if ((bubble=thread->stack->count) > 0) {
			work = zend_llist_get_first(thread->stack);
			if (work && (current = *work) && current) {
				
				/*
				* Find the class entry for work item
				*/
				{
					if ((scope=pthreads_prepared_entry(thread, current->std.ce TSRMLS_CC))==NULL) {
						if (acquire != EDEADLK) {
							pthread_mutex_unlock(thread->lock);
						}
						return 0;
					}
				}
				
				/*
				* Allocate a $that
				*/
				MAKE_STD_ZVAL(that_ptr);
				
				/*
				* Initialize it with the new entry
				*/
				object_and_properties_init(that_ptr, scope, current->std.properties);
				
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
							pthreads_connect(current, stackable);
							pthreads_state_set(
								current->state, PTHREADS_ST_RUNNING TSRMLS_CC
							);
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
	PTHREAD imported = NULL;
	zend_class_entry *pce;
	
	if (!PTHREADS_IS_CREATOR(thread)) {
		acquire = pthread_mutex_lock(thread->lock);
		
		if (acquire == SUCCESS || acquire == EDEADLK) {
			/*
			* Find the class entry for the selected thread in the current context
			*/
			{
				if ((pce=pthreads_prepared_entry(thread, thread->std.ce TSRMLS_CC))==NULL) {
					if (acquire != EDEADLK) 
						pthread_mutex_unlock(thread->lock);
					imported = NULL;
				}
			}
			
			/*
			* Initialize the object in this context
			*/
			if (pce && object_and_properties_init(*return_value, pce, thread->std.properties)==SUCCESS) {
				if ((imported = PTHREADS_FETCH_FROM(*return_value))!=NULL) {
					/*
					* Connect to the imported thread
					*/
					pthreads_connect(thread, imported);
					
					/*
					* Set the imports significant other
					*/
					imported->sig = thread->sig;
				}
			}
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		}
	}
	return imported != NULL ? 1 : 0;
} /* }}} */

/* {{{ thread object constructor */
zend_object_value pthreads_object_thread_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD thread = calloc(1, sizeof(*thread));
	if (thread) {
		thread->scope |= PTHREADS_SCOPE_THREAD;
		pthreads_base_ctor(thread, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			thread,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ thread connection object constructor */
zend_object_value pthreads_connection_thread_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD thread = calloc(1, sizeof(*thread));
	if (thread) {
		thread->scope |= PTHREADS_SCOPE_THREAD|PTHREADS_SCOPE_CONNECTION;
		pthreads_base_ctor(thread, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			thread,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ worker object constructor */
zend_object_value pthreads_object_worker_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD worker = calloc(1, sizeof(*worker));
	if (worker) {
		worker->scope |= PTHREADS_SCOPE_WORKER;
		pthreads_base_ctor(worker, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			worker,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ worker connection object constructor */
zend_object_value pthreads_connection_worker_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD worker = calloc(1, sizeof(*worker));
	if (worker) {
		worker->scope |= PTHREADS_SCOPE_WORKER|PTHREADS_SCOPE_CONNECTION;
		pthreads_base_ctor(worker, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			worker,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ stackable object constructor */
zend_object_value pthreads_object_stackable_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD stackable = calloc(1, sizeof(*stackable));
	if (stackable) {
		stackable->scope |= PTHREADS_SCOPE_STACKABLE;
		pthreads_base_ctor(stackable, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			stackable,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ stackable connection object constructor */
zend_object_value pthreads_connection_stackable_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD stackable = calloc(1, sizeof(*stackable));
	if (stackable) {
		stackable->scope |= PTHREADS_SCOPE_STACKABLE|PTHREADS_SCOPE_CONNECTION;
		pthreads_base_ctor(stackable, entry TSRMLS_CC);
		attach.handle = zend_objects_store_put(
			stackable,
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_base_dtor, 
			NULL TSRMLS_CC
		);
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ connect pthread objects */
static void pthreads_connect(PTHREAD source, PTHREAD destination) {
	if (source && destination) {
		destination->thread = source->thread;
		destination->tid = source->tid;
		destination->tls = source->tls;
		destination->cid = source->cid;
		destination->synchronized = source->synchronized;
		
		destination->lock = source->lock;
		destination->state = source->state;
		destination->synchro = source->synchro;
		destination->modifiers = source->modifiers;
		destination->store = source->store;
		destination->stack = source->stack;
		destination->status = source->status;
	}
} /* }}} */

/* {{{ pthreads base constructor */
void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry TSRMLS_DC) {

	if (base) {
		zend_object_std_init(&base->std, entry TSRMLS_CC);
#if PHP_VERSION_ID < 50399
		{
			zval *temp;
		
			zend_hash_copy(															
				base->std.properties,
				&entry->default_properties,
				ZVAL_COPY_CTOR,
				&temp, sizeof(zval*)
			);
		}
#else
		object_properties_init(&(base->std), entry);
#endif		

		if (PTHREADS_IS_CONNECTION(base)) {
			base->tid = pthreads_self();
			base->tls = tsrm_ls;
			pthreads_prepare_classes_init(base TSRMLS_CC);
		} else {
			base->cid = pthreads_self();
			base->cls = tsrm_ls;
			if ((base->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t)))!=NULL) {
				pthread_mutex_init(base->lock, &defmutex);
			}
			base->state = pthreads_state_alloc(0 TSRMLS_CC);
			base->synchro = pthreads_synchro_alloc(TSRMLS_C);
			base->modifiers = pthreads_modifiers_alloc(TSRMLS_C);
			base->store = pthreads_serial_alloc(TSRMLS_C);
			base->status = (long*) calloc(1, sizeof(long));
			(*base->status) = 0L;
			
			pthreads_prepare_classes_init(base TSRMLS_CC);
			pthreads_modifiers_init(base->modifiers, entry TSRMLS_CC);
			if (PTHREADS_IS_WORKER(base)) {
				base->stack = (zend_llist*) calloc(1, sizeof(zend_llist));
				zend_llist_init(base->stack, sizeof(void**), NULL, 1);
			}
			
			if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
				pthreads_globals_add(base);
			}
		}
	}
} /* }}} */

/* {{{ pthreads base destructor */
void pthreads_base_dtor(void *arg TSRMLS_DC) {
	PTHREAD base = (PTHREAD) arg;
	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
			if (pthreads_state_isset(base->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
				if (!pthreads_state_isset(base->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
					pthreads_state_set(base->state, PTHREADS_ST_JOINED TSRMLS_CC);
					if (pthread_join(base->thread, NULL)!=SUCCESS) {
						/* do something */
					}
				}
			}
			pthreads_globals_del(base);
		}

		free(base->status);
		if (base->lock) {
			pthread_mutex_destroy(base->lock);
			free(base->lock);
		}
		pthreads_state_free(base->state  TSRMLS_CC);
		pthreads_modifiers_free(base->modifiers TSRMLS_CC);
		pthreads_serial_free(base->store TSRMLS_CC);
		pthreads_synchro_free(base->synchro TSRMLS_CC);
		if (PTHREADS_IS_WORKER(base)) {
			zend_llist_destroy(base->stack);
			free(base->stack);
		}
	}
	
	zend_llist_destroy(&(base->preparation.classes));
	
	if (base->std.properties) {
		zend_hash_destroy(base->std.properties);
		FREE_HASHTABLE(base->std.properties);
		base->std.properties = NULL;
	}
	
#if PHP_VERSION_ID > 50399
	if (base->std.ce->default_properties_count > 0 && base->std.properties_table) {
		int i=0;
		for (;i<base->std.ce->default_properties_count;i++){
			if (base->std.properties_table[i]) {
				zval_ptr_dtor(&base->std.properties_table[i]);
			}
		}
		efree(base->std.properties_table);
	}
#endif
	free(base);
} /* }}} */

/* {{{ this is aptly named ... */
void * PHP_PTHREAD_ROUTINE(void *arg){

	/*
	* The thread object
	*/
	PTHREAD thread = (PTHREAD) arg;

	if (thread) {
	
		/*
		* The connection to the thread object
		*/
		PTHREAD	connection = NULL;
		
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
			zend_class_entry *scope = NULL;
			
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
				* Prepare for execution
				*/
				pthreads_prepare(thread TSRMLS_CC);
				
				/*
				* Initialize thread and connection
				*/
				{
					/*
					* Fetch prepared entry for connection
					*/
					scope = pthreads_prepared_entry
					(
						thread, 
						thread->std.ce TSRMLS_CC
					);
					
					/*
					* Initialize $this
					*/
					object_and_properties_init
					(
						getThis(), 
						scope, 
						thread->std.properties
					);
					
					/*
					* Fetch connection from object storage in this context
					*/
					connection = PTHREADS_FETCH_FROM(getThis());
					
					/*
					* Make connection
					*/
					pthreads_connect(thread, connection);
					
					/*
					* Set significant other and local storage pointers
					*/
					{
						thread->sig = connection;
						connection->sig = thread;
						connection->tls = tsrm_ls;
					}
				}
				
				/*
				* Allow the creating thread to continue where appropriate
				*/
				if (!thread->synchronized)
					pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
				
				/*
				* Runtime ...
				*/
				{
					/*
					* Now time to execute ::run
					*/
					zend_try {
						EG(called_scope) = scope;
						EG(scope) = scope;
						EG(This) = getThis();
				
						do {	
							zend_function *run;
							zval *return_value;
							
							if (zend_hash_find(&EG(scope)->function_table, "run", sizeof("run"), (void**) &run)==SUCCESS) {
								/* Distinct *return and ::run */
								EG(in_execution) = 1;
								EG(active_op_array)=(zend_op_array*)run;
								EG(return_value_ptr_ptr)=&return_value;
								/* Do the dance ... */
								zend_execute
								(
									EG(active_op_array) TSRMLS_CC
								);
#if PHP_VERSION_ID > 50399
								/* Deal with cache ... */
								{
									zend_op_array *ran = &run->op_array;
									
									if (ran->run_time_cache) {
										efree(ran->run_time_cache);
										ran->run_time_cache = NULL;
									}
								}
#endif	
								/* Handle *return from ::run */
								if (return_value) {
									switch(Z_TYPE_P(return_value)){
										case IS_NULL: (*connection->status) = 0L; break;
										case IS_LONG: (*connection->status) = Z_LVAL_P(return_value); break;
										case IS_DOUBLE: (*connection->status) = (long) Z_LVAL_P(return_value); break;
										case IS_BOOL: (*connection->status) = (long) Z_BVAL_P(return_value); break;
										
										default:
											zend_error_noreturn(E_WARNING, "pthreads has detected that %s::run (%lu) attempted to return an unsupported type", EG(scope)->name, connection->tid);
											(*connection->status) = 0L;
									}
									
									FREE_ZVAL(return_value);
								} else (*connection->status) = 0L;
							} else zend_error_noreturn(E_ERROR, "pthreads has experienced an internal error while trying to execute %s::run", EG(scope)->name);
						} while(PTHREADS_IS_WORKER(thread) && pthreads_stack_next(thread TSRMLS_CC));
					} zend_catch {
						
					} zend_end_try();
				}
			} zend_catch {
				/* do something, it's all gone wrong */
			} zend_end_try();
			
			/*
			* Unset running flag
			*/
			pthreads_unset_state(thread, PTHREADS_ST_RUNNING TSRMLS_CC);
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
