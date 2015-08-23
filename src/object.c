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

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_PREPARE_H
#	include <src/prepare.h>
#endif

/* {{{ */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ */
static void pthreads_base_ctor(PTHREAD base, zend_class_entry *entry);
static void pthreads_base_clone(void *arg, void **pclone); /* }}} */

/* {{{ */
static void * pthreads_routine(void *arg); /* }}} */

/* {{{ */
uint32_t pthreads_stack_pop(PTHREAD thread, zval *work) {
	uint32_t left = 0;

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 
			0, "only the creator of this %s may pop from the stack", 
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_lock(thread->monitor)) {
		if (PTHREADS_IS_WORKER(thread)) {
			if (!work) {
				zend_hash_destroy(&thread->stack->objects);
			}
			left = zend_hash_num_elements(&thread->stack->objects);
		}
		pthreads_monitor_unlock(thread->monitor);
	}

	return left;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_push(PTHREAD thread, zval *work) {
	uint32_t counted = 0;

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may push to the stack",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_lock(thread->monitor)) {
		if (zend_hash_next_index_insert(&thread->stack->objects, work)) {
			Z_ADDREF_P(work);
			counted = zend_hash_num_elements(&thread->stack->objects);
		}
		pthreads_monitor_notify(thread->monitor);
		pthreads_monitor_unlock(thread->monitor);
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
	uint32_t waiting = 0;

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,	
			"only the creator of this %s may collect from the stack",
			thread->std.ce->name->val);
		return 0;
	}	

	if (pthreads_monitor_lock(thread->monitor)) {
		zend_hash_apply_with_argument(
			&thread->stack->objects,
			pthreads_stack_collect_function, call);
		waiting = zend_hash_num_elements(&thread->stack->objects);
		if (waiting == 0) {
			/* destroy and reset stack to free up unused memory */
			zend_hash_destroy(&thread->stack->objects);
			zend_hash_init(
				&thread->stack->objects, 8, NULL, ZVAL_PTR_DTOR, 0);
		} else pthreads_monitor_notify(thread->monitor);
		pthreads_monitor_unlock(thread->monitor);
	}

	return waiting;
} /* }}} */

/* {{{ */
uint32_t pthreads_stack_length(PTHREAD thread) {
	uint32_t length = 0;
	if (pthreads_monitor_lock(thread->monitor)) {
		length = zend_hash_num_elements
			(&thread->stack->objects);
		pthreads_monitor_unlock(thread->monitor);
	}
	return length;
} /* }}} */

/* {{{ */
zend_object* pthreads_thread_ctor(zend_class_entry *entry) {
	PTHREAD thread = pthreads_globals_object_alloc(sizeof(*thread) + zend_object_properties_size(entry));
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
	PTHREAD worker = pthreads_globals_object_alloc(sizeof(*worker) + zend_object_properties_size(entry));
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
	PTHREAD threaded = pthreads_globals_object_alloc(sizeof(*threaded) + zend_object_properties_size(entry));
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
int pthreads_connect(PTHREAD source, PTHREAD destination) {
	if (source && destination) {
		if (PTHREADS_IS_NOT_CONNECTION(destination)) {
			if (destination->store)
				pthreads_store_free(destination->store);
			if (destination->monitor)
				pthreads_monitor_free(destination->monitor);

			if (PTHREADS_IS_WORKER(destination)) {
				if (destination->stack) {
					zend_hash_destroy(&destination->stack->objects);
					free(destination->stack);
				}
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
		destination->monitor = source->monitor;
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
	TSRMLS_CACHE_UPDATE();

	zend_object_std_init(&base->std, entry);

	base->creator.ls = TSRMLS_CACHE;
	base->creator.id = pthreads_self();
	base->options = PTHREADS_INHERIT_ALL;

	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		base->monitor = pthreads_monitor_alloc();
		base->store = pthreads_store_alloc(base->monitor);

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
} /* }}} */

/* {{{ */
void pthreads_base_free(zend_object *object) {
	PTHREAD base = PTHREADS_FETCH_FROM(object);

	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		if ((PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) &&
			!pthreads_monitor_check(base->monitor, PTHREADS_MONITOR_JOINED)) {
			pthread_t *pthread = &base->thread;
			if (pthread) {
				pthreads_join(base);
			}
		}

		pthreads_store_free(base->store);
		pthreads_monitor_free(base->monitor);

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
zend_bool pthreads_start(PTHREAD thread) {
	int dostart = 0;
	int started = FAILURE;

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 
			0, "only the creator of this %s may start it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already started it", thread->std.ce->name->val);
		return 0;
	}

	pthreads_monitor_add(thread->monitor, PTHREADS_MONITOR_STARTED);

	switch (pthread_create(&thread->thread, NULL, pthreads_routine, (void*)thread)) {
		case SUCCESS:
			pthreads_monitor_wait_until(thread->monitor, PTHREADS_MONITOR_READY);
			return 1;

		case EAGAIN:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, out of resources", thread->std.ce->name->val);
		break;

		default:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, unknown error", thread->std.ce->name->val);
	}
	
	pthreads_monitor_remove(thread->monitor, PTHREADS_MONITOR_STARTED);
	
	return 0;
} /* }}} */

/* {{{ */
zend_bool pthreads_join(PTHREAD thread) {

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 
			0, "only the creator of this %s may join with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_lock(thread->monitor)) {
		if (pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED)) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"the creator of %s already joined with it",
				thread->std.ce->name->val);
			pthreads_monitor_unlock(thread->monitor);
			return 0;
		}

		pthreads_monitor_add(thread->monitor, PTHREADS_MONITOR_JOINED);
		pthreads_monitor_unlock(thread->monitor);

		return (pthread_join(thread->thread, NULL) == SUCCESS);
	}
	
	return 0;
} /* }}} */

