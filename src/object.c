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

#ifndef HAVE_PTHREADS_COMPAT_H
#	include <src/compat.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

/* {{{ setup reference to self using from as the threading version of $this */
#define PTHREADS_SET_SELF(from) do{\
	self = PTHREADS_FETCH_FROM(from);\
	pthreads_copy(thread, self);\
	thread->sig = self;\
	self->sig = thread;\
	self->tls = tsrm_ls;\
}while(0) /* }}} */

/* {{{ pthreads object handlers */
extern zend_object_handlers poh; /* }}} */

/* {{{ pthreads module entry */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ Empty method entries */
zend_function_entry	pthreads_empty_methods[] = {
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ set worker flag on PTHREAD */
int pthreads_set_worker(PTHREAD thread, zend_bool flag TSRMLS_DC) {
	int acquire = pthread_mutex_lock(thread->lock);
	int result = 0;
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->worker = flag;
		result = 1;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else result = 0;
	return result;
} /* }}} */

/* {{{ get worker flag on PTHREAD */
int pthreads_is_worker(PTHREAD thread TSRMLS_DC) {
	int result;
	int acquire = pthread_mutex_lock(thread->lock);
	
	if (acquire == SUCCESS || acquire == EDEADLK) {
		result = thread->worker;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else result = -1;
	return result;
} /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
int pthreads_set_state_ex(PTHREAD thread, int mask, long timeout TSRMLS_DC) {
	int result = 0;
	
	switch(mask){
		case PTHREADS_ST_WAITING: {
			if (pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC) && 
				!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
				if (pthreads_state_set(thread->state, PTHREADS_ST_WAITING TSRMLS_CC)) {
					result = pthreads_synchro_wait_ex(thread->synchro, timeout TSRMLS_CC);
				}
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
int pthreads_stack_pop_ex(PTHREAD thread, PTHREAD work TSRMLS_DC) {
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
	acquire = pthread_mutex_lock(thread->lock);
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
				MAKE_STD_ZVAL(that_ptr);
				
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
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (bubble && remain < 0 && !pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
		if (pthreads_set_state(thread, PTHREADS_ST_WAITING TSRMLS_CC)) {
			remain = -1;
			goto burst;
		}
	}
	
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
int pthreads_stack_push(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	int acquire = 0;
	int counted = -1;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->worker = 1;
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

/* {{{ return the number of items currently stacked */
int pthreads_stack_length(PTHREAD thread TSRMLS_DC) {
	int acquire = 0;
	int counted = -1;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		counted = thread->stack->count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
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
							
							/*
							* Import user declared methods
							*/
							{
								zend_function *tf;
								zend_hash_copy(
									&pce->function_table, 
									&((thread->std.ce)->function_table),
									(copy_ctor_func_t) function_add_ref,
									&tf, sizeof(zend_function)
								);
							}
							
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
			* Ensure correct handling of the imported object
			*/
			pce->create_object = pthreads_attach_to_other;
			
			/*
			* Initialize the object in this context
			*/
			if (object_init_ex(*return_value, pce)==SUCCESS) {
				imported = PTHREADS_FETCH_FROM(*return_value);
				if (imported) {
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
				pthread_mutex_unlock(thread->lock);
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
	* Lock Initialization
	*/
	instance->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
	if (instance->lock) {
		pthread_mutex_init(instance->lock, &defmutex);
	}
	
	/*
	* State Initialization
	*/
	instance->state = pthreads_state_alloc(0 TSRMLS_CC);
	
	/*
	* Synchro Initialization
	*/
	instance->synchro = pthreads_synchro_alloc(TSRMLS_C);
	
	/*
	* Allocate Modifiers
	*/
	instance->modifiers = pthreads_modifiers_alloc(TSRMLS_C);
	
	/*
	* Allocate Serial Buffers
	*/
	instance->store = pthreads_serial_alloc(TSRMLS_C);
	
	/*
	* Look for private/protected methods
	*/
	pthreads_modifiers_init(instance->modifiers, entry TSRMLS_CC);
	
	/*
	* Allocate Work Buffer
	*/
	instance->stack = (zend_llist*) calloc(1, sizeof(zend_llist));
	
	/*
	* Initialize Work buffer
	*/
	zend_llist_init(instance->stack, sizeof(void**), NULL, 1);
	
	/*
	* This is not a copy
	*/
	instance->copy = 0;
	
	/*
	* Set upon return from Thread::run
	*/
	instance->status = (long*) calloc(1, sizeof(long));
	
	/*
	* Set initial status makes for easier debugging
	*/
	(*instance->status) = 0L;
	
	/*
	* Add thread to global list
	*/
	pthreads_globals_add(instance);
	
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
	* Store the object
	*/
	attach.handle = zend_objects_store_put(
		instance,
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_context, 
		NULL TSRMLS_CC
	);
	
	/*
	* Attach pthreads object handlers
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
	* Attach pthreads object handlers
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
	
	if (arg && thread) {
		/*
		* Remove from global list
		*/
		pthreads_globals_del(thread);
		
		/*
		* Attempt to join unjoined threads
		*/
		if (pthreads_state_isset(thread->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
				pthreads_state_set(thread->state, PTHREADS_ST_JOINED TSRMLS_CC);
				pthread_join(
					thread->thread, NULL
				);
			}
		}
		
		/*
		* Free thread status
		*/
		free(thread->status);
		
		/*
		* Destroy Thread Lock
		*/
		if (thread->lock) {
			pthread_mutex_destroy(thread->lock);
			free(thread->lock);
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
		pthreads_state_free(thread->state  TSRMLS_CC);
		
		/*
		* Free modifiers
		*/
		pthreads_modifiers_free(thread->modifiers TSRMLS_CC);
		
		/*
		* Free serial storage
		*/
		pthreads_serial_free(thread->store TSRMLS_CC);
		
		/*
		* Free synchro
		*/
		pthreads_synchro_free(thread->synchro TSRMLS_CC);
		
		/*
		* Run PHP specific detachment routine on thread
		*/
		pthreads_detach_from_other(thread TSRMLS_CC);
	}
}
/* }}} */

/* {{{ prepares the current context to execute the referenced thread */
static inline void pthreads_prepare(PTHREAD thread TSRMLS_DC){
	HashPosition position;
	zend_class_entry **entry;
	HashTable *source = PTHREADS_CG(thread->cls, class_table);
	HashTable *destination = CG(class_table);
	
	for(zend_hash_internal_pointer_reset_ex(source, &position);
		zend_hash_get_current_data_ex(source, (void**) &entry, &position)==SUCCESS;
		zend_hash_move_forward_ex(source, &position)) {
		char *lcname;
		uint lcnamel;
		ulong idx;
		
		if (zend_hash_get_current_key_ex(source, &lcname, &lcnamel, &idx, 0, &position)==HASH_KEY_IS_STRING) {
			if (!zend_hash_exists(destination, lcname, lcnamel)){
				zend_class_entry ce;
				zend_class_entry *pce;
		
				INIT_CLASS_ENTRY_EX(
					ce, 
					(*entry)->name, 
					(*entry)->name_length,
					pthreads_empty_methods
				);
				ce.create_object = (*entry)->create_object;
				ce.clone = (*entry)->clone;
				ce.serialize = (*entry)->serialize;
				ce.unserialize = (*entry)->unserialize;
				
				pce=zend_register_internal_class(&ce TSRMLS_CC);
				
				{
					zend_function *tf;
					zend_hash_copy(
						&pce->function_table, 
						&((*entry)->function_table),
						(copy_ctor_func_t) function_add_ref,
						&tf, sizeof(zend_function)
					);
				}
				
#if PHP_VERSION_ID > 50399
				pce->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif

			}
		}
	}
} /* }}} */

/* {{{ this is aptly named ... */
void * PHP_PTHREAD_ROUTINE(void *arg){

	/*
	* The thread in the context where it was created
	*/
	PTHREAD thread = (PTHREAD) arg;

	if (thread) {
	
		/*
		* The thread in this context
		*/
		PTHREAD	self = NULL;
		
		/*
		* Thread safe storage
		*/
		void ***tsrm_ls;
		
		/*
		* Startup Block
		*/
		{
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
		}
		
		/*
		* Thread Block
		*/
		{
			/*
			* Pointers to the usual
			*/
			zval *this_ptr = NULL, *return_value = NULL;
			
			/*
			* Set bailout address
			*/
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
				
				/*
				* Initialize the thread in the current context
				*/
				{
					zend_class_entry ce;
					
					INIT_CLASS_ENTRY_EX(
						ce,				
						thread->std.ce->name,
						thread->std.ce->name_length,
						pthreads_empty_methods
					);
					ce.create_object = pthreads_attach_to_other;
					ce.serialize = zend_class_serialize_deny;
					ce.unserialize = zend_class_unserialize_deny;
					object_init_ex(this_ptr, zend_register_internal_class(
						&ce TSRMLS_CC
					));
				}
				
				/*
				* Import the users methods to the current context
				*/
				{
					zend_function *tf;
					
					zend_hash_copy(
						&Z_OBJCE_P(getThis())->function_table, 
						&thread->std.ce->function_table,
						(copy_ctor_func_t) function_add_ref,
						&tf, sizeof(zend_function)
					);
				}
				
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
				* Prepare for execution
				*/
				pthreads_prepare(thread TSRMLS_CC);
				
				/*
				* Allow the creating thread to continue where appropriate
				*/
				if (!thread->synchronized)
					pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
				
#if PHP_VERSION_ID > 50399
				/*
				* Versions above 5.4 have run_time_cache to contend with, handled in compat for now
				*/
				EG(scope)->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif
				
				/*
				* Runtime ...
				*/
				{
					zend_function *run;
					
					if (zend_hash_find(&EG(scope)->function_table, "run", sizeof("run"), (void**) &run)==SUCCESS) {
					
						/*
						* Setup executor for Thread::run
						*/
						EG(return_value_ptr_ptr)=&return_value;
						EG(active_op_array)=(zend_op_array*)run;
						EG(in_execution) = 1;
						
						/*
						* Now time to execute Thread::run
						*/
						zend_try {
							do {
								if (EG(active_op_array)) {
									EG(in_execution) = 1;
									zend_execute(
										EG(active_op_array) TSRMLS_CC
									);
								}
							} while(pthreads_stack_pop(thread, getThis() TSRMLS_CC)>-1);
						} zend_end_try();
						
					} else zend_error_noreturn(E_ERROR, "pthreads has experienced an internal error while trying to execute %s::run", thread->std.ce->name);
				}
				
				/*
				* Remove from global list and set flags
				*/
				pthreads_unset_state(thread, PTHREADS_ST_RUNNING TSRMLS_CC);
				
				/*
				* Store and destroy return value from Thread::run
				*/
				if (return_value) {
					switch(Z_TYPE_P(return_value)){
						case IS_NULL: (*thread->status) = 0L; break;
						case IS_LONG: (*thread->status) = Z_LVAL_P(return_value); break;
						case IS_DOUBLE: (*thread->status) = (long) Z_LVAL_P(return_value); break;
						case IS_BOOL: (*thread->status) = (long) Z_BVAL_P(return_value); break;
						
						default:
							zend_error_noreturn(E_WARNING, "pthreads has detected that %s::run (%lu) attempted to return an unsupported symbol", Z_OBJCE_P(getThis())->name, thread->tid);
							FREE_ZVAL(return_value);
							(*thread->status) = 0L;
					}
				} else (*thread->status) = 0L;
			} zend_catch {	
				if (return_value && Z_TYPE_P(return_value) > IS_BOOL){
					zval_ptr_dtor(&return_value);
				}
			} zend_end_try();
		}
		
		
		
		/*
		* Shutdown Block
		*/
		{
			int acquire = 0;
			
			/*
			* Acquire global lock
			*/
			pthreads_globals_lock();
		
			/*
			* Prepare for shutdown
			*/
			acquire = pthread_mutex_lock(thread->lock);

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
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
				
			/*
			* Release global lock
			*/
			pthreads_globals_unlock();
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
