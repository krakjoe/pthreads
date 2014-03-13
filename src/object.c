/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

/* {{{ pthreads module entry */
extern zend_module_entry pthreads_module_entry; /* }}} */

static inline pthreads_address pthreads_address_alloc(PTHREAD object TSRMLS_DC) {
	pthreads_address address = (pthreads_address) calloc(1, sizeof(*address));
	if (address) {
	    pid_t pid = PTHREADS_PID();
	    
		/* add the space for null here, once */
		address->length = snprintf(
			NULL, 0, "%lu:%lu", (long)pid, (long) object
		);
		if (address->length) {
			address->serial = calloc(1, address->length+1);
			if (address->serial) {
				sprintf(
					(char*) address->serial, "%lu:%lu", (long)pid, (long) object
				);
			}
		}
	}
	return address;
}

static inline void pthreads_address_free(pthreads_address address) {
	if (address->serial) {
		free(address->serial);
	}
	free(address);
}

/* {{{ base ctor/clone/dtor/free */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry TSRMLS_DC);
static void pthreads_base_dtor(void *arg, zend_object_handle handle TSRMLS_DC); 
static void pthreads_base_free(void *arg TSRMLS_DC);
static void pthreads_base_clone(void *arg, void **pclone TSRMLS_DC); /* }}} */

/* {{{ connect objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination TSRMLS_DC); /* }}} */

/* {{{ pthreads routine */
static void * pthreads_routine(void *arg); /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
zend_bool pthreads_set_state_ex(PTHREAD thread, int mask, long timeout TSRMLS_DC) {
	zend_bool locked, dowait, result;
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_lock(thread->state, &locked TSRMLS_CC)) {
			dowait = !pthreads_state_check(thread->state, PTHREADS_ST_JOINED TSRMLS_CC);
			if (dowait)
				pthreads_state_set_locked(thread->state, PTHREADS_ST_WAITING TSRMLS_CC);
			if (locked)
				pthreads_state_unlock(thread->state, locked TSRMLS_CC);
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
zend_bool pthreads_set_state(PTHREAD thread, int mask TSRMLS_DC) {
	return pthreads_set_state_ex(thread, mask, 0L TSRMLS_CC);
} /* }}} */

/* {{{ unset state bits on a thread */
zend_bool pthreads_unset_state(PTHREAD thread, int mask TSRMLS_DC){
	zend_bool result = 0;
	
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_unset(thread->state, mask TSRMLS_CC)) {
			result = pthreads_synchro_notify(thread->synchro TSRMLS_CC);
		} else result = 0;
	} else result = pthreads_state_unset(thread->state, mask TSRMLS_CC);
	
	return result;
} /* }}} */

/* {{{ save the current error for object */
static inline void pthreads_error_save(pthreads_error error TSRMLS_DC) {
    if (EG(active_op_array)) {
        zend_function *active = (zend_function*) EG(active_op_array);
        
        /* deal with scope stuff */
        if (active) {
            if (active->common.scope) {
                error->clazz = (unsigned char *)strdup(
                    active->common.scope->name);
            } else error->clazz = NULL;
            
            if (active->common.function_name) {
                error->method = (unsigned char *)strdup(
                    active->common.function_name);
            } else error->method = NULL;
        }
        
        /* deal with file stuff */
        {
            const char *tmp;
            
            tmp = zend_get_executed_filename(TSRMLS_C);
            if (tmp)
                error->file = (unsigned char *)strdup(tmp);
            
           error->line = zend_get_executed_lineno(TSRMLS_C);
        }
    }
} /* }}} */

/* {{{ allocate error structure */
static inline pthreads_error pthreads_error_alloc(TSRMLS_D) {
    pthreads_error error = calloc(1, sizeof(*error));
    
    error->clazz = NULL;
    error->method = NULL;
    error->file = NULL;
    
    return error;
} /* }}} */

/* {{{ free the error saved for object */
static inline void pthreads_error_free(pthreads_error error TSRMLS_DC) {
    if (error) {
        if (error->clazz)
            free(error->clazz);
            
        if (error->method)
            free(error->method);
            
        if (error->file)
            free(error->file);
            
        free(
            error);
        
        error = NULL;
    }
} /* }}} */

