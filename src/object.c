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
static void pthreads_base_ctor(pthreads_object_t* base, zend_class_entry *entry); /* }}} */

/* {{{ */
static void * pthreads_routine(void *arg); /* }}} */

static inline void pthreads_object_iterator_dtor(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF)
		zval_ptr_dtor(&iterator->zit.data);
	zval_ptr_dtor(&iterator->object);
}

static inline int pthreads_object_iterator_validate(pthreads_iterator_t* iterator) {
	return (iterator->position != HT_INVALID_IDX) ? SUCCESS : FAILURE;
}

static inline zval* pthreads_object_iterator_current_data(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		zval_ptr_dtor(&iterator->zit.data);
	}

	pthreads_store_data(&iterator->object, &iterator->zit.data, &iterator->position);

	if (Z_ISUNDEF(iterator->zit.data)) {
		return &EG(uninitialized_zval);
	}

    return &iterator->zit.data;
}

static inline void pthreads_object_iterator_current_key(pthreads_iterator_t* iterator, zval* result) {
    pthreads_store_key(&iterator->object, result, &iterator->position);
}

static inline void pthreads_object_iterator_move_forward(pthreads_iterator_t* iterator) {
    pthreads_store_forward(&iterator->object, &iterator->position);
}

static inline void pthreads_object_iterator_rewind(pthreads_iterator_t* iterator) {
    pthreads_store_reset(&iterator->object, &iterator->position);
}

static zend_object_iterator_funcs pthreads_object_iterator_funcs = {
    (void (*) (zend_object_iterator*)) 				pthreads_object_iterator_dtor,
    (int (*)(zend_object_iterator *)) 				pthreads_object_iterator_validate,
    (zval* (*)(zend_object_iterator *)) 			pthreads_object_iterator_current_data,
    (void (*)(zend_object_iterator *, zval *)) 		pthreads_object_iterator_current_key,
    (void (*)(zend_object_iterator *))				pthreads_object_iterator_move_forward,
    (void (*)(zend_object_iterator *)) 				pthreads_object_iterator_rewind
};

zend_object_iterator* pthreads_object_iterator_create(zend_class_entry *ce, zval *object, int by_ref) {
    pthreads_iterator_t *iterator;

	if (by_ref) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
			"iteration by reference is not allowed on %s objects", ZSTR_VAL(ce->name));
		return NULL;
	}
	
	iterator = (pthreads_iterator_t*) 
		ecalloc(1, sizeof(pthreads_iterator_t));
	
    zend_iterator_init((zend_object_iterator*)iterator);

	ZVAL_COPY(&iterator->object, object);
	ZVAL_UNDEF(&iterator->zit.data);

	pthreads_store_reset(&iterator->object, &iterator->position);

    iterator->zit.funcs = &pthreads_object_iterator_funcs;

    return (zend_object_iterator*) iterator;
}

/* {{{ */
zend_object* pthreads_thread_ctor(zend_class_entry *entry) {
	pthreads_object_t* thread = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	thread->scope = PTHREADS_SCOPE_THREAD;
	pthreads_base_ctor(thread, entry);
	thread->std.handlers = &pthreads_handlers;

	return &thread->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_worker_ctor(zend_class_entry *entry) {
	pthreads_object_t* worker = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	worker->scope = PTHREADS_SCOPE_WORKER;
	pthreads_base_ctor(worker, entry);
	worker->std.handlers = &pthreads_handlers;

	return &worker->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_threaded_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_THREADED;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
int pthreads_threaded_serialize(zval *object, unsigned char **buffer, size_t *buflen, zend_serialize_data *data) {
	pthreads_object_t *address = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	(*buflen) = snprintf(NULL, 0, ":%lu:", (long unsigned int) address);
	(*buffer) = emalloc((*buflen) + 1);
	sprintf(
		(*buffer), ":%lu:", (long unsigned int) address);
	(*buffer)[(*buflen)] = 0;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_threaded_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t buflen, zend_unserialize_data *data) {
	pthreads_object_t *address = NULL;	

	if (!sscanf((const char*) buffer, ":%lu:", (long unsigned int*)&address)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"pthreads detected an attempt to connect to a corrupted object");
		return FAILURE;
	}

	if (!address) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
			"pthreads detected an attempt to connect to an invalid object");
		return FAILURE;	
	}
	
	if (!pthreads_globals_object_connect((zend_ulong) address, ce, object)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
			"pthreads detected an attempt to connect to an object which has already been destroyed");
		return FAILURE;
	}

	return SUCCESS;
} /* }}} */

/* {{{ */
void pthreads_current_thread(zval *return_value) {
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		ZVAL_COPY(return_value, &PTHREADS_ZG(this));
	}
} /* }}} */

