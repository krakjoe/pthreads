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

/* {{{ state constants */
#define PTHREADS_ST_STARTED 1
#define PTHREADS_ST_RUNNING 2
#define PTHREADS_ST_JOINED	4
/* }}} */

/* {{{ prototypes */
static 	zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
static 	void pthreads_detach_from_instance(void * child TSRMLS_DC);

zend_function_entry	pthreads_runnable_methods[] = {{NULL, NULL, NULL}};

extern	char * pthreads_serialize(zval *unserial TSRMLS_DC);
extern	zval * pthreads_unserialize(char *serial TSRMLS_DC);
extern	int pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC);

void * PHP_PTHREAD_ROUTINE(void *);
/* }}} */

/* {{{ structs */
typedef struct _pthread_construct {
	/*
	* The Standard Entry
	*/
	zend_object std;
	
	/*
	* The Thread Identifier
	*/
	pthread_t thread;
	
	/*
	* Syncrhonization Event
	*/
	PEVENT sync;	

	/*
	* Thread Safe Local Storage
	*/
	void ***ls;
	
	/*
	* State Management
	*/
	pthread_mutex_t *lock;
	int state;
	
	/*
	* Flags
	*/
	zend_bool self;
	zend_bool synchronized;

	/*
	* Serial Buffer
	*/
	char *serial;
	
	/* 
	* Significant Other
	*/
	struct _pthread_construct *sig;
} THREAD, *PTHREAD;
/* }}} */

/* {{{ state management */
static int pthreads_get_state(PTHREAD thread){
	int result = -1;
	if (pthread_mutex_lock(thread->lock)==SUCCESS) {
		result = thread->state;
		pthread_mutex_unlock(thread->lock);
	}
	return result;
}

static int pthreads_set_state(PTHREAD thread, int state){
	int result = -1;
	if (pthread_mutex_lock(thread->lock)==SUCCESS) {
		thread->state += state;
		result = thread->state;
		pthread_mutex_unlock(thread->lock);
	}
	return result;
}

static int pthreads_unset_state(PTHREAD thread, int state){
	int result = -1;
	if (pthread_mutex_lock(thread->lock)==SUCCESS) {
		thread->state -= state;
		result = thread->state;
		pthread_mutex_unlock(thread->lock);
	}
	return result;
}
/* }}} */

/* {{{ macros */
#define PTHREADS_FETCH_FROM_EX(object, ls) (PTHREAD) zend_object_store_get_object(object, ls)
#define PTHREADS_FETCH_FROM(object) (PTHREAD) zend_object_store_get_object(object TSRMLS_CC)
#define PTHREADS_FETCH (PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC)
#define PTHREADS_SET_SELF(from) do{\
	self = PTHREADS_FETCH_FROM(from);\
	self->self = 1;\
	self->sig = thread;\
	thread->sig = self;\
}while(0)
#define PTHREADS_IS_SELF(t)	(t->self)
#define PTHREADS_SET_JOINED(t) pthreads_set_state(t, PTHREADS_ST_JOINED)
#define PTHREADS_SET_RUNNING(t)	pthreads_set_state(t, PTHREADS_ST_RUNNING)
#define PTHREADS_UNSET_RUNNING(t) pthreads_unset_state(t, PTHREADS_ST_RUNNING)
#define PTHREADS_IS_JOINED(t) ((pthreads_get_state(t) & PTHREADS_ST_JOINED)==PTHREADS_ST_JOINED)
#define PTHREADS_IS_STARTED(t) ((pthreads_get_state(t) & PTHREADS_ST_STARTED)==PTHREADS_ST_STARTED)
#define PTHREADS_IS_RUNNING(t) ((pthreads_get_state(t) & PTHREADS_ST_RUNNING)==PTHREADS_ST_RUNNING)
#define PTHREADS_SET_STARTED(t) pthreads_set_state(t, PTHREADS_ST_STARTED)
#define PTHREADS_ST(t) pthreads_get_state(t)
/* }}} */

/* {{{ compat */
#if PHP_VERSION_ID > 50399
#	include "pthreads_compat.h"
#endif
/* }}} */

/* {{{ */
static zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	
	zend_object_value attach;
	
	/*
	* Allocate an initialize thread object for storage
	*/
	PTHREAD thread = calloc(1, sizeof(*thread));			
	thread->synchronized = 0;
	thread->self = 0;
	thread->ls	= tsrm_ls;
	thread->sig = NULL;
	
	/*
	* State Initialization
	*/
	thread->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
	if (thread->lock)
		pthread_mutex_init(thread->lock, &defmutex);
	thread->state = 0;
	
	/*
	* To be initialized by the calling context
	*/
	thread->sync = NULL;
	
	/*
	* Standard Entry Initialization
	*/ 
	zend_object_std_init(&thread->std, entry TSRMLS_CC);
	
	/*
	* Initialize instance properties
	*/
#if PHP_VERSION_ID < 50399
	 zval *temp;
	 
	 zend_hash_copy(															
		thread->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
#else
	object_properties_init(&(thread->std), entry);
#endif

	/*
	* Store the object
	*/
	attach.handle = zend_objects_store_put(
		thread,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_instance, 
		NULL TSRMLS_CC
	);
	
	/*
	* For now, standard handlers ...
	*/
	attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();

	return attach;																
}
/* }}} */

