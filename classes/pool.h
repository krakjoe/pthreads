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
#ifndef HAVE_PTHREADS_CLASS_POOL_H
#define HAVE_PTHREADS_CLASS_POOL_H
PHP_METHOD(Pool, __construct);
PHP_METHOD(Pool, resize);
PHP_METHOD(Pool, submit);
PHP_METHOD(Pool, submitTo);
PHP_METHOD(Pool, collect);
PHP_METHOD(Pool, shutdown);

ZEND_BEGIN_ARG_INFO_EX(Pool___construct, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, class, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, ctor, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_resize, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_submit, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, task, Collectable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_submitTo, 0, 0, 2)
	ZEND_ARG_TYPE_INFO(0, worker, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, task, Collectable, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_collect, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, collector, Closure, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_noargs, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_pool_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_POOL
#	define HAVE_PTHREADS_CLASS_POOL
zend_function_entry pthreads_pool_methods[] = {
	PHP_ME(Pool, __construct, 	Pool___construct, 	ZEND_ACC_PUBLIC)
	PHP_ME(Pool, resize, 		Pool_resize, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, submit, 		Pool_submit, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, submitTo, 		Pool_submitTo, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, collect, 		Pool_collect, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, shutdown, 		Pool_noargs, 		ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto Pool Pool::__construct(integer size, [class worker, [array $ctor]]) 	
	Construct a pool ready to create a maximum of $size workers of class $worker
	$ctor will be used as arguments to constructor when spawning workers */
PHP_METHOD(Pool, __construct) 
{
	zend_long size = 0;
	zend_class_entry *clazz = NULL;
	zval *ctor = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|Ca", &size, &clazz, &ctor) != SUCCESS) {
		return;
	}
	
	if (clazz == NULL) clazz = pthreads_worker_entry;
	
	if (!instanceof_function(clazz, pthreads_worker_entry)) {
		zend_throw_exception_ex(NULL, 0, 
			"The class provided (%s) does not extend Worker", clazz->name);
	}
	
	zend_update_property_long(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"), size);
	zend_update_property_stringl(
		Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("class"), clazz->name->val, clazz->name->len);
	if (ctor)
		zend_update_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("ctor"), ctor);
} /* }}} */

/* {{{ proto void Pool::resize(integer size) 
	Resize the pool to the given number of workers, if the pool size is being reduced 
	then the last workers started will be shutdown until the pool is the requested size */
PHP_METHOD(Pool, resize) {
	zend_long newsize = 0;
	zval *workers = NULL;
	zval *size = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &newsize) != SUCCESS) {
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1, workers);
	size    = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"),    1, size);
	
	if (Z_TYPE_P(workers) == IS_ARRAY &&
		newsize < zend_hash_num_elements(Z_ARRVAL_P(workers))) {
		do {
			zval *worker = NULL;
			zend_long top = zend_hash_num_elements(Z_ARRVAL_P(workers));
			
			if ((worker = zend_hash_index_find(
				Z_ARRVAL_P(workers), top-1))) {
				zend_call_method(
					worker, Z_OBJCE_P(worker), NULL, ZEND_STRL("shutdown"), NULL, 0, NULL, NULL);

			}
			
			zend_hash_index_del(Z_ARRVAL_P(workers), top-1);
		} while (zend_hash_num_elements(Z_ARRVAL_P(workers)) != newsize);
	}
	
	ZVAL_LONG(size, newsize);
} /* }}} */

/* {{{ proto integer Pool::submit(Collectable task) 
	Will submit the given task to the next worker in the pool, by default workers are selected round robin */
