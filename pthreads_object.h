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

/*
* NOTES
*	1. Not happy with inability to detect scope properly in Threads, too many annoyances to be acceptable
*	2. Private/Protected Members in user declared Threads not working properly as a result of the above
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* {{{ state constants */
#define PTHREADS_ST_STARTED 1
#define PTHREADS_ST_RUNNING 2
#define PTHREADS_ST_JOINED	4
#define PTHREADS_ST_WAITING	8
#define PTHREADS_ST_CHECK(state, bits)	((state & bits)==bits)
/* }}} */

/* {{{ THREAD structure */
#ifndef HAVE_PTHREADS_THREAD_H
#	include "pthreads_thread.h"
#endif
/* }}} */

/* {{{ Empty method entries */
zend_function_entry	pthreads_empty_methods[] = {
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ prototypes */
zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
zend_object_value pthreads_attach_to_import(zend_class_entry *entry TSRMLS_DC);
void pthreads_detach_from_instance(void * child TSRMLS_DC);
void * PHP_PTHREAD_ROUTINE(void *);
/* }}} */

/* {{{ macros */

/* {{{ inline static prototypes support macros */
static inline int pthreads_is_worker(PTHREAD thread);
static inline int pthreads_set_worker(PTHREAD thread, zend_bool flag);
static inline int pthreads_get_state(PTHREAD thread);
static inline int pthreads_set_state_ex(PTHREAD thread, int state, long timeout);
static inline int pthreads_set_state(PTHREAD thread, int state);
static inline int pthreads_unset_state(PTHREAD thread, int state);
static inline int pthreads_import(PTHREAD thread, zval **return_value TSRMLS_DC);
static inline int pthreads_pop(PTHREAD thread, zval *this_ptr TSRMLS_DC);
static inline int pthreads_pop_ex(PTHREAD thread, PTHREAD work TSRMLS_DC);
static inline int pthreads_push(PTHREAD thread, PTHREAD work);
static inline int pthreads_get_stacked(PTHREAD thread);
/* }}} */

/* {{{ fetches a PTHREAD from a specific object in a specific context */
#define PTHREADS_FETCH_FROM_EX(object, ls) (PTHREAD) zend_object_store_get_object(object, ls) /* }}} */

/* {{{ fetches a PTHREAD from a specific object in the current context */
#define PTHREADS_FETCH_FROM(object) (PTHREAD) zend_object_store_get_object(object TSRMLS_CC) /* }}} */

/* {{{ fetches the current PTHREAD from $this */
#define PTHREADS_FETCH (PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC) /* }}} */

/* {{{ tell if a referenced PTHREAD is threading version of the users object */
#define PTHREADS_IS_SELF(t)	(t->self) /* }}} */

/* {{{ tell if the current thread created referenced PTHREAD */
#define PTHREADS_IS_CREATOR(t)	(t->cid == pthreads_self()) /* }}} */

/* {{{ get the import flag on a PTHREAD */
#define PTHREADS_IS_IMPORT(t) t->import /* }}} */

/* {{{ set the worker flag on a PTHREAD */
#define PTHREADS_SET_WORKER pthreads_set_worker /* }}} */

/* {{{ get the worker flag for a PTHREAD */
#define PTHREADS_IS_WORKER pthreads_is_worker /* }}} */

/* {{{ get state from a PTHREAD */
#define PTHREADS_ST(t) pthreads_get_state(t) /* }}} */

/* {{{ tell if a PTHREAD is blocking */
#define PTHREADS_IS_BLOCKING(t) ((PTHREADS_ST(t) & PTHREADS_ST_WAITING)==PTHREADS_ST_WAITING) /* }}} */

/* {{{ set the joined state on the referenced PTHREAD */
#define PTHREADS_SET_JOINED(t) pthreads_set_state(t, PTHREADS_ST_JOINED) /* }}} */

/* {{{ set the running state on the referenced PTHREAD */
#define PTHREADS_SET_RUNNING(t)	pthreads_set_state(t, PTHREADS_ST_RUNNING) /* }}} */

/* {{{ unset the running state on the referenced PTHREAD */
#define PTHREADS_UNSET_RUNNING(t) pthreads_unset_state(t, PTHREADS_ST_RUNNING) /* }}} */

/* {{{ set the joined state on the referenced PTHREAD */
#define PTHREADS_IS_JOINED(t) ((PTHREADS_ST(t) & PTHREADS_ST_JOINED)==PTHREADS_ST_JOINED) /* }}} */

/* {{{ set the started state on the referenced PTHREAD */
#define PTHREADS_IS_STARTED(t) ((PTHREADS_ST(t) & PTHREADS_ST_STARTED)==PTHREADS_ST_STARTED) /* }}} */

/* {{{ tell if the referenced PTHREAD is in the running state */
#define PTHREADS_IS_RUNNING(t) ((PTHREADS_ST(t) & PTHREADS_ST_RUNNING)==PTHREADS_ST_RUNNING) /* }}} */

/* {{{ tell if the referenced PTHREAD has been started */
#define PTHREADS_SET_STARTED(t) pthreads_set_state(t, PTHREADS_ST_STARTED) /* }}} */

/* {{{ wait on the referenced PTHREAD monitor to be notified */
#define PTHREADS_WAIT(t) pthreads_set_state(t, PTHREADS_ST_WAITING) /* }}} */

/* {{{ wait on the referenced PTHREAD monitor for a specific amount of time */
#define PTHREADS_WAIT_EX(t, l) pthreads_set_state_ex(t, PTHREADS_ST_WAITING, l) /* }}} */

/* {{{ notify the referenced PTHREAD monitor */
#define PTHREADS_NOTIFY(t) pthreads_unset_state(t, PTHREADS_ST_WAITING) /* }}} */

/* {{{ lock a threads state */
#define PTHREADS_LOCK(t) pthread_mutex_lock(t->lock) /* }}} */

/* {{{ unlock a threads state */
#define PTHREADS_UNLOCK(t) pthread_mutex_unlock(t->lock) /* }}} */

/* {{{ setup reference to self using from as the threading version of $this */
#define PTHREADS_SET_SELF(from) do{\
	self = PTHREADS_FETCH_FROM(from);\
	self->self = 1;\
	self->tid = thread->tid;\
	self->cid = thread->cid;\
	self->state = PTHREADS_ST_STARTED|PTHREADS_ST_RUNNING;\
	self->sig = thread;\
	thread->sig = self;\
}while(0) /* }}} */