/* {{{ */
int pthreads_connect(pthreads_object_t* source, pthreads_object_t* destination) {
	if (source && destination) {
		if (PTHREADS_IS_NOT_CONNECTION(destination)) {
			pthreads_store_free(destination->store);	

			if (PTHREADS_IS_WORKER(destination)) {
				pthreads_stack_free(destination->stack);
			}

			pthreads_monitor_free(destination->ready);
			pthreads_monitor_free(destination->monitor);	

			destination->scope |= PTHREADS_SCOPE_CONNECTION;

			return pthreads_connect(source, destination);
		}

		destination->thread = source->thread;
		destination->local.id = source->local.id;
		destination->local.ls = source->local.ls;
		destination->monitor = source->monitor;
		destination->ready  = source->ready;
		destination->store = source->store;
		destination->stack = source->stack;	
		
		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ */
static inline void pthreads_base_init(pthreads_object_t* base) {	
	zend_property_info *info;
	zend_string *key;
	zval tmp;

	ZVAL_OBJ(&tmp, &base->std);	

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
			&tmp, key,
			&base->std.ce->default_properties_table[offset]);
		zend_string_release(key);
	
	} ZEND_HASH_FOREACH_END();

} /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_object_t* base, zend_class_entry *entry) {
	zend_object_std_init(&base->std, entry);
	object_properties_init(&base->std, entry);

	base->creator.ls = TSRMLS_CACHE;
	base->creator.id = pthreads_self();
	base->options = PTHREADS_INHERIT_ALL;

	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		base->monitor = pthreads_monitor_alloc();
		base->ready   = pthreads_monitor_alloc();
		base->store   = pthreads_store_alloc();

		if (PTHREADS_IS_WORKER(base)) {
			base->stack = pthreads_stack_alloc(base->monitor);
		}

		pthreads_base_init(base);
	}
} /* }}} */

/* {{{ */
void pthreads_base_free(zend_object *object) {
	pthreads_object_t* base = PTHREADS_FETCH_FROM(object);
	
	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		if ((PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) &&
			!pthreads_monitor_check(base->monitor, PTHREADS_MONITOR_JOINED)) {
			pthreads_join(base);
		}

		if (pthreads_monitor_lock(base->monitor)) {
			pthreads_store_free(base->store);

			if (PTHREADS_IS_WORKER(base)) {
				pthreads_stack_free(base->stack);	
			}

			pthreads_monitor_free(base->monitor);	
		}
	}

	zend_object_std_dtor(object);

	pthreads_globals_object_delete(base);
} /* }}} */

/* {{{ */
zend_object* pthreads_base_clone(zval *object) {
	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects cannot be cloned", ZSTR_VAL(Z_OBJCE_P(object)->name));

	/* assume this is okay ? */
	return Z_OBJ_P(object);
} /* }}} */

/* {{{ */
HashTable* pthreads_base_gc(zval *object, zval **table, int *n) {
	*table = NULL;
	*n = 0;
	return Z_OBJ_P(object)->properties;
} /* }}} */

/* {{{ */
zend_bool pthreads_start(pthreads_object_t* thread) {

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
			pthreads_monitor_wait_until(thread->ready, PTHREADS_MONITOR_READY);
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
zend_bool pthreads_join(pthreads_object_t* thread) {

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 
			0, "only the creator of this %s may join with it",
			thread->std.ce->name->val);
		return 0;
	}

	if (pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"the creator of %s already joined with it",
			thread->std.ce->name->val);
		return 0;
	}

	pthreads_monitor_add(thread->monitor, PTHREADS_MONITOR_JOINED);

	return (pthread_join(thread->thread, NULL) == SUCCESS);
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_routine_run_function(pthreads_object_t* object, pthreads_object_t* connection, zval *work) {
	zend_function *run;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	zval zresult;	

	if (pthreads_connect(object, connection) != SUCCESS) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_monitor_add(object->monitor, PTHREADS_MONITOR_RUNNING);

	if (work)
		pthreads_store_write(work, PTHREADS_G(strings).worker, &PTHREADS_ZG(this));

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
	    pthreads_monitor_add(object->monitor, PTHREADS_MONITOR_ERROR);		

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
} /* }}} */

/* {{{ */
static void * pthreads_routine(void *arg) {
	pthreads_object_t* thread = (pthreads_object_t*) arg;

	pthreads_prepared_startup(thread);

	zend_first_try {
		ZVAL_UNDEF(&PTHREADS_ZG(this));
		object_init_ex(&PTHREADS_ZG(this), pthreads_prepared_entry(thread, thread->std.ce));
		pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))), NULL);

		if (PTHREADS_IS_WORKER(thread)) {
			zval stacked;

			while (pthreads_stack_next(thread->stack, &stacked) != PTHREADS_MONITOR_JOINED) {
				zval that;
				pthreads_object_t* work = PTHREADS_FETCH_FROM(Z_OBJ(stacked));

				object_init_ex(&that, pthreads_prepared_entry(thread, work->std.ce));
				pthreads_routine_run_function(work, PTHREADS_FETCH_FROM(Z_OBJ(that)), &that);
				zval_ptr_dtor(&that);
			}
		}

		zval_ptr_dtor(&PTHREADS_ZG(this));
	} zend_end_try();

	pthreads_prepared_shutdown(thread);

	pthread_exit(NULL);

#ifdef _WIN32
	return NULL;
#endif
} /* }}} */

#endif