/* {{{ */
static void pthreads_detach_from_instance(void * arg TSRMLS_DC){

	PTHREAD thread = (PTHREAD) arg;
	
	if (thread) {
		if (!PTHREADS_IS_SELF(thread)) {
			/*
			* If the thread is running we must attempt to join
			*/
			if (PTHREADS_IS_STARTED(thread)) {
				if (!PTHREADS_IS_JOINED(thread)) {
					PTHREADS_SET_JOINED(thread);
					
					/*
					* Get rid of blockages
					*/
					if (PTHREADS_IS_BLOCKING(thread)) {
						PTHREADS_E_FIRE(thread);
					}
					
					/*
					* We must check for a result even when it's ignored
					*/
					char *result = NULL;
					
					pthread_join(thread->thread, (void*) &result);
					
					/*
					* Free the result for the thread
					*/
					if (result)
						free(result);
				}
			}
			PTHREADS_E_DESTROY(thread);
		}
		
		/*
		* Destroy object properties
		*/
		if (thread->std.properties){
			zend_hash_destroy(thread->std.properties);
			FREE_HASHTABLE(thread->std.properties);
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
		* Destroy State Lock
		*/
		if (thread->lock) {
			pthread_mutex_destroy(thread->lock);
			free(thread->lock);
		}

		/*
		* Finally free the thread object
		*/
		free(thread);
	}
}
/* }}} */

/* {{{ */
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
	
	if (thread) {									
	
		/*
		* Allocate and Initialize an interpreter context for this Thread
		*/
		void ***ctx = thread->ls = tsrm_new_interpreter_context(); { 
			
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
			* Set Context/TSRMLS
			*/
			tsrm_set_interpreter_context(ctx);
			TSRMLS_FETCH_FROM_CTX(ctx);		
			
			/*
			* Activate Zend
			*/
			zend_activate(TSRMLS_C);							
			zend_activate_modules(TSRMLS_C);					
			
			/*
			* A new reference to $this for the current context
			*/ 
			MAKE_STD_ZVAL(this_ptr);
			
			zend_first_try {				
			
				/*
				* First some basic executor setup, using zend_first_try sets a sensible bailout address incase of error
				*/
				EG(in_execution) = 1;							
				EG(current_execute_data)=NULL;					
				EG(current_module)=&pthreads_module_entry;
				
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
				zend_class_entry ce; {
					INIT_CLASS_ENTRY_EX(
						ce, 									
						rename,
						thread->std.ce->name_length,
						pthreads_runnable_methods
					);
					ce.create_object = pthreads_attach_to_instance;
					ce.serialize = zend_class_serialize_deny;
					ce.unserialize = zend_class_unserialize_deny;
					object_init_ex(this_ptr, zend_register_internal_class(
						&ce TSRMLS_CC
					));
				}
				
				/*
				* Fetches a reference to thread from the current context
				*/
				PTHREADS_SET_SELF(getThis());

				/*
				* Setup wake event
				*/
				PTHREADS_E(self)=PTHREADS_E(thread);
				
				/*
				* We can now set the executor and scope to reference a thread safe version of $this
				*/
				EG(called_scope) = Z_OBJCE_P(getThis());
				EG(scope) = Z_OBJCE_P(getThis());
				EG(This) = getThis();
				
				/*
				* Import the users object definition
				*/
				zend_function *tf; {
					zend_hash_copy(
						&Z_OBJCE_P(getThis())->function_table, 
						&thread->std.ce->function_table,
						(copy_ctor_func_t) function_add_ref,
						&tf, sizeof(zend_function)
					);
				}
				
				/*
				* Allow the creating thread to continue where appropriate
				*/
				if (!thread->synchronized)
					PTHREADS_E_FIRE(thread);

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
					ALLOC_INIT_ZVAL(discard);
					EG(return_value_ptr_ptr)=&discard;				
					EG(active_op_array)=(zend_op_array*)prepare;
					
					zend_try {
						zend_execute(EG(active_op_array) TSRMLS_CC);
					} zend_end_try();
					
					if (discard && Z_TYPE_P(discard) != IS_NULL) {
						FREE_ZVAL(discard);
					}
				}
				
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
				}
				
				/*
				* Allocate the return value for the next execution
				*/ 
				ALLOC_INIT_ZVAL(return_value);
				
				/*
				* Now time to execute Thread::run
				*/
				EG(return_value_ptr_ptr)=&return_value;					
				EG(active_op_array)=(zend_op_array*)run;
				
				zend_try {
					zend_execute(EG(active_op_array) TSRMLS_CC);
				} zend_end_try();
				
				/*
				* Serialize return value into thread result pointer and free source zval
				*/
				if (return_value && Z_TYPE_P(return_value) != IS_NULL) {
					result = pthreads_serialize(
						return_value TSRMLS_CC
					);
					FREE_ZVAL(return_value);
				}
				
				/*
				* Free symbols
				*/
				if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
					FREE_ZVAL(symbols);
				}
			} zend_catch {	
				
				/*
				* Freeing symbols and return value in case of error
				*/
				if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
					FREE_ZVAL(symbols);
				}
				
				if (return_value && Z_TYPE_P(return_value) != IS_NULL){
					FREE_ZVAL(return_value);
				}
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
			* Deactivate Zend
			*/
			zend_deactivate_modules(TSRMLS_C);
			zend_deactivate(TSRMLS_C);
			
			/*
			* Free Context
			*/
			tsrm_free_interpreter_context(ctx);	
		}
	}

	if (PTHREADS_IS_BLOCKING(thread)) {
		PTHREADS_E_FIRE(thread);
	}
	
	PTHREADS_UNSET_RUNNING(thread);
	
	/*
	* Passing serialized symbols back to thread
	*/
	pthread_exit((void*)result);
} 
/* }}} */

#endif /* HAVE_PTHREADS_OBJECT_H */
