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

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

/* {{{ pthreads module entry */
extern zend_module_entry pthreads_module_entry; /* }}} */

static inline pthreads_address pthreads_address_alloc(PTHREAD object) {
	pthreads_address address = (pthreads_address) calloc(1, sizeof(*address));
	if (address) {
	    pid_t pid = PTHREADS_PID();
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
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry);
static void pthreads_base_clone(void *arg, void **pclone); /* }}} */

/* {{{ connect objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination); /* }}} */

/* {{{ pthreads routine */
static void * pthreads_routine(void *arg); /* }}} */

/* {{{ set state bits on a thread, timeout where appropriate/required */
zend_bool pthreads_set_state_ex(PTHREAD thread, int mask, long timeout) {
	zend_bool locked, dowait, result;
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_lock(thread->state, &locked)) {
			dowait = !pthreads_state_check(thread->state, PTHREADS_ST_JOINED);
			if (dowait)
				pthreads_state_set_locked(thread->state, PTHREADS_ST_WAITING);
			if (locked)
				pthreads_state_unlock(thread->state, locked);
			if (dowait) {
				result = pthreads_synchro_wait_ex(
					thread->synchro, timeout
				);
			} else result = 0;
		} else result = 0;
	} else result = pthreads_state_set(thread->state, mask);
	return result;
} /* }}} */

/* {{{ set state bits on a thread */
zend_bool pthreads_set_state(PTHREAD thread, int mask) {
	return pthreads_set_state_ex(thread, mask, 0L);
} /* }}} */

/* {{{ unset state bits on a thread */
zend_bool pthreads_unset_state(PTHREAD thread, int mask){
	zend_bool result = 0;
	
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_unset(thread->state, mask)) {
			result = pthreads_synchro_notify(thread->synchro);
		} else result = 0;
	} else result = pthreads_state_unset(thread->state, mask);
	
	return result;
} /* }}} */

/* {{{ pop for php */
size_t pthreads_stack_pop(PTHREAD thread, PTHREAD work) {
	zend_bool locked;
	int remain = 0;
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		if (PTHREADS_IS_WORKER(thread)) {
			HashTable *stack = &thread->stack->objects;
			if (work) {
				HashPosition position;
				PTHREAD search = NULL;
				zval *bucket;
				for (zend_hash_internal_pointer_reset_ex(stack, &position);
				    (bucket = zend_hash_get_current_data_ex(stack, &position));
				    zend_hash_move_forward_ex(stack, &position)) {
				    /* arghhh */
				}
			} else zend_hash_destroy(stack);
			remain = zend_hash_num_elements(stack);
		}
		pthreads_lock_release(thread->lock, locked);
	} else remain = -1;
	return remain;
} /* }}} */

/* {{{ push an item onto the work buffer */
size_t pthreads_stack_push(PTHREAD thread, zval *work) {
	zend_bool locked;
	PTHREAD threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(work));
	size_t counted = 0L;
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		HashTable *stack = &thread->stack->objects;
		if (stack) {
			if (!zend_hash_num_elements(stack)) {
			    zend_hash_clean(stack);
			    thread->stack->position = 0L;
			}
			
			zend_hash_next_index_insert_ptr(stack, threaded);
			counted = zend_hash_num_elements(stack);
		}
		pthreads_lock_release(thread->lock, locked);
		
		if (counted > 0L) {
		    pthreads_synchro_lock(thread->synchro);
			pthreads_unset_state(
			    thread, PTHREADS_ST_WAITING);
			pthreads_synchro_unlock(thread->synchro);
		}
	}
	
	return counted;
} /* }}} */

