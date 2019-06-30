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

#ifndef HAVE_PTHREADS_STREAMS_INTERNAL_H
#	include <src/streams/internal.h>
#endif

/* {{{ */
extern zend_module_entry pthreads_module_entry; /* }}} */

/* {{{ */
static void pthreads_base_ctor(pthreads_object_t* base, zend_class_entry *entry); /* }}} */

/* {{{ */
static void * pthreads_routine(pthreads_routine_arg_t *arg); /* }}} */

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

/* {{{ */
pthreads_object_t* pthreads_object_init(zend_class_entry *ce) {
	zval object;
	object_init_ex(&object, ce);

	return PTHREADS_FETCH_FROM(Z_OBJ(object));
} /* }}} */


/* {{{ */
int pthreads_object_compare(pthreads_object_t* left, pthreads_object_t *right) {
	/* comparing property tables is not useful or efficient for threaded objects */
	/* in addition, it might be useful to know if two variables are infact the same physical threaded object */
	if (left->monitor == right->monitor) {
		return 0;
	}

	return 1;
} /* }}} */

/* {{{ */
void pthreads_ptr_dtor(pthreads_object_t* threaded) {
	zval obj;
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));
	zval_ptr_dtor(&obj);
}

void pthreads_add_ref(pthreads_object_t* threaded) {
	zval obj;
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));
	Z_ADDREF_P(&obj);
}

void pthreads_del_ref(pthreads_object_t* threaded) {
	zval obj;
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));
	Z_DELREF_P(&obj);
}

int pthreads_refcount(pthreads_object_t* threaded) {
	zval obj;
	ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded));
	return Z_REFCOUNT_P(&obj);
} /* }}} */

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
static void pthreads_routine_init(pthreads_routine_arg_t *r, pthreads_object_t *thread) {
	r->thread = thread;
	r->ready  = pthreads_monitor_alloc();
	pthreads_monitor_add(
		r->thread->monitor, PTHREADS_MONITOR_STARTED);
	pthreads_prepare_parent(thread);
}

static void pthreads_routine_wait(pthreads_routine_arg_t *r) {
	pthreads_monitor_wait_until(
		r->ready, PTHREADS_MONITOR_READY);
	pthreads_monitor_free(r->ready);
}

static void pthreads_routine_free(pthreads_routine_arg_t *r) {
	pthreads_monitor_remove(
		r->thread->monitor, PTHREADS_MONITOR_STARTED);
	pthreads_monitor_free(r->ready);
} /* }}} */

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
zend_object* pthreads_socket_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_SOCKET;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_socket_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_stream_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_context_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM_CONTEXT;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_socket_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_filter_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM_FILTER;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_wrapper_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM_WRAPPER;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_stream_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_bucket_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM_BUCKET;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
zend_object* pthreads_stream_brigade_ctor(zend_class_entry *entry) {
	pthreads_object_t* threaded = pthreads_globals_object_alloc(
		sizeof(pthreads_object_t) + zend_object_properties_size(entry));

	threaded->scope = PTHREADS_SCOPE_STREAM_BRIGADE;
	pthreads_base_ctor(threaded, entry);
	threaded->std.handlers = &pthreads_stream_handlers;

	return &threaded->std;
} /* }}} */

/* {{{ */
int pthreads_threaded_serialize(zval *object, unsigned char **buffer, size_t *buflen, zend_serialize_data *data) {
	pthreads_object_t *address = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
#ifdef _WIN64
	(*buflen) = snprintf(NULL, 0, ":%I64u:", (unsigned __int64) address);
#else
	(*buflen) = snprintf(NULL, 0, ":%lu:", (long unsigned int) address);
#endif
	(*buffer) = emalloc((*buflen) + 1);
	sprintf(
#ifdef _WIN64
		(char*) (*buffer), ":%I64u:", (unsigned __int64) address);
#else
		(char*) (*buffer), ":%lu:", (long unsigned int) address);