#ifdef PTHREADS_KILL_SIGNAL
static inline void pthreads_kill_handler(int signo) /* {{{ */
{	
	PTHREAD current = PTHREADS_FETCH_FROM(Z_OBJ(PTHREADS_ZG(this)));
	
	if (current) {
		pthreads_monitor_add(
		    current->monitor, PTHREADS_MONITOR_ERROR);
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

static inline zend_bool pthreads_routine_run_function(PTHREAD object, PTHREAD connection) {
	zend_function *run;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	zval zresult;					

	if (pthreads_connect(object, connection) != SUCCESS) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_monitor_add(object->monitor, PTHREADS_MONITOR_RUNNING);

	zend_try {
		if ((run = zend_hash_find_ptr(&connection->std.ce->function_table, PTHREADS_G(strings).run))) {							
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
	    pthreads_monitor_add(
		    object->monitor, PTHREADS_MONITOR_ERROR);		

	    if (PTHREADS_ZG(signal) == PTHREADS_KILL_SIGNAL) {
		    /* like, totally bail man ! */
		    zend_bailout();
	    }
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pthreads_monitor_remove(object->monitor, PTHREADS_MONITOR_RUNNING);

	return 1;
}

extern zend_class_entry *pthreads_collectable_entry;

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
		zend_alter_ini_entry_chars(
			PTHREADS_G(strings).session.cache_limiter,
			"nocache", sizeof("nocache")-1, 
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
		zend_alter_ini_entry_chars(
			PTHREADS_G(strings).session.use_cookies,
			"0", sizeof("0")-1,
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
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

	pthreads_monitor_add(thread->monitor, PTHREADS_MONITOR_READY);

	zend_first_try {
		ZVAL_UNDEF(&PTHREADS_ZG(this));
		object_init_ex(
			&PTHREADS_ZG(this),
			pthreads_prepared_entry(thread, thread->std.ce));
		pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))));
		
		if (PTHREADS_IS_WORKER(thread)) {
			do {
				if (pthreads_monitor_lock(thread->monitor)) {
					zval *next;

					ZEND_HASH_FOREACH_VAL(&thread->stack->objects, next) {
						if (!pthreads_collectable_is_garbage(next)) {
							PTHREAD work = PTHREADS_FETCH_FROM(Z_OBJ_P(next));
							
							object_init_ex(&that, pthreads_prepared_entry(thread, work->std.ce));
							pthreads_store_write(
								work->store, PTHREADS_G(strings).worker, &PTHREADS_ZG(this));
							pthreads_routine_run_function(
								work, PTHREADS_FETCH_FROM(Z_OBJ(that)));
							zval_dtor(&that);

							pthreads_collectable_set_garbage(next);
						}
					} ZEND_HASH_FOREACH_END();

					if (pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED)) {
						pthreads_monitor_unlock(thread->monitor);						
						break;
					}

					pthreads_monitor_wait(thread->monitor, 0);
					pthreads_monitor_unlock(thread->monitor);
				}
			} while (1);
		}

		zval_ptr_dtor(&PTHREADS_ZG(this));
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