/* {{{ pop the next item from the work buffer */
size_t pthreads_stack_next(zval *that) {
	PTHREAD thread,
		work;
	zend_bool locked;
	size_t bubble = 0;
	zend_class_entry *popped;
	TSRMLS_CACHE_UPDATE();
	
	thread = PTHREADS_FETCH_FROM(Z_OBJ(PTHREADS_ZG(this)));

burst:
	if (Z_TYPE_P(that) != IS_UNDEF) {
		if (Z_OBJ_P(that) != Z_OBJ(PTHREADS_ZG(this))) {
			zval_dtor(that);
		}
		ZVAL_UNDEF(that);
	}
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		if ((bubble=zend_hash_num_elements((HashTable*) thread->stack)) > 0) {
			if ((work = zend_hash_index_find_ptr((HashTable*) thread->stack, thread->stack->position))) {
				PTHREAD threaded;
	
				/*
				* Initialize it with the new entry
				*/
				object_init_ex(
					that,
					pthreads_prepared_entry
					(
						thread, 
						work->std.ce
					)
				);
				
				/*
				* Setup threaded object for runtime
				*/	
				if ((threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(that)))) {
					zend_string *prop = zend_string_init(ZEND_STRL("worker"), 1);
					
					work->tid = thread->tid;
					work->tls = thread->tls;
					
					pthreads_connect(work, threaded);
					pthreads_store_write(
						threaded->store, prop, &PTHREADS_ZG(this));
					zend_string_release(prop);
				}
			}
			
			zend_hash_index_del(
			    (HashTable*) thread->stack, thread->stack->position++);
		}
		pthreads_lock_release(thread->lock, locked);
		
		if (!bubble) {
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED)) {
			    pthreads_synchro_lock(thread->synchro);
				if (pthreads_set_state(thread, PTHREADS_ST_WAITING)) {
				    pthreads_synchro_unlock(thread->synchro);
					goto burst;
				} else pthreads_synchro_unlock(thread->synchro);
			} else return 0;
		}
	}
	
	return bubble;
} /* }}} */

/* {{{ return the number of items currently stacked */
size_t pthreads_stack_length(PTHREAD thread) {
	zend_bool locked;
	size_t counted = 0;
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		counted = zend_hash_num_elements(
		    &thread->stack->objects);
		pthreads_lock_release(thread->lock, locked);
	}
	return counted;
} /* }}} */

/* {{{ thread object constructor */
zend_object* pthreads_thread_ctor(zend_class_entry *entry) {
	PTHREAD thread = pthreads_globals_object_alloc(sizeof(*thread));
	if (!thread) {	
		return NULL;
	}

	thread->scope = PTHREADS_SCOPE_THREAD;
	pthreads_base_ctor(thread, entry);
	thread->std.handlers = &pthreads_handlers;

	return &thread->std;
} /* }}} */

/* {{{ worker object constructor */
zend_object* pthreads_worker_ctor(zend_class_entry *entry) {
	PTHREAD worker = pthreads_globals_object_alloc(sizeof(*worker));
	if (!worker) {
		return NULL;
	}

	worker->scope = PTHREADS_SCOPE_WORKER;
	pthreads_base_ctor(worker, entry);
	worker->std.handlers = &pthreads_handlers;

	return &worker->std;
} /* }}} */

/* {{{ threaded object constructor */
zend_object* pthreads_threaded_ctor(zend_class_entry *entry) {
	PTHREAD threaded = pthreads_globals_object_alloc(sizeof(*threaded));
	if (!threaded) {
		return NULL;
	}

	threaded->scope = PTHREADS_SCOPE_THREADED;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
void pthreads_current_thread(zval *return_value) {
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		ZVAL_COPY(return_value, &PTHREADS_ZG(this));
	}
} /* }}} */

/* {{{ connect pthread objects */
static int pthreads_connect(PTHREAD source, PTHREAD destination) {
	if (source && destination) {

		if (PTHREADS_IS_NOT_CONNECTION(destination)) {
			pthreads_lock_free(destination->lock);
			pthreads_state_free(destination->state );
			pthreads_store_free(destination->store);
			pthreads_synchro_free(destination->synchro);
			pthreads_address_free(destination->address);

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
				destination
			);
		}
		
		destination->thread = source->thread;
		destination->tid = source->tid;
		destination->tls = source->tls;
		destination->address = source->address;
		destination->lock = source->lock;
		destination->state = source->state;
		destination->synchro = source->synchro;
		destination->store = source->store;
		destination->stack = source->stack;
		
		if (PTHREADS_IS_DETACHED(source)) {
		    destination->scope |= PTHREADS_SCOPE_DETACHED;
		}
		
		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ pthreads_base_init */
static inline void pthreads_base_init(PTHREAD base) {	
	zend_property_info *info;
	zend_string *key;
	ZEND_HASH_FOREACH_PTR(&base->std.ce->properties_info, info) {
		zend_ulong offset;
		const char *clazz = NULL, 
			    *prop = NULL;
		size_t plen = 0;

		if (info->flags & ZEND_ACC_STATIC) {
			continue;
		}

		offset = OBJ_PROP_TO_NUM(info->offset);

		zend_unmangle_property_name_ex(
			info->name, &clazz, &prop, &plen);

		key = zend_string_init(prop, plen, 0);
		pthreads_store_write(
			base->store, key,
			&base->std.ce->default_properties_table[offset]);
		zend_string_release(key);

	} ZEND_HASH_FOREACH_END();

} /* }}} */

/* {{{ pthreads base constructor */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry) {
	if (base) {
		TSRMLS_CACHE_UPDATE();
		
		zend_object_std_init(&base->std, entry);

		base->cls = TSRMLS_CACHE;
		base->cid = pthreads_self();
		base->address = pthreads_address_alloc(base);
		base->options = PTHREADS_INHERIT_ALL;

		if (!PTHREADS_IS_CONNECTION(base)) {
			base->lock = pthreads_lock_alloc();
			base->state = pthreads_state_alloc(0);
			base->synchro = pthreads_synchro_alloc();
			base->store = pthreads_store_alloc();

			if (PTHREADS_IS_WORKER(base)) {
				base->stack = (pthreads_stack) calloc(1, sizeof(*base->stack));
				if (base->stack) {
					zend_hash_init(
						&base->stack->objects, 8, NULL, NULL, 1);
                    			base->stack->position = 0L;
				}
			}

			pthreads_base_init(base);
		}
	}
} /* }}} */