PHP_METHOD(Pool, submit) {
	zval *task = NULL;
	zval *last = NULL;
	zval *size = NULL;
	zval *workers = NULL;
	zval worker;
	zval *clazz = NULL;
	zval *ctor = NULL;
	zval *working = NULL;
	zval *selected = NULL;
	
	zend_class_entry *ce = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &task, pthreads_collectable_entry) != SUCCESS) {
		return;
	}

	if (!instanceof_function(Z_OBJCE_P(task), pthreads_threaded_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Threaded objects may be submitted, %s is not Threaded",
			ZSTR_VAL(Z_OBJCE_P(task)->name));
		return;
	}
	
	last = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("last"), 1, last);
	size = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"), 1, size);
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1, workers);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		array_init(workers);
	
	if (Z_LVAL_P(last) >= Z_LVAL_P(size))
		ZVAL_LONG(last, 0);

	if (!(selected = zend_hash_index_find(Z_ARRVAL_P(workers), Z_LVAL_P(last)))) {
		clazz = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("class"), 1, clazz);
		
		if (Z_TYPE_P(clazz) != IS_STRING) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"this Pool has not been initialized properly, Worker class not valid");
			return;
		}
		
		if (!(ce = zend_lookup_class(
			Z_STR_P(clazz)))) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
				"this Pool has not been initialized properly, the Worker class %s could not be found", 
				Z_STRVAL_P(clazz));
			return;
		}
		
		ctor  = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("ctor"), 1, ctor);
		
		object_init_ex(&worker, ce);
		
		{
			zend_class_entry *scope = EG(scope);
			zend_function *constructor = NULL;
			zval retval;

			ZVAL_UNDEF(&retval);
			
			EG(scope) = ce;
			constructor = Z_OBJ_HT(worker)->get_constructor(Z_OBJ(worker));
			EG(scope) = scope;
			
			if (constructor) {
				zend_fcall_info fci;
				zend_fcall_info_cache fcc;
				
				memset(&fci, 0, sizeof(zend_fcall_info));
				memset(&fcc, 0, sizeof(zend_fcall_info_cache));
				
				fci.size = sizeof(zend_fcall_info);
				fci.function_table = EG(function_table);
				fci.object = Z_OBJ(worker);
				fci.retval = &retval;
				fci.no_separation = 1;
				
				fcc.initialized = 1;
				fcc.function_handler = constructor;
				fcc.calling_scope = EG(scope);
				fcc.called_scope = Z_OBJCE(worker);
				fcc.object = Z_OBJ(worker);
				
				if (ctor)
					zend_fcall_info_args(&fci, ctor);
				
				zend_call_function(&fci, &fcc);
				
				if (ctor)
					zend_fcall_info_args_clear(&fci, 1);
				
				if (Z_TYPE(retval) != IS_UNDEF)
					zval_dtor(&retval);
			}
			
			zend_call_method(&worker, Z_OBJCE(worker), NULL, ZEND_STRL("start"), NULL, 0, NULL, NULL);
		}
		
		selected = zend_hash_index_update(
			Z_ARRVAL_P(workers), Z_LVAL_P(last), &worker);
		
	}
	
	zend_call_method(selected, Z_OBJCE_P(selected), NULL, ZEND_STRL("stack"), NULL, 1, task, NULL);
	ZVAL_LONG(return_value, Z_LVAL_P(last));
	Z_LVAL_P(last)++;
} /* }}} */

/* {{{ proto integer Pool::submitTo(integer $worker, Collectable task) 
	Will submit the given task to the specified worker */
PHP_METHOD(Pool, submitTo) {
	zval *task = NULL;
	zval *workers = NULL;
	zend_long worker = 0;
	zval *selected = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "lO", &worker, &task, pthreads_collectable_entry) != SUCCESS) {
		return;
	}

	if (!instanceof_function(Z_OBJCE_P(task), pthreads_threaded_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Threaded objects may be submitted, %s is not Threaded",
			ZSTR_VAL(Z_OBJCE_P(task)->name));
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1, workers);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		array_init(workers);
	
	if ((selected = zend_hash_index_find(Z_ARRVAL_P(workers), worker))) {
		zend_call_method(selected, 
			Z_OBJCE_P(selected), NULL, 
			ZEND_STRL("stack"), NULL, 1, task, NULL);
		ZVAL_LONG(return_value, worker);
	} else {
		zend_throw_exception_ex(NULL, 0, 
			"The selected worker (%ld) does not exist", worker);
	}
} /* }}} */

/* {{{ proto void Pool::collect([callable collector])
	Shall execute the collector on each of the tasks in the working set 
		removing the task if the collector returns positively
		the collector should be a function accepting a single task */
PHP_METHOD(Pool, collect) {
	pthreads_call_t call;
	zval *workers = NULL,
	     *worker = NULL;
	zend_long collectable = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|f", &call.fci, &call.fcc) != SUCCESS) {
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1, workers);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		RETURN_LONG(0);

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(workers), worker) {
		pthreads_object_t *thread = 
			PTHREADS_FETCH_FROM(Z_OBJ_P(worker));
		if (!ZEND_NUM_ARGS())
			PTHREADS_WORKER_COLLECTOR_INIT(call, Z_OBJ_P(worker));
		collectable += pthreads_stack_collect(
			thread->stack, 
			&call, 
			pthreads_worker_collectable_running_function, 
			pthreads_worker_collect_function);
		if (!ZEND_NUM_ARGS())
			PTHREADS_WORKER_COLLECTOR_DTOR(call);
	} ZEND_HASH_FOREACH_END();

	RETURN_LONG(collectable);
} /* }}} */

/* {{{ */
static inline int pthreads_pool_shutdown_worker(zval *worker) {
	zval retval;

	ZVAL_UNDEF(&retval);
	zend_call_method_with_0_params(
		worker, Z_OBJCE_P(worker), NULL, "shutdown", &retval);
	if (Z_TYPE(retval) != IS_UNDEF)
		zval_ptr_dtor(&retval);

	return ZEND_HASH_APPLY_REMOVE;
} /* }}} */

/* {{{ */
static inline void pthreads_pool_shutdown(zval *pool) {
	zval tmp;
	zval *workers = zend_read_property(
		Z_OBJCE_P(pool), pool, ZEND_STRL("workers"), 1, &tmp);
	
	if (Z_TYPE_P(workers) == IS_ARRAY) {
		if (zend_hash_num_elements(Z_ARRVAL_P(workers))) {
			zend_hash_apply(Z_ARRVAL_P(workers), pthreads_pool_shutdown_worker);	
		}

		zend_hash_clean(Z_ARRVAL_P(workers));
	}
} /* }}} */

/* {{{ proto void Pool::shutdown(void)
	Will cause all the workers to finish executing their stacks and shutdown */
PHP_METHOD(Pool, shutdown) {
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	pthreads_pool_shutdown(getThis());
} /* }}} */
#	endif
#endif
