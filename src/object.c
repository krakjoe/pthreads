/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
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

/* {{{ */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ */
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
} /* }}} */

/* {{{ */
static inline void pthreads_address_free(pthreads_address address) {
	if (address->serial) {
		free(address->serial);
	}
	free(address);
} /* }}} */

/* {{{ */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry);
static void pthreads_base_clone(void *arg, void **pclone); /* }}} */

/* {{{ */
static int pthreads_connect(PTHREAD source, PTHREAD destination); /* }}} */

/* {{{ */
static void * pthreads_routine(void *arg); /* }}} */

/* {{{ */
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

/* {{{ */
zend_bool pthreads_set_state(PTHREAD thread, int mask) {
	return pthreads_set_state_ex(thread, mask, 0L);
} /* }}} */

/* {{{ */
zend_bool pthreads_unset_state(PTHREAD thread, int mask){
	zend_bool result = 0;
	
	if (mask & PTHREADS_ST_WAITING) {
		if (pthreads_state_unset(thread->state, mask)) {
			result = pthreads_synchro_notify(thread->synchro);
		} else result = 0;
	} else result = pthreads_state_unset(thread->state, mask);
	
	return result;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_pop(PTHREAD thread, zval *work) {
	zend_bool locked;
	uint32_t left = 0;
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		if (PTHREADS_IS_WORKER(thread)) {
			if (!work) {
				zend_hash_destroy(&thread->stack->objects);
			}
			left = zend_hash_num_elements(&thread->stack->objects);
		}
		pthreads_lock_release(thread->lock, locked);
	}

	return left;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_push(PTHREAD thread, zval *work) {
	zend_bool locked;
	uint32_t counted = 0;
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		HashTable *stack = &thread->stack->objects;
		if (stack) {
			counted = zend_hash_num_elements(stack);
			
			zend_hash_next_index_insert(stack, work);
			Z_ADDREF_P(work);

			if (counted == 0) {
				zend_hash_internal_pointer_reset_ex(
					&thread->stack->objects, &thread->stack->position);
			}
			
			counted = zend_hash_num_elements(stack);
		}

		pthreads_lock_release(thread->lock, locked);
		
		if (counted > 0) {
		    pthreads_synchro_lock(thread->synchro);
			pthreads_unset_state(
			    thread, PTHREADS_ST_WAITING);
			pthreads_synchro_unlock(thread->synchro);
		}
	}
	
	return counted;
} /* }}} */

/* {{{ */
static inline int pthreads_stack_collect_function(zval *collectable, void *argument) {
	pthreads_call_t *call = (pthreads_call_t*) argument;
	int apply = ZEND_HASH_APPLY_KEEP | ZEND_HASH_APPLY_STOP;
	zval result;

	ZVAL_UNDEF(&result);

	call->fci.retval = &result;				
	call->fci.no_separation = 1;

	zend_fcall_info_argn(&call->fci, 1, collectable);

	if (zend_call_function(&call->fci, &call->fcc) != SUCCESS) {
		return apply;
	}

	zend_fcall_info_args_clear(&call->fci, 1);

	if (Z_TYPE(result) != IS_UNDEF) {
		if (zend_is_true(&result)) {
			apply = ZEND_HASH_APPLY_REMOVE;
		}
		zval_dtor(&result);
	}

	return apply;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_collect(PTHREAD thread, pthreads_call_t *call) {
	zend_bool locked;
	uint32_t waiting = 0;
	
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		zend_hash_apply_with_argument(
			&thread->stack->objects,
			pthreads_stack_collect_function, call);
		waiting = zend_hash_num_elements(&thread->stack->objects);
		if (waiting == 0) {
			/* destroy and reset stack to free up unused memory */
			zend_hash_destroy(&thread->stack->objects);
			zend_hash_init(
				&thread->stack->objects, 8, NULL, ZVAL_PTR_DTOR, 0);
		} else {
			zend_bool state;

			if (pthreads_state_lock(thread->state, &state)) {
				if (pthreads_state_check(thread->state, PTHREADS_ST_WAITING)) {
					pthreads_unset_state(thread, PTHREADS_ST_WAITING);
				}
				pthreads_state_unlock(thread->state, state);
			}
		}

		pthreads_lock_release(thread->lock, locked);
	}

	return waiting;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_length(PTHREAD thread) {
	zend_bool locked;
	uint32_t length = 0;
	if (pthreads_lock_acquire(thread->lock, &locked)) {
		length = zend_hash_num_elements
			(&thread->stack->objects);
		pthreads_lock_release(thread->lock, locked);
	}
	return length;
} /* }}} */

/* {{{ */
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

/* {{{ */
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

/* {{{ */
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

/* {{{ */
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
		destination->local.id = source->local.id;
		destination->local.ls = source->local.ls;
		destination->address = source->address;
		destination->lock = source->lock;
		destination->state = source->state;
		destination->synchro = source->synchro;
		destination->store = source->store;
		destination->stack = source->stack;
		
		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ */
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

/* {{{ */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry) {
	if (base) {
		TSRMLS_CACHE_UPDATE();

		zend_object_std_init(&base->std, entry);

		base->creator.ls = TSRMLS_CACHE;
		base->creator.id = pthreads_self();
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
						&base->stack->objects, 8, NULL, ZVAL_PTR_DTOR, 0);
                    			zend_hash_internal_pointer_reset_ex(&base->stack->objects, &base->stack->position);
				}
			}

			pthreads_base_init(base);
		}
	}
} /* }}} */