/* {{{ pthreads base destructor */
void pthreads_base_free(zend_object *object) {
	PTHREAD base = PTHREADS_FETCH_FROM(object);

	if (PTHREADS_IS_NOT_CONNECTION(base) && PTHREADS_IS_NOT_DETACHED(base)) {
		if (PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) {
			pthread_t *pthread = &base->thread;
			if (pthread) {
				pthreads_join(base);
			}
		}

		pthreads_lock_free(base->lock);
		pthreads_state_free(base->state );
		pthreads_store_free(base->store);
		pthreads_synchro_free(base->synchro);
		pthreads_address_free(base->address);

		if (PTHREADS_IS_WORKER(base)) {
		    zend_hash_destroy(&base->stack->objects);
		    free(base->stack);
		}
	}

	if (object->properties) {
		zend_hash_destroy(object->properties);
		FREE_HASHTABLE(object->properties);
	}
	
	if (GC_FLAGS(object) & IS_OBJ_HAS_GUARDS) {
		HashTable *guards = Z_PTR(object->properties_table[object->ce->default_properties_count]);
		
		if (guards) {
			zend_hash_destroy(guards);
			FREE_HASHTABLE(guards);
		}
	}

	pthreads_globals_object_delete(base);
} /* }}} */

/* {{{ clone object */
static void pthreads_base_clone(void *arg, void **pclone) {
	printf("pthreads_base_clone: executing ...\n");
} /* }}} */

/* {{{ start a pthread */
int pthreads_start(PTHREAD thread) {
	int dostart = 0;
	int started = FAILURE;
	zend_bool tlocked, slocked;
	if (pthreads_state_lock(thread->state, &slocked)) {
		if (!pthreads_state_check(thread->state, PTHREADS_ST_STARTED)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_STARTED);
			dostart = 1;
		} else started = PTHREADS_ST_STARTED;
		if (slocked)
			pthreads_state_unlock(thread->state, slocked);
		
	}
	if (dostart) {
		if (pthreads_lock_acquire(thread->lock, &tlocked)) {
			started = pthread_create(&thread->thread, NULL, pthreads_routine, (void*)thread);
			if (started == SUCCESS) 
				pthreads_state_wait(thread->state, PTHREADS_ST_RUNNING);
			pthreads_lock_release(thread->lock, tlocked);
		}
	}
	
	return started;
} /* }}} */

/* {{{ gracefully join a pthread object */
int pthreads_join(PTHREAD thread) {
	int dojoin = 0;
	int donotify = 0;
	zend_bool slocked;
	
	if (pthreads_state_lock(thread->state, &slocked)) {
		if (pthreads_state_check(thread->state, PTHREADS_ST_STARTED) && 
			!pthreads_state_check(thread->state, PTHREADS_ST_JOINED)) {
			pthreads_state_set_locked(thread->state, PTHREADS_ST_JOINED);
			if (PTHREADS_IS_WORKER(thread))
				donotify = pthreads_state_check(thread->state, PTHREADS_ST_WAITING);
			dojoin = 1;
		}
		if (slocked)
			pthreads_state_unlock(thread->state, slocked);
	}
	
	if (donotify) do {
		pthreads_unset_state(thread, PTHREADS_ST_WAITING);
	} while(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING));
	
	return dojoin ? pthread_join(thread->thread, NULL) 	: FAILURE;
} /* }}} */