/* {{{ pop for php */
size_t pthreads_stack_pop(PTHREAD thread, PTHREAD work TSRMLS_DC) {
	zend_bool locked;
	int remain = 0;
	
	if (pthreads_lock_acquire(thread->lock, &locked TSRMLS_CC)) {
		if (PTHREADS_IS_WORKER(thread)) {
			HashTable *stack = &thread->stack->objects;
			if (work) {
				HashPosition position;
				PTHREAD search = NULL;
				for (zend_hash_internal_pointer_reset_ex(stack, &position);
				    zend_hash_get_current_data_ex(stack, (void**)&search, &position) == SUCCESS;
				    zend_hash_move_forward_ex(stack, &position)) {
				    /* arghhh */
				}
			} else zend_hash_destroy(stack);
			remain = zend_hash_num_elements(stack);
		}
		pthreads_lock_release(thread->lock, locked TSRMLS_CC);
	} else remain = -1;
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
size_t pthreads_stack_push(PTHREAD thread, zval *work TSRMLS_DC) {
	zend_bool locked;
	PTHREAD threaded = PTHREADS_FETCH_FROM(work);
	size_t counted = 0L;
	
	if (pthreads_lock_acquire(thread->lock, &locked TSRMLS_CC)) {
		HashTable *stack = &thread->stack->objects;
		if (stack) {
	        if (!zend_hash_num_elements(stack)) {
	            zend_hash_clean(
	                stack);
	            thread->stack->position = 0L;
	        }
	        
		    zend_hash_next_index_insert(
		        stack, (void**) &threaded, sizeof(struct _pthread_construct), NULL
		    );
			counted = zend_hash_num_elements(stack);
		}
		pthreads_lock_release(thread->lock, locked TSRMLS_CC);
		
		if (counted > 0L) {
		    pthreads_synchro_lock(thread->synchro TSRMLS_CC);
			pthreads_unset_state(
			    thread, PTHREADS_ST_WAITING TSRMLS_CC);
			pthreads_synchro_unlock(thread->synchro TSRMLS_CC);
		}
	}
	
	return counted;
} /* }}} */

/* {{{ pop the next item from the work buffer */
size_t pthreads_stack_next(PTHREAD thread, zval *this_ptr TSRMLS_DC) {
	PTHREAD *work, current = NULL;
	zend_bool locked;
	size_t bubble = 0;
	zval *that_ptr;
	zend_class_entry *popped;
	
burst:
	if (pthreads_lock_acquire(thread->lock, &locked TSRMLS_CC)) {
		if ((bubble=zend_hash_num_elements((HashTable*) thread->stack)) > 0) {
			zend_hash_index_find(
			    (HashTable*) thread->stack,
			     thread->stack->position, (void**) &work
			);
			if ((current = *work)) {
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
				popped = Z_OBJCE_P(that_ptr);

				/*
				* Setup Executor
				*/
				EG(This) = that_ptr;
				EG(scope) = popped;
				EG(called_scope) = popped;
				
				/*
				* Setup threaded object for runtime
				*/
				{
					PTHREAD threaded = PTHREADS_FETCH_FROM(that_ptr);
					
					if (threaded) {
						current->tid = thread->tid;
						current->tls = thread->tls;
						
						pthreads_connect(current, threaded TSRMLS_CC);

						pthreads_store_write(
							threaded->store, "worker", sizeof("worker")-1, &this_ptr TSRMLS_CC
						);

						Z_ADDREF_P(this_ptr);
					}
				}
			}
			
			zend_hash_index_del(
			    (HashTable*) thread->stack, thread->stack->position++);

		}
		pthreads_lock_release(thread->lock, locked TSRMLS_CC);
		
		if (!bubble) {
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			    pthreads_synchro_lock(thread->synchro TSRMLS_CC);
				if (pthreads_set_state(thread, PTHREADS_ST_WAITING TSRMLS_CC)) {
				    pthreads_synchro_unlock(thread->synchro TSRMLS_CC);
					goto burst;
				} else pthreads_synchro_unlock(thread->synchro TSRMLS_CC);
			} else return 0;
		}
	}
	
	return bubble;
} /* }}} */

/* {{{ return the number of items currently stacked */
size_t pthreads_stack_length(PTHREAD thread TSRMLS_DC) {
	zend_bool locked;
	size_t counted = 0;
	if (pthreads_lock_acquire(thread->lock, &locked TSRMLS_CC)) {
		counted = zend_hash_num_elements(
		    &thread->stack->objects);
		pthreads_lock_release(thread->lock, locked TSRMLS_CC);
	}
	return counted;
} /* }}} */