/* {{{ */
void pthreads_base_free(zend_object *object) {
	PTHREAD base = PTHREADS_FETCH_FROM(object);

	if (PTHREADS_IS_NOT_CONNECTION(base)) {
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

/* {{{ */
static void pthreads_base_clone(void *arg, void **pclone) {
	printf("pthreads_base_clone: executing ...\n");
} /* }}} */

/* {{{ */
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

/* {{{ */
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

/* {{{ */
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

/* {{{ */
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
				if (address->creator.ls == TSRMLS_CACHE) {
					ZVAL_OBJ(object, &address->std);
					Z_ADDREF_P(object);
					return SUCCESS;
				}

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

static inline int pthreads_resources_cleanup(zval *bucket) {
	TSRMLS_CACHE_UPDATE();
	if (pthreads_resources_kept(Z_RES_P(bucket))) {
		return ZEND_HASH_APPLY_REMOVE;
	} else return ZEND_HASH_APPLY_KEEP;
}

static inline zend_bool pthreads_routine_run_function(PTHREAD object, PTHREAD connection, zend_string *method) {
	zend_function *run;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	zval zresult;					

	if (pthreads_connect(object, connection) != SUCCESS) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_state_set(object->state, PTHREADS_ST_RUNNING);

	zend_try {
		if ((run = zend_hash_find_ptr(&connection->std.ce->function_table, method))) {							
			if (run->type == ZEND_USER_FUNCTION) {
				call.fci.size = sizeof(zend_fcall_info);
			    	call.fci.retval = &zresult;
				call.fci.object = &connection->std;
				call.fci.no_separation = 1;
				call.fcc.initialized = 1;
				call.fcc.object = &connection->std;
				call.fcc.calling_scope = connection->std.ce;
				call.fcc.called_scope = connection->std.ce;
				call.fcc.function_handler = run;
			
				zend_call_function(&call.fci, &call.fcc);
			}
		}
	} zend_catch {
	    pthreads_state_set(
		    object->state, PTHREADS_ST_ERROR);		

	    if (PTHREADS_ZG(signal) == PTHREADS_KILL_SIGNAL) {
		    /* like, totally bail man ! */
		    zend_bailout();
	    }
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pthreads_state_unset(object->state, PTHREADS_ST_RUNNING);
	
	return 1;
}

/* {{{ */
static void * pthreads_routine(void *arg) {
	PTHREAD thread = (PTHREAD) arg;

#ifdef PTHREADS_PEDANTIC
	zend_bool  glocked = 0;
#endif
	zval that;

#ifdef PTHREADS_KILL_SIGNAL
	signal(PTHREADS_KILL_SIGNAL, pthreads_kill_handler);
#endif

	thread->local.ls 
		= tsrm_new_interpreter_context();
	tsrm_set_interpreter_context(thread->local.ls);
	TSRMLS_CACHE_UPDATE();

#ifdef PTHREADS_PEDANTIC
	pthreads_globals_lock(&glocked);
#endif

	thread->local.id = pthreads_self();

	SG(server_context) = PTHREADS_SG(thread->creator.ls, server_context);

	PG(expose_php) = 0;
	PG(auto_globals_jit) = 0;

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

	php_request_startup();

	SG(sapi_started) = 0;

	if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
		SG(headers_sent)=1;
		SG(request_info).no_headers = 1;
	}

	pthreads_prepare(thread);

#ifdef PTHREADS_PEDANTIC
	pthreads_globals_unlock(glocked);
#endif

	zend_first_try {
		zend_string *run = zend_string_init(ZEND_STRL("run"), 1);

		ZVAL_UNDEF(&PTHREADS_ZG(this));
		object_init_ex(
			&PTHREADS_ZG(this),
			pthreads_prepared_entry(thread, thread->std.ce));
		pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))), run);
		
		if (PTHREADS_IS_WORKER(thread)) {
			zend_bool locked;
			zend_string *key = zend_string_init(ZEND_STRL("worker"), 0);

			do {
				if (pthreads_lock_acquire(thread->lock, &locked)) {
					zval *next;

					ZEND_HASH_FOREACH_VAL(&thread->stack->objects, next) {
						if (!pthreads_collectable_is_garbage(next)) {
							PTHREAD work = PTHREADS_FETCH_FROM(Z_OBJ_P(next));

							ZVAL_UNDEF(&that);
							object_init_ex(
								&that,
								pthreads_prepared_entry(thread, work->std.ce));
							pthreads_store_write(
								work->store, key, &PTHREADS_ZG(this));
							pthreads_routine_run_function(
								work, PTHREADS_FETCH_FROM(Z_OBJ(that)), run);
							zval_dtor(&that);

							pthreads_collectable_set_garbage(next);
						}
					} ZEND_HASH_FOREACH_END();

					pthreads_lock_release(thread->lock, locked);

					if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED)) {
						pthreads_synchro_lock(thread->synchro);
						if (pthreads_set_state(thread, PTHREADS_ST_WAITING)) {
						    pthreads_synchro_unlock(thread->synchro);
							continue;
						} else pthreads_synchro_unlock(thread->synchro);
					} else break;
				}
			} while (1);

			zend_string_release(key);
		}

		zval_ptr_dtor(&PTHREADS_ZG(this));
		zend_string_release(run);
	} zend_end_try();

	zend_hash_apply(&EG(regular_list), pthreads_resources_cleanup);		

	PG(report_memleaks) = 0;

#ifdef PTHREADS_PEDANTIC
	pthreads_globals_lock(&glocked);
#endif
	php_request_shutdown((void*)NULL);

#ifdef PTHREADS_PEDANTIC
	pthreads_globals_unlock(glocked);
#endif
	tsrm_free_interpreter_context(TSRMLS_CACHE);

	pthread_exit(NULL);

#ifdef _WIN32
	return NULL;
#endif
} 
/* }}} */

#endif