/* {{{ detach a thread */
int pthreads_detach(PTHREAD thread) {

    if (PTHREADS_IS_NOT_DETACHED(thread)) {
        if ((pthread_detach(thread->thread) == SUCCESS)) {
            thread->scope |= PTHREADS_SCOPE_DETACHED;
            
            return SUCCESS;
        }
    }
    
    return FAILURE;
} /* }}} */

/* {{{ synchronization helper */
zend_bool pthreads_wait_member_ex(PTHREAD thread, zval *member, ulong timeout) {
	if (!pthreads_store_isset(thread->store, Z_STR_P(member), 2)) {
		if (pthreads_synchro_wait_ex(thread->synchro, timeout))
			return pthreads_store_isset(thread->store, Z_STR_P(member), 2);
		else return 0;
	} else return 1;
} /* }}} */

/* {{{ synchronization helper */
zend_bool pthreads_wait_member(PTHREAD thread, zval *member) {
	return pthreads_wait_member_ex(thread, member, 0L);
} /* }}} */

/* {{{ serialize an instance of a threaded object for connection in another thread */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, size_t *blength, zend_serialize_data *data) {
	PTHREAD threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	if (threaded) {
		(*buffer) = (unsigned char*) estrndup(
			(char*)threaded->address->serial, 
			threaded->address->length
		);
		(*blength) = threaded->address->length + 1;
		
		return SUCCESS;
	}
	return FAILURE;
} /* }}} */

/* {{{ connects to an instance of a threaded object */
int pthreads_internal_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t blength, zend_unserialize_data *data) {
	PTHREAD address = NULL;
	pid_t pid = 0L;
	size_t scanned = sscanf(
		(const char*)buffer, 
		"%lu:%lu", 
		(long unsigned int *)&pid, (long unsigned int *)&address);
	
	if (scanned) {
		pid_t mpid = PTHREADS_PID();

		if (address && pthreads_globals_object_validate((zend_ulong)address)) {
			if (pid == mpid) {
				/*if we already own this object do not create another handle */
				if (address->cls == TSRMLS_CACHE) {
					ZVAL_OBJ(object, &address->std);
					Z_ADDREF_P(object);
					return SUCCESS;
				}

				/* else initialize and connect to the original object */
				if (object_init_ex(object, ce) == SUCCESS) {
					pthreads_connect(address, PTHREADS_FETCH_FROM(Z_OBJ_P(object)));
					return SUCCESS;
				}
			} else {
				zend_throw_exception_ex(
					spl_ce_RuntimeException, 0,
					"pthreads detected an attempt to connect to a %s "
					"which belongs to another process", ce->name->val);
			}
		} else {
			zend_throw_exception_ex(
				spl_ce_RuntimeException, 0,
				"pthreads detected an attempt to connect to a %s "
				"which has already been destroyed", ce->name->val);
		}
	} else {
		zend_throw_exception_ex(
		spl_ce_RuntimeException, 0,
		"pthreads detected an attempt to connect to a %s "
		"which is corrupted", ce->name->val);
	}
	
	//ZVAL_NULL(object);
	
	return FAILURE;
} /* }}} */

#ifdef PTHREADS_KILL_SIGNAL
static inline void pthreads_kill_handler(int signo) /* {{{ */
{	

	PTHREAD current = PTHREADS_FETCH_FROM(Z_OBJ(PTHREADS_ZG(this)));
	
	if (current) {
		pthreads_state_set(
		    current->state, PTHREADS_ST_ERROR);
	}
	
	PTHREADS_ZG(signal) = signo;
	zend_bailout();
} /* }}} */
#endif

/* {{{ this is aptly named ... */
static void * pthreads_routine(void *arg) {
	/* passed the object as argument */
	PTHREAD thread = (PTHREAD) arg;
	
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
		
		/* $this */
		zval that;		
		
		/**
		* Startup Block Begin
		**/
		/* create new context */
		TSRMLS_CACHE = thread->tls 
			= tsrm_new_interpreter_context();

		/* set interpreter context */
		tsrm_set_interpreter_context(TSRMLS_CACHE);


#ifdef PTHREADS_PEDANTIC
		/* acquire a global lock */
		pthreads_globals_lock(&glocked);
#endif

		/* set thread id for this object */
		thread->tid = pthreads_self();
		
		/* set context the same as parent */
		SG(server_context) = PTHREADS_SG(thread->cls, server_context);
		
		/* some php globals */
		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;
		
		/* fixup sessions for compatibility */
		if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
			/*zend_alter_ini_entry(
				"session.cache_limiter", 
				sizeof("session.cache_limiter"), 
				"nocache", sizeof("nocache")-1, 
				PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
			zend_alter_ini_entry(
				"session.use_cookies", 
				sizeof("session.use_cookies"), 
				"0", sizeof("0")-1,
				PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);*/
		}

		/* request startup */
		php_request_startup();

		/* fix php-fpm compatibility */
		SG(sapi_started) = 0;
		
		if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
			/* do not send headers again */
			SG(headers_sent)=1;
			SG(request_info).no_headers = 1;
		}

		/* prepare environment */
		pthreads_prepare(thread);