/* {{{ thread object constructor */
zend_object_value pthreads_thread_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD thread = pthreads_globals_object_alloc(sizeof(*thread) TSRMLS_CC);
	if (thread) {
		thread->scope = PTHREADS_SCOPE_THREAD;
		pthreads_base_ctor(thread, entry TSRMLS_CC);
		thread->handle = zend_objects_store_put(
			thread,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		attach.handle   = thread->handle;
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ worker object constructor */
zend_object_value pthreads_worker_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD worker = pthreads_globals_object_alloc(sizeof(*worker) TSRMLS_CC);
	if (worker) {
		worker->scope = PTHREADS_SCOPE_WORKER;
		pthreads_base_ctor(worker, entry TSRMLS_CC);
		worker->handle = zend_objects_store_put(
			worker,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		attach.handle = worker->handle;
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ threaded object constructor */
zend_object_value pthreads_threaded_ctor(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value attach;
	PTHREAD threaded = pthreads_globals_object_alloc(sizeof(*threaded) TSRMLS_CC);
	if (threaded) {
		threaded->scope = PTHREADS_SCOPE_THREADED;
		pthreads_base_ctor(threaded, entry TSRMLS_CC);
		threaded->handle = zend_objects_store_put(
			threaded,
			(zend_objects_store_dtor_t) pthreads_base_dtor,
			(zend_objects_free_object_storage_t) pthreads_base_free,
			(zend_objects_store_clone_t) pthreads_base_clone TSRMLS_CC
		);
		
		attach.handle   = threaded->handle;
		attach.handlers = &pthreads_handlers;
	}
	return attach;
} /* }}} */

/* {{{ */
void pthreads_current_thread(zval **return_value TSRMLS_DC) {
	if (PTHREADS_ZG(pointer)) {
		PTHREAD current = (PTHREAD) PTHREADS_ZG(pointer);
		object_init_ex(
			(*return_value), pthreads_prepared_entry(NULL, current->std.ce TSRMLS_CC));
		pthreads_connect(current, PTHREADS_FETCH_FROM((*return_value)) TSRMLS_CC);
	}
} /* }}} */

/* {{{ connect pthread objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination TSRMLS_DC) {
	if (source && destination) {

		if (PTHREADS_IS_NOT_CONNECTION(destination)) {
			pthreads_lock_free(destination->lock TSRMLS_CC);
			pthreads_state_free(destination->state  TSRMLS_CC);
			pthreads_modifiers_free(destination->modifiers TSRMLS_CC);
			pthreads_store_free(destination->store TSRMLS_CC);
			pthreads_synchro_free(destination->synchro TSRMLS_CC);
			pthreads_resources_free(destination->resources TSRMLS_CC);
			pthreads_address_free(destination->address);
            pthreads_error_free(destination->error TSRMLS_CC);
            
			if (PTHREADS_IS_WORKER(destination)) {
				zend_hash_destroy(
				    &destination->stack->objects
				);
				free(destination->stack);
			}
			
			destination->scope |= PTHREADS_SCOPE_CONNECTION;
			
			return pthreads_connect
			(
				source,
				destination TSRMLS_CC
			);
		}
		
		destination->thread = source->thread;
		destination->tid = source->tid;
		destination->tls = source->tls;
		destination->address = source->address;
		destination->resources = source->resources;
		destination->lock = source->lock;
		destination->state = source->state;
		destination->synchro = source->synchro;
		destination->modifiers = source->modifiers;
		destination->store = source->store;
		destination->stack = source->stack;
		destination->error = source->error;
		
		if (PTHREADS_IS_DETACHED(source)) {
		    destination->scope |= PTHREADS_SCOPE_DETACHED;
		}
		
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

		base->cls = tsrm_ls;
		base->cid = pthreads_self();
		base->address = pthreads_address_alloc(base TSRMLS_CC);
		base->options = PTHREADS_INHERIT_ALL;

		if (!PTHREADS_IS_CONNECTION(base)) {
			base->lock = pthreads_lock_alloc(TSRMLS_C);
			base->state = pthreads_state_alloc(0 TSRMLS_CC);
			base->synchro = pthreads_synchro_alloc(TSRMLS_C);
			base->modifiers = pthreads_modifiers_alloc(TSRMLS_C);
			base->store = pthreads_store_alloc(TSRMLS_C);
			base->resources = pthreads_resources_alloc(TSRMLS_C);
            base->error = pthreads_error_alloc(TSRMLS_C);
			
			pthreads_modifiers_init(base->modifiers, entry TSRMLS_CC);
			if (PTHREADS_IS_WORKER(base)) {
				base->stack = (pthreads_stack) calloc(1, sizeof(*base->stack));
				if (base->stack) {
				    zend_hash_init(
				        &base->stack->objects, 8, NULL, NULL, 1);
                    base->stack->position = 0L;
				}	
			}
		}
	}
} /* }}} */

/* {{{ pthreads base destructor */
static void pthreads_base_dtor(void *arg, zend_object_handle handle TSRMLS_DC) {
	PTHREAD base = (PTHREAD) arg;

	if (PTHREADS_IS_NOT_CONNECTION(base) && PTHREADS_IS_NOT_DETACHED(base)) {
	     
	     assert(base->cls == TSRMLS_C);
	     
	     if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
	        pthread_t *pthread = &base->thread;
	        if (pthread) {
		        pthreads_join(base TSRMLS_CC);
	        }
        }

	    pthreads_lock_free(base->lock TSRMLS_CC);
	    pthreads_state_free(base->state  TSRMLS_CC);
	    pthreads_modifiers_free(base->modifiers TSRMLS_CC);
	    pthreads_store_free(base->store TSRMLS_CC);
	    pthreads_synchro_free(base->synchro TSRMLS_CC);
	    pthreads_resources_free(base->resources TSRMLS_CC);
	    pthreads_address_free(base->address);
        pthreads_error_free(base->error TSRMLS_CC);
        
	    if (PTHREADS_IS_WORKER(base)) {
		    zend_hash_destroy(
			    &base->stack->objects
		    );
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
} /* }}} */

/* {{{ free object */
static void pthreads_base_free(void *arg TSRMLS_DC) {
	PTHREAD base = (PTHREAD) arg;
	if (base) {
	    if (PTHREADS_IS_NOT_DETACHED(base)) {
	    	if (pthreads_globals_object_delete(base TSRMLS_CC)) {
	    		base = NULL;
	    	}
	    }
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
	zend_bool tlocked, slocked;
	if (pthreads_state_lock(thread->state, &slocked TSRMLS_CC)) {
		if (!pthreads_state_check(thread->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_STARTED TSRMLS_CC);
			dostart = 1;
		} else started = PTHREADS_ST_STARTED;
		if (slocked)
			pthreads_state_unlock(thread->state, slocked TSRMLS_CC);
		
	}
	if (dostart) {
		if (pthreads_lock_acquire(thread->lock, &tlocked TSRMLS_CC)) {
			started = pthread_create(&thread->thread, NULL, pthreads_routine, (void*)thread);
			if (started == SUCCESS) 
				pthreads_state_wait(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC);
			pthreads_lock_release(thread->lock, tlocked TSRMLS_CC);
		}
	}
	
	return started;
} /* }}} */

/* {{{ gracefully join a pthread object */
int pthreads_join(PTHREAD thread TSRMLS_DC) {
	int dojoin = 0;
	int donotify = 0;
	zend_bool slocked;
	
	if (pthreads_state_lock(thread->state, &slocked TSRMLS_CC)) {
		if (pthreads_state_check(thread->state, PTHREADS_ST_STARTED TSRMLS_CC) && 
			!pthreads_state_check(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_JOINED TSRMLS_CC);
			if (PTHREADS_IS_WORKER(thread))
				donotify = pthreads_state_check(thread->state, PTHREADS_ST_WAITING TSRMLS_CC);
			dojoin = 1;
		}
		if (slocked)
			pthreads_state_unlock(thread->state, slocked TSRMLS_CC);
	}
	
	if (donotify) do {
		pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
	} while(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	
	return dojoin ? pthread_join(thread->thread, NULL) 	: FAILURE;
} /* }}} */

/* {{{ detach a thread */
int pthreads_detach(PTHREAD thread TSRMLS_DC) {

    if (PTHREADS_IS_NOT_DETACHED(thread)) {
        if ((pthread_detach(thread->thread) == SUCCESS)) {
            thread->scope |= PTHREADS_SCOPE_DETACHED;
            
            return SUCCESS;
        }
    }
    
    return FAILURE;
} /* }}} */

/* {{{ synchronization helper */
zend_bool pthreads_wait_member_ex(PTHREAD thread, zval *member, ulong timeout TSRMLS_DC) {
	if (!pthreads_store_isset(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), 2 TSRMLS_CC)) {
		if (pthreads_synchro_wait_ex(thread->synchro, timeout TSRMLS_CC))
			return pthreads_store_isset(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), 2 TSRMLS_CC);
		else return 0;
	} else return 1;
} /* }}} */

/* {{{ synchronization helper */
zend_bool pthreads_wait_member(PTHREAD thread, zval *member TSRMLS_DC) {
	return pthreads_wait_member_ex(thread, member, 0L TSRMLS_CC);
} /* }}} */

/* {{{ serialize an instance of a threaded object for connection in another thread */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, zend_uint *blength, zend_serialize_data *data TSRMLS_DC) {
	PTHREAD threaded = PTHREADS_FETCH_FROM(object);
	if (threaded) {
		(*buffer) = (unsigned char*) estrndup(
			(char*)threaded->address->serial, 
			threaded->address->length
		);
		(*blength) = threaded->address->length+1;
		
		return SUCCESS;
	}
	return FAILURE;
} /* }}} */

/* {{{ connects to an instance of a threaded object */
int pthreads_internal_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buffer, zend_uint blength, zend_unserialize_data *data TSRMLS_DC) {
	PTHREAD address = NULL;
	pid_t pid = 0L;
	zend_ulong len = sscanf(
		(const char*)buffer, 
		"%lu:%lu", 
		(long unsigned int *)&pid, (long unsigned int *)&address);
	
    if (len) {
        pid_t mpid = PTHREADS_PID();
        
        if (address && pthreads_globals_object_validate((zend_ulong)address TSRMLS_CC)) {
            if (pid == mpid) {

            	/* if we already own this object do not create another handle */
            	if (address->cls == TSRMLS_C) {
            		Z_TYPE_PP(object) = IS_OBJECT;
            		Z_OBJ_HANDLE_PP(object) = address->handle;
            		Z_OBJ_HT_PP(object) = &pthreads_handlers;
            		Z_OBJ_HT_PP(object)->add_ref(*object TSRMLS_CC);
            		
            		return SUCCESS;
            	}
            	
            	/* else initialize and connect to the original object */
            	if (object_init_ex(*object, ce) == SUCCESS) {
					pthreads_connect(
		                	address, 
		                	PTHREADS_FETCH_FROM(*object) TSRMLS_CC);

		            return SUCCESS;
				}
            } else {
            	zend_throw_exception_ex(
					spl_ce_RuntimeException, 0 TSRMLS_CC,
					"pthreads detected an attempt to connect to a %s "
					"which belongs to another process", ce->name);
            }
        } else {
        	zend_throw_exception_ex(
				spl_ce_RuntimeException, 0 TSRMLS_CC,
				"pthreads detected an attempt to connect to a %s "
				"which has already been destroyed", ce->name);
        }
    } else {
    	zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC,
			"pthreads detected an attempt to connect to a %s "
			"which has already is corrupted", ce->name);
    }
	
	ZVAL_NULL(*object);
	
	return FAILURE;
} /* }}} */

#ifdef PTHREADS_KILL_SIGNAL
static inline void pthreads_kill_handler(int signo) /* {{{ */
{	
	TSRMLS_FETCH();
	PTHREADS_ZG(signal) = signo;
	zend_bailout();
} /* }}} */
#endif

/* {{{ this is aptly named ... */
static void * pthreads_routine(void *arg) {
	/* passed the object as argument */
	PTHREAD thread = (PTHREAD) arg;
	char nothing[] = {0,};
	
#ifdef PTHREADS_KILL_SIGNAL
	/* installed to support a graceful-ish kill function */
	signal(
		PTHREADS_KILL_SIGNAL, pthreads_kill_handler);
#endif

	if (thread) {
#ifdef PTHREADS_PEDANTIC
		zend_bool  glocked = 0; /* global lock indicator */
#endif
        zend_bool  worker = 0,  /* worker indicator */
                   inwork = 0;  /* working indicator */
		
		/* $this original pointer */
		zval *this_ptr = NULL, 
			 *this = NULL;		
		
		/* executor globals */
		zend_executor_globals *ZEG = NULL;

		/**
		* Startup Block Begin
		**/
		TSRMLS_FETCH();
		
		/* set thread local storage */
		thread->tls = TSRMLS_C;
		
#ifdef PTHREADS_PEDANTIC
		/* acquire a global lock */
		pthreads_globals_lock(&glocked TSRMLS_CC);
#endif

		/* set thread id for this object */
		thread->tid = pthreads_self();
		
		/* set context the same as parent */
		SG(server_context) = PTHREADS_SG(thread->cls, server_context);
		
		/* some php globals */
		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;

#ifdef HAVE_PHP_SESSION
		/* fixup sessions for compatibility */
		if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
			PS(cache_limiter) = nothing;
			PS(use_cookies) = 0;
		}
#endif

		/* fix php-fpm compatibility */
		SG(sapi_started)=0;		
		
		if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
			/* do not send headers again */
			SG(headers_sent)=1;
			SG(request_info).no_headers = 1;
		}
		
		/* request startup */
		php_request_startup(TSRMLS_C);
		
		/* prepare environment */
		pthreads_prepare(thread TSRMLS_CC);

#ifdef PTHREADS_PEDANTIC
		/* release global lock */
		pthreads_globals_unlock(glocked TSRMLS_CC);
#endif

		/**
		* Startup Block End
		**/
		ZEG = PTHREADS_EG_ALL(TSRMLS_C);
		
		/*
		* Allocate $this
		*/
		ALLOC_INIT_ZVAL(this);
		
		/*
		* Assign $this
		*/
		this_ptr = this;
		
		/**
		* Thread Block Begin
		**/
		zend_first_try {
			/*
			* Set worker indicator
			*/
			worker = PTHREADS_IS_WORKER(thread);

			/* EG setup */
			ZEG->in_execution = 1;							
			ZEG->current_execute_data=NULL;					
			ZEG->current_module=&pthreads_module_entry;
			
			/* init $this */
			object_init_ex
			(
				ZEG->This = getThis(), 
				ZEG->scope = pthreads_prepared_entry(
					thread, thread->std.ce
					 TSRMLS_CC
				)
			);
			
			/* connect $this */
			if (pthreads_connect(PTHREADS_ZG(pointer)=thread, PTHREADS_FETCH_FROM(ZEG->This) TSRMLS_CC)==SUCCESS) {
				/* execute $this */
				do {
					PTHREAD current = PTHREADS_FETCH_FROM(ZEG->This);
					zval *zresult = NULL;
					
					pthreads_state_set(current->state, PTHREADS_ST_RUNNING TSRMLS_CC);
					{
					    zend_bool terminated = 0;
						/* graceful fatalities */
						zend_try {
						    /* ::run */
							zend_call_method(
								&ZEG->This, ZEG->scope, NULL, 
								ZEND_STRL("run"), 
								&zresult, 0, NULL, NULL TSRMLS_CC);
						} zend_catch {
						    /* catches fatal errors and uncaught exceptions */
							terminated = 1;
							
							/* danger lurking ... */
							if (PTHREADS_ZG(signal) == PTHREADS_KILL_SIGNAL) {
								/* like, totally bail man ! */
								zend_bailout();
							}
						} zend_end_try();
						
						if (current) {
						    /* set terminated state */
						    if (terminated) {
						        pthreads_state_set(
							        current->state, PTHREADS_ST_ERROR TSRMLS_CC);
							    /* save error information */
							    pthreads_error_save(current->error TSRMLS_CC);
						    }
						    
						    /* unset running for waiters */
						    pthreads_state_unset(current->state, PTHREADS_ST_RUNNING TSRMLS_CC);
						}

						/* deal with references to stacked objects */
						if (!terminated && inwork) {
							zval_ptr_dtor(&ZEG->This);
						} else inwork = 1;

						/* deal with zresult (ignored) */
						if (zresult) {
							zval_ptr_dtor(&zresult);
						}
					}
				} while(worker && pthreads_stack_next(thread, this_ptr TSRMLS_CC));
			}
		} zend_catch {
			/* do something, it's all gone wrong */ 
		} zend_end_try();
		
		/**
		* Thread Block End
		**/
		
	    /*
		* Free original reference to $this
		*/
		if (!BG(user_shutdown_function_names)) {
			zval_ptr_dtor(&this);
		}
		
		/**
		* Shutdown Block Begin
		**/
		PG(report_memleaks) = 0;
		
#ifdef PTHREADS_PEDANTIC
		/* acquire global lock */
		pthreads_globals_lock(&glocked TSRMLS_CC);
#endif
		/* shutdown request */
	    php_request_shutdown(TSRMLS_C);
	    
#ifdef PTHREADS_PEDANTIC
		/* release global lock */
		pthreads_globals_unlock(glocked TSRMLS_CC);
#endif

		/**
		* Shutdown Block End
		**/
	}

	pthread_exit(NULL);
	
#ifdef _WIN32
	return NULL; /* silence MSVC compiler */
#endif
} 
/* }}} */

#endif