#endif
	(*buffer)[(*buflen)] = 0;

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_threaded_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t buflen, zend_unserialize_data *data) {
	pthreads_object_t *address = NULL;	

#ifdef _WIN64
	if (!sscanf((const char*) buffer, ":%I64u:", (unsigned __int64*)&address)) {
#else
	if (!sscanf((const char*) buffer, ":%lu:", (long unsigned int*)&address)) {
#endif
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
		pthreads_ident_t destCreator = destination->creator;

		if (PTHREADS_IS_NOT_CONNECTION(destination)) {

#if PTHREADS_STREAM_DEBUG
	printf("connecting stream object [source] std(%p) threaded(%p) type(%s) thread(%i) \n", PTHREADS_STD_P(source), source, pthreads_get_object_name(source), pthreads_self());
	printf("connecting stream object [destination] std(%p) threaded(%p) type(%s) thread(%i) \n", PTHREADS_STD_P(destination), destination, pthreads_get_object_name(source), pthreads_self());
#endif

			if(PTHREADS_IS_STREAMS(destination)) {

				if(PTHREADS_IS_STREAM_CONTEXT(destination)) {
					pthreads_stream_context_free(PTHREADS_FETCH_STREAMS_CONTEXT(destination));
				} else if(PTHREADS_IS_STREAM_WRAPPER(destination)) {
					pthreads_stream_wrapper_free(PTHREADS_FETCH_STREAMS_WRAPPER(destination));
				} else if (PTHREADS_IS_STREAM_BRIGADE(destination)) {
					pthreads_stream_bucket_brigade_free(PTHREADS_FETCH_STREAMS_BRIGADE(destination));
				}
				pthreads_streams_free(destination->store.streams);
			}

			if (PTHREADS_IS_SOCKET(destination)) {
				pthreads_socket_free(destination->store.sock, 0);
			} else {
				pthreads_store_free(destination->store.props);
				if (PTHREADS_IS_WORKER(destination)) {
					pthreads_stack_free(destination->stack);
				}
				free(destination->running);
			}

			pthreads_monitor_free(destination->monitor);
		}

		memcpy(destination, source, sizeof(pthreads_object_t) - sizeof(zend_object));
		
		destination->creator = destCreator;
		destination->scope |= PTHREADS_SCOPE_CONNECTION;

		if (destination->std.properties)
			zend_hash_clean(destination->std.properties);
		
		return SUCCESS;
	} else return FAILURE;
} /* }}} */

/* {{{ */
static inline void pthreads_base_init(pthreads_object_t* base) {	
	zend_property_info *info;
	zval tmp, key;

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

		ZVAL_STR(&key, zend_string_init(prop, plen, 0));
		pthreads_store_write(
			&tmp, &key,
			&base->std.ce->default_properties_table[offset]);
		zval_ptr_dtor(&key);
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

		if (PTHREADS_IS_STREAMS(base)) {
#if PTHREADS_STREAM_DEBUG
	printf("creating stream object std(%p) threaded(%p) type: %s \n", PTHREADS_STD_P(base), base, pthreads_get_object_name(base));
#endif
			base->store.streams = pthreads_streams_alloc();

			if (PTHREADS_IS_STREAM_CONTEXT(base)) {
				PTHREADS_FETCH_STREAMS_CONTEXT(base) = pthreads_stream_context_alloc();
			} else if (PTHREADS_IS_STREAM_WRAPPER(base)) {
				PTHREADS_FETCH_STREAMS_WRAPPER(base) = pthreads_stream_wrapper_alloc();
			} else if (PTHREADS_IS_STREAM_BRIGADE(base)) {
				PTHREADS_FETCH_STREAMS_BRIGADE(base) = pthreads_stream_bucket_brigade_alloc();
			}
		}

		if (PTHREADS_IS_SOCKET(base)) {
			base->store.sock = pthreads_socket_alloc();
		} else {
			base->store.props = pthreads_store_alloc();
			base->running = malloc(sizeof(pthreads_object_t**));

			if (PTHREADS_IS_WORKER(base)) {
				base->stack = pthreads_stack_alloc(base->monitor);
			} else if (PTHREADS_IS_STREAM_BUCKET(base)) {

			}
			pthreads_base_init(base);
		}
	}
} /* }}} */