/* {{{ check for state, assumes you have the mutex for thread */
#define PTHREADS_IN_STATE(t, s) PTHREADS_ST_CHECK(t->state, s) /* }}} */

/* {{{ import a thread from another context */
#define PTHREADS_IMPORT pthreads_import /* }}} */

/* {{{ pop the next item from the work buffer onto the current stack */
#define PTHREADS_POP pthreads_pop /* }}} */

/* {{{ pop a specific item or all items from the stack and discard */
#define PTHREADS_POP_EX pthreads_pop_ex /* }}} */

/* {{{ push an item onto the work stack for a thread */
#define PTHREADS_PUSH pthreads_push /* }}} */

/* {{{ get the number of items on the stack */
#define PTHREADS_GET_STACKED pthreads_get_stacked /* }}} */

/*{{{ inline statics declarations */

/* {{{ set worker flag on PTHREAD */
static inline int pthreads_set_worker(PTHREAD thread, zend_bool flag) {
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
static inline int pthreads_is_worker(PTHREAD thread) {
	int result;
	int acquire = pthread_mutex_lock(thread->lock);
	
	if (acquire == SUCCESS || acquire == EDEADLK) {
		result = thread->worker;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else result = -1;
	return result;
} /* }}} */

/* {{{ get thread state mask */
static inline int pthreads_get_state(PTHREAD thread){
	int result = -1;
	int acquire = pthread_mutex_lock(thread->lock);
	if (acquire==SUCCESS || acquire == EDEADLK) {
		result = thread->state;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	}
	return result;
} /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
static inline int pthreads_set_state_ex(PTHREAD thread, int state, long timeout){
	int result = -1;
	struct timeval now;
	struct timespec until;
	int acquire;
	int wacquire;
	int timed = 0;
	
	if (timeout>0L) {
		if (pthreads_gettimeofday(&now, NULL)==SUCCESS) {
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
	
	if (PTHREADS_ST_CHECK(state, PTHREADS_ST_WAITING)) {
		wacquire = pthread_mutex_lock(thread->wait);
	}
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->state |= state;
		
		switch(state) {
			case PTHREADS_ST_WAITING: do {
				if (timed) {
						result=!pthread_cond_timedwait(thread->sync, thread->lock, &until);
				} else pthread_cond_wait(thread->sync, thread->lock);
			} while(PTHREADS_IN_STATE(thread, PTHREADS_ST_WAITING));
			break;
			
			case PTHREADS_ST_RUNNING: PTHREADS_G_ADD(thread); break;
		}
		
		if (PTHREADS_ST_CHECK(state, PTHREADS_ST_WAITING)) {
			if (result == -1)
				result = thread->state;
		}
		
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (PTHREADS_ST_CHECK(state, PTHREADS_ST_WAITING)) {
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->wait);
	}
	
	return result;
} /* }}} */

/* {{{ set state bits on a thread */
static inline int pthreads_set_state(PTHREAD thread, int state) {
	return pthreads_set_state_ex(thread, state, 0L);
} /* }}} */

/* {{{ unset state bits on a thread */
static inline int pthreads_unset_state(PTHREAD thread, int state){
	int result = -1;
	int acquire = 0;
	
check:
	switch (state) {
		case PTHREADS_ST_WAITING:
			if (!PTHREADS_IS_BLOCKING(thread)) {
#if _WIN32
				Sleep(10);
#else
				usleep(1000);
#endif
				goto check;
			}
		break;
	}
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->state &= ~state;
		switch (state) {
			case PTHREADS_ST_WAITING: pthread_cond_broadcast(thread->sync); break;
			case PTHREADS_ST_RUNNING: PTHREADS_G_DEL(thread); break;
		}
		result = thread->state;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	return ((result & state) != state);
} /* }}} */

/* {{{ pop for php */
static inline int pthreads_pop_ex(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	int acquire = 0;
	int remain = 0;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		if (work) {
			zend_llist_del_element(&thread->stack, &work, (int (*)(void *, void *)) pthreads_equal_func);
		} else zend_llist_destroy(&thread->stack);
		remain = thread->stack.count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	return remain;
} /* }}} */

/* {{{ pop the next item from the work buffer */
static inline int pthreads_pop(PTHREAD thread, zval *this_ptr TSRMLS_DC) {
	PTHREAD *work = NULL;
	PTHREAD current = NULL;
	int acquire = 0;
	int remain = -1;
	int bubble = 0;
	zend_function *run;
burst:
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		bubble = thread->worker;
		if (thread->stack.count > 0) {
			work = zend_llist_get_first(&thread->stack);
			if (work && (current = *work) && current) {
				bubble = 0;
				
				/*
				* Cleanup List
				*/
				zend_llist_del_element(&thread->stack, work, (int (*)(void *, void *)) pthreads_equal_func);
				/*
				* Setup the executor again
				*/
				if (zend_hash_find(&current->std.ce->function_table, "run", sizeof("run"), (void**) &run)==SUCCESS) {
#if PHP_VERSION_ID > 50399
					/*
					* Versions above 5.4 have run_time_cache to contend with, handled in compat for now
					*/
					current->std.ce->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif
					/*
					* Would usually be added when we create the class entry for a thread
					*/
					function_add_ref(run);
					
					/*
					* Set next op array
					*/
					PTHREADS_EG(thread->ls, active_op_array)=(zend_op_array*)run;
					
					/*
					* Return what is left on stack
					*/
					remain = thread->stack.count;
				}
			}
		}
		
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	if (bubble && remain < 0 && !PTHREADS_IS_JOINED(thread)) {
		if (PTHREADS_WAIT(thread) > 0) {
			remain = -1;
			goto burst;
		}
	}
	
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
static inline int pthreads_push(PTHREAD thread, PTHREAD work) {
	int acquire = 0;
	int counted = -1;
	int notify = 0;
	zend_function *run;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		thread->worker = 1;
		zend_llist_add_element(&thread->stack, &work);
		counted = thread->stack.count;
		notify = PTHREADS_IN_STATE(thread, PTHREADS_ST_WAITING);
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
		if (counted && notify) {
			PTHREADS_NOTIFY(thread);
		}
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue: %d", acquire);
	
	return counted;
} /* }}} */

/* {{{ return the number of items currently stacked */
static inline int pthreads_get_stacked(PTHREAD thread) {
	int acquire = 0;
	int counted = -1;
	
	acquire = pthread_mutex_lock(thread->lock);
	if (acquire == SUCCESS || acquire == EDEADLK) {
		counted = thread->stack.count;
		if (acquire != EDEADLK)
			pthread_mutex_unlock(thread->lock);
	} else zend_error(E_ERROR, "pthreads has suffered an internal error and cannot continue");
	return counted;
} /* }}} */

/* {{{ import a thread created in another context into the current context */
static inline int pthreads_import(PTHREAD thread, zval** return_value TSRMLS_DC){
	if (!PTHREADS_IS_CREATOR(thread)) {
		/*
		* Initialize the thread in the current context
		*/
		if (object_init_ex(*return_value, pthreads_import_entry)==SUCCESS) {
			PTHREAD imported = PTHREADS_FETCH_FROM(*return_value);
			if (imported) {
				/*
				* Block all changes to state while we setup
				*/
				PTHREADS_LOCK(thread);
				
				/*
				* The Thread identifier of the imported thread
				*/
				imported->tid = thread->tid;
				
				/*
				* Set creator to the importing thread
				*/
				imported->cid = pthreads_self();
				
				/*
				* These objects are treated differently so must set import flag
				*/
				imported->import = 1;
				
				/*
				* These objects do not require a sync or lock or state of their own
				*/
				imported->wait = NULL;
				imported->sync = NULL;
				imported->lock = NULL;
				imported->state = 0L;
				
				/*
				* Set the imports significant other
				*/
				imported->sig = thread;
				
				/*
				* Unlock the thread we've imported
				*/
				PTHREADS_UNLOCK(thread);
				
				return 1;
			} else zend_error_noreturn(E_WARNING, "pthreads has detected a failure while importing Thread %lu from Thread %lu", thread->tid, thread->cid);
		} else zend_error_noreturn(E_WARNING, "pthreads has detected a failure while trying to initialize imported Thread %lu", thread->tid);
	} else zend_error_noreturn(E_WARNING, "pthreads has detected an attempt to import a thread that was created in the current context");
	return 0;
} /* }}} */

/* }}} */

/* {{{ this constructor is used to build the original PTHREAD in the creating context */
zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	
	zend_object_value attach;
	zval *temp;
	
	/*
	* Allocate an initialize thread object for storage
	*/
	PTHREAD thread = calloc(1, sizeof(*thread));			
	thread->synchronized = 0;
	thread->self = 0;
	thread->ls	= tsrm_ls;
	thread->sig = NULL;
	thread->tid = 0L;
	thread->cid = pthreads_self();
	
	/*
	* State Initialization
	*/
	thread->lock = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
	if (thread->lock) {
		pthread_mutex_init(thread->lock, &defmutex);
		thread->state = 0;
		thread->wait = (pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t));
		if (thread->wait) {
			pthread_mutex_init(thread->wait, &defmutex);
		}
	}
	
	/*
	* Sync Initialization
	*/
	thread->sync = (pthread_cond_t*) calloc(1, sizeof(pthread_cond_t));
	if (thread->sync)
		pthread_cond_init(thread->sync, NULL);
	
	/*
	* Initialize Work buffer
	*/
	zend_llist_init(&thread->stack, sizeof(void**), NULL, 1);
	
	/*
	* Initialize standard entry
	*/ 
	zend_object_std_init(&thread->std, entry TSRMLS_CC);
	
	/*
	* Initialize instance properties
	*/
#if PHP_VERSION_ID < 50399
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

/* {{{ this constructor is used to build a PTHREAD skeleton for use as a base object to represent an imported thread */
zend_object_value pthreads_attach_to_import(zend_class_entry *entry TSRMLS_DC){
	
	zend_object_value attach;
	zval *temp;
	 
	/*
	* Allocate an initialize thread object for storage
	*/
	PTHREAD import = calloc(1, sizeof(*import));			
	import->synchronized = 0;
	import->self = 0;
	import->ls	= tsrm_ls;
	import->sig = NULL;
	import->tid = 0L;
	import->cid = pthreads_self();
	import->lock = NULL;
	import->sync = NULL;
	import->state = 0;
	
	/*
	* Standard Entry Initialization
	*/ 
	zend_object_std_init(&import->std, entry TSRMLS_CC);
	
	/*
	* Initialize instance properties
	*/
#if PHP_VERSION_ID < 50399
	 zend_hash_copy(															
		import->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
#else
	object_properties_init(&(import->std), entry);
#endif

	/*
	* Store the object
	*/
	attach.handle = zend_objects_store_put(
		import,  
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

/* {{{ this destructor handles all types of PTHREAD gracefully */
void pthreads_detach_from_instance(void * arg TSRMLS_DC){
	/*
	* We must check for a result even when it's ignored
	*/
	char *result;
	PTHREAD thread = (PTHREAD) arg;
	
	if (thread) {
		if (!PTHREADS_IS_SELF(thread) && !PTHREADS_IS_IMPORT(thread)) {
			
			/*
			* If the thread is running we must attempt to join
			*/
			if (PTHREADS_IS_STARTED(thread)) {
				if (!PTHREADS_IS_JOINED(thread)) {
					
					/*
					* Here we force blocking threads to wake and warn users of the deadlock
					*/
					if (PTHREADS_IS_BLOCKING(thread)) {
#ifndef _WIN32
						zend_error(E_WARNING, "pthreads has avoided a deadlock, thread #%lu was waiting for notification", (ulong) pthread_self());
#else	
						zend_error(E_WARNING, "pthreads has avoided a deadlock, thread #%lu was waiting for notification", (unsigned long) GetCurrentThreadId());
#endif
						PTHREADS_NOTIFY(thread);
					}
					
					pthread_join(thread->thread, (void*) &result);
					PTHREADS_SET_JOINED(thread);
					
					/*
					* Free the result for the thread
					*/
					if (result)
						free(result);
				}
			}
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

		if (!PTHREADS_IS_IMPORT(thread)) {
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
			zend_llist_destroy(&thread->stack);
		}

		/*
		* Finally free the thread object
		*/
		free(thread);
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
			* Set Context/TSRMLS
			*/
			tsrm_set_interpreter_context(ctx); {
				TSRMLS_FETCH_FROM_CTX(ctx);
			
				/*
				* Set Thread Identifier
				*/
				thread->tid = pthreads_self();

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
					ce.create_object = pthreads_attach_to_instance;
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
						ALLOC_INIT_ZVAL(discard);
						EG(return_value_ptr_ptr)=&discard;				
						EG(active_op_array)=(zend_op_array*)prepare;
						EG(active_op_array)->filename = rename;
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
						do {
							if (EG(active_op_array)) {
								zend_execute(EG(active_op_array) TSRMLS_CC);
							}
						} while(pthreads_pop(thread, getThis() TSRMLS_CC)>-1);
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
			}
			
			/*
			* Free Context
			*/
			tsrm_free_interpreter_context(ctx);	
		}
	}

	PTHREADS_UNSET_RUNNING(thread);
	
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

#endif /* HAVE_PTHREADS_OBJECT_H */