#ifdef PTHREADS_PEDANTIC
		/* release global lock */
		pthreads_globals_unlock(glocked);
#endif

		/**
		* Startup Block End
		**/

		/**
		* Thread Block Begin
		**/
		zend_first_try {
			/*
			* Set worker indicator
			*/
			worker = PTHREADS_IS_WORKER(thread);
					
			EG(current_module) = &pthreads_module_entry;

			/*  */
			ZVAL_UNDEF(&PTHREADS_ZG(this));		
			object_init_ex(
				&PTHREADS_ZG(this),
				pthreads_prepared_entry(thread, thread->std.ce));
			ZVAL_COPY(&that, &PTHREADS_ZG(this));

			/* connect $this */
			if (pthreads_connect(thread, PTHREADS_FETCH_FROM(Z_OBJ(PTHREADS_ZG(this))))==SUCCESS) {
				/* execute $this */
				do {
					PTHREAD current = PTHREADS_FETCH_FROM(Z_OBJ(that));
					zval zresult;					
					ZVAL_UNDEF(&zresult);

					pthreads_state_set(current->state, PTHREADS_ST_RUNNING);
					{
						zend_bool terminated = 0;
						
						/* graceful fatalities */
						zend_try {
							zend_function *fun;
							zend_fcall_info fci = empty_fcall_info;
							zend_fcall_info_cache fcc = empty_fcall_info_cache;
							zend_string *method = zend_string_init(ZEND_STRL("run"), 0);

							if ((fun = zend_hash_find_ptr(&Z_OBJCE(that)->function_table, method))) {							
								if (fun->type == ZEND_USER_FUNCTION) {
									fci.size = sizeof(zend_fcall_info);
								    	fci.retval = &zresult;
									fci.object = Z_OBJ(that);
									fci.no_separation = 1;
									fcc.initialized = 1;
									fcc.object = Z_OBJ(that);
									fcc.calling_scope = Z_OBJCE(that);
									fcc.called_scope = Z_OBJCE(that);
									fcc.function_handler = fun;
									
									zend_call_function(&fci, &fcc);
								}
							}
							zend_string_free(method);
						} zend_catch {
						    /* catches fatal errors and uncaught exceptions */
						    pthreads_state_set(
							    current->state, PTHREADS_ST_ERROR);				

						    /* danger lurking ... */
						    if (PTHREADS_ZG(signal) == PTHREADS_KILL_SIGNAL) {
							    /* like, totally bail man ! */
							    zend_bailout();
						    }
						} zend_end_try();

						if (current) {
						    /* unset running for waiters */
						    pthreads_state_unset(current->state, PTHREADS_ST_RUNNING);
						}

						/* deal with references to stacked objects */
						if (!terminated) {
							Z_SET_REFCOUNT(that, 0);
							zval_dtor(&that);
						}

						/* deal with zresult (ignored) */
						if (Z_TYPE(zresult) != IS_UNDEF) {
							zval_ptr_dtor(&zresult);
						}
					}
				} while(worker && pthreads_stack_next(&that));
			}
		} zend_end_try();
		
		/**
		* Thread Block End
		**/
		
	        /*
		* Destroy $this
		*/
		zval_ptr_dtor_nogc(&PTHREADS_ZG(this));
		
		/**
		* Shutdown Block Begin
		**/
		PG(report_memleaks) = 0;
		
#ifdef PTHREADS_PEDANTIC
		/* acquire global lock */
		pthreads_globals_lock(&glocked);
#endif
		/* shutdown request */
		php_request_shutdown((void*)NULL);

#ifdef PTHREADS_PEDANTIC
		/* release global lock */
		pthreads_globals_unlock(glocked);
#endif

		/* free interpreter */
		tsrm_free_interpreter_context(TSRMLS_CACHE);

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