/* {{{ */
void pthreads_base_free(zend_object *object) {
	pthreads_object_t* base = PTHREADS_FETCH_FROM(object);

	if (PTHREADS_IS_NOT_CONNECTION(base)) {
		if(PTHREADS_IS_STREAMS(base)) {
#if PTHREADS_STREAM_DEBUG
	printf("freeing stream object std(%p) threaded(%p) type(%s) thread(%i) refcount(%i) \n", object, base, pthreads_get_object_name(base), pthreads_self(), pthreads_refcount(base));
#endif
			if(PTHREADS_IS_STREAM(base)) {
				pthreads_stream * stream = PTHREADS_FETCH_STREAMS_STREAM(base);

				if(!PTHREADS_IS_STREAM_CLOSING(stream)) {
					pthreads_stream_close(base, PTHREADS_STREAM_FREE_CLOSE);
				}
				pthreads_stream_free(base);
			} else if(PTHREADS_IS_STREAM_CONTEXT(base) && PTHREADS_FETCH_STREAMS_CONTEXT(base)) {
				pthreads_stream_context_free(PTHREADS_FETCH_STREAMS_CONTEXT(base));
			} else if(PTHREADS_IS_STREAM_FILTER(base) && PTHREADS_FETCH_STREAMS_FILTER(base)) {
				if(pthreads_stream_filter_is_integrated(base)) {
					_pthreads_stream_filter_remove(base, 0);
				}
				pthreads_stream_filter_free(PTHREADS_FETCH_STREAMS_FILTER(base), base);
			} else if(PTHREADS_IS_STREAM_BUCKET(base) && PTHREADS_FETCH_STREAMS_BUCKET(base)) {
				pthreads_stream_bucket_free(PTHREADS_FETCH_STREAMS_BUCKET(base));
			} else if (PTHREADS_IS_STREAM_WRAPPER(base) && PTHREADS_FETCH_STREAMS_WRAPPER(base)) {
				pthreads_stream_wrapper_free(PTHREADS_FETCH_STREAMS_WRAPPER(base));
			} else if (PTHREADS_IS_STREAM_BRIGADE(base) && PTHREADS_FETCH_STREAMS_BRIGADE(base)) {
				pthreads_stream_bucket_brigade_free(PTHREADS_FETCH_STREAMS_BRIGADE(base));
			}
			pthreads_streams_free(base->store.streams);
		}

		if (PTHREADS_IS_SOCKET(base)) {
			pthreads_socket_free(base->store.sock, 1);
		} else {
			if ((PTHREADS_IS_THREAD(base)||PTHREADS_IS_WORKER(base)) &&
				pthreads_monitor_check(base->monitor, PTHREADS_MONITOR_STARTED) &&
				!pthreads_monitor_check(base->monitor, PTHREADS_MONITOR_JOINED)) {
				pthreads_join(base);
			}

			if (pthreads_monitor_lock(base->monitor)) {
				pthreads_store_free(base->store.props);
				if (PTHREADS_IS_WORKER(base)) {
					pthreads_stack_free(base->stack);	
				}
				pthreads_monitor_unlock(base->monitor);
			}

			if (base->running) {
				free(base->running);
			}
		}
		pthreads_monitor_free(base->monitor);
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
	pthreads_routine_arg_t routine;

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

	pthreads_routine_init(&routine, thread);

	switch (pthread_create(&thread->thread, NULL, (void* (*) (void*)) pthreads_routine, (void*)&routine)) {
		case SUCCESS:
			pthreads_routine_wait(&routine);
			return 1;

		case EAGAIN:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, out of resources", thread->std.ce->name->val);
		break;

		default:
			zend_throw_exception_ex(spl_ce_RuntimeException,
				0, "cannot start %s, unknown error", thread->std.ce->name->val);
	}
	
	pthreads_routine_free(&routine);

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

	if (!pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_STARTED)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"%s has not been started",
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

	if (pthreads_monitor_check(object->monitor, PTHREADS_MONITOR_ERROR)) {
		return 0;
	}

	ZVAL_UNDEF(&zresult);

	pthreads_monitor_add(object->monitor, PTHREADS_MONITOR_RUNNING);

	if (work)
		pthreads_store_write(work, &PTHREADS_G(strings).worker, &PTHREADS_ZG(this));

	zend_try {
		if ((run = zend_hash_find_ptr(&connection->std.ce->function_table, PTHREADS_G(strings).run))) {							
			if (run->type == ZEND_USER_FUNCTION) {
				call.fci.size = sizeof(zend_fcall_info);
			    call.fci.retval = &zresult;
				call.fci.object = &connection->std;
				call.fci.no_separation = 1;
#if PHP_VERSION_ID < 70300
				call.fcc.initialized = 1;
#endif
				call.fcc.object = &connection->std;
				call.fcc.calling_scope = connection->std.ce;
				call.fcc.called_scope = connection->std.ce;
				call.fcc.function_handler = run;

				zend_call_function(&call.fci, &call.fcc);
			}
		}
	} zend_catch {
	    pthreads_monitor_add(object->monitor, PTHREADS_MONITOR_ERROR);
	} zend_end_try();

	if (Z_TYPE(zresult) != IS_UNDEF) {
		zval_ptr_dtor(&zresult);
	}

	pthreads_monitor_remove(object->monitor, PTHREADS_MONITOR_RUNNING);

	return 1;
} /* }}} */

/* {{{ */
static void * pthreads_routine(pthreads_routine_arg_t *routine) {
	pthreads_object_t* thread = routine->thread;
	pthreads_monitor_t* ready = routine->ready;
	
	if (pthreads_prepared_startup(thread, ready) == SUCCESS) {
		
		zend_first_try {
			ZVAL_UNDEF(&PTHREADS_ZG(this));
			object_init_ex(&PTHREADS_ZG(this), pthreads_prepared_entry(thread, thread->std.ce));
			pthreads_routine_run_function(thread, PTHREADS_FETCH_FROM(Z_OBJ_P(&PTHREADS_ZG(this))), NULL);

			if (PTHREADS_IS_WORKER(thread)) {
				zval stacked;

				while (pthreads_stack_next(thread->stack, &stacked, thread->running) != PTHREADS_MONITOR_JOINED) {
					zval that;
					pthreads_object_t* work = PTHREADS_FETCH_FROM(Z_OBJ(stacked));
					object_init_ex(&that, pthreads_prepared_entry(thread, work->std.ce));
					pthreads_routine_run_function(work, PTHREADS_FETCH_FROM(Z_OBJ(that)), &that);
					zval_ptr_dtor(&that);
				}
			}

			zval_ptr_dtor(&PTHREADS_ZG(this));
			ZVAL_UNDEF(&PTHREADS_ZG(this));
		} zend_end_try();	
	}

	pthreads_prepared_shutdown();

	pthread_exit(NULL);

#ifdef _WIN32
	return NULL;
#endif
} /* }}} */

#endif

