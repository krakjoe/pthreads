/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                               		 |
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
#ifndef HAVE_PTHREADS_CLASS_POOL_H
#define HAVE_PTHREADS_CLASS_POOL_H
PHP_METHOD(Pool, __construct);
PHP_METHOD(Pool, resize);
PHP_METHOD(Pool, submit);
PHP_METHOD(Pool, collect);
PHP_METHOD(Pool, shutdown);
PHP_METHOD(Pool, __destruct);

ZEND_BEGIN_ARG_INFO_EX(Pool___construct, 0, 0, 2)
	ZEND_ARG_INFO(0, size)
	ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_resize, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_submit, 0, 0, 1)
	ZEND_ARG_INFO(0, task)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Pool_collect, 0, 0, 1)
	ZEND_ARG_INFO(0, callable)
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
	PHP_ME(Pool, collect, 		Pool_collect, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, shutdown, 		Pool_noargs, 		ZEND_ACC_PUBLIC)
	PHP_ME(Pool, __destruct,	Pool_noargs,		ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto Pool Pool::__construct(integer size, class worker [, array $ctor]) 	
	Construct a pool ready to create a maximum of $size workers of class $worker
	$ctor will be used as arguments to constructor when spawning workers */
PHP_METHOD(Pool, __construct) 
{
	long size = 0;
	zend_class_entry *clazz = NULL;
	zval *ctor = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lC|a", &size, &clazz, &ctor) != SUCCESS) {
		return;
	}
	
	zend_update_property_long(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"), size TSRMLS_CC);
	zend_update_property_stringl(
		Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("class"), clazz->name, clazz->name_length TSRMLS_CC);
	if (ctor)
		zend_update_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("ctor"), ctor TSRMLS_CC);
} /* }}} */

/* {{{ proto void Pool::resize(integer size) 
	Resize the pool to the given number of workers, if the pool size is being reduced 
	then the last workers started will be shutdown until the pool is the requested size */
PHP_METHOD(Pool, resize) {
	long newsize = 0;
	zval *workers = NULL;
	zval *size = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &newsize) != SUCCESS) {
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1 TSRMLS_CC);
	size    = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"),    1 TSRMLS_CC);
	
	if (Z_TYPE_P(workers) == IS_ARRAY &&
		newsize < zend_hash_num_elements(Z_ARRVAL_P(workers))) {
		do {
			zval **worker = NULL;
			long top = zend_hash_num_elements(Z_ARRVAL_P(workers));
			
			if (zend_hash_index_find(
				Z_ARRVAL_P(workers), top-1, (void**)&worker) == SUCCESS) {
				zend_call_method(
					worker, Z_OBJCE_PP(worker), NULL, ZEND_STRL("shutdown"), NULL, 0, NULL, NULL TSRMLS_CC);

			}
			
			zend_hash_index_del(Z_ARRVAL_P(workers), top-1);
		} while (zend_hash_num_elements(Z_ARRVAL_P(workers)) != newsize);
	}
	
	ZVAL_LONG(size, newsize);
} /* }}} */

/* {{{ proto bool Pool::submit(Stackable task) 
	Will submit the given task to the next worker in the pool, by default workers are selected round robin */
PHP_METHOD(Pool, submit) {
	zval *task = NULL;
	zval *last = NULL;
	zval *size = NULL;
	zval *workers = NULL;
	zval *worker = NULL;
	zval *clazz = NULL;
	zval *ctor = NULL;
	zval *work = NULL;
	zval **working = NULL;
	zval **selected = NULL;
	
	zend_class_entry **ce = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &task, pthreads_stackable_entry) != SUCCESS) {
		return;
	}
	
	last = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("last"), 1 TSRMLS_CC);
	size = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("size"), 1 TSRMLS_CC);
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1 TSRMLS_CC);
	work = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("work"), 1 TSRMLS_CC);

	if (Z_TYPE_P(workers) != IS_ARRAY)
		array_init(workers);
	
	if (Z_TYPE_P(work ) != IS_ARRAY)
		array_init(work);

	if (Z_LVAL_P(last) >= Z_LVAL_P(size)) 
		ZVAL_LONG(last, 0);

	if (zend_hash_index_find(Z_ARRVAL_P(workers), Z_LVAL_P(last), (void**)&selected) != SUCCESS) {
		MAKE_STD_ZVAL(worker);
		
		clazz = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("class"), 1 TSRMLS_CC);
		ctor  = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("ctor"), 1 TSRMLS_CC);
		
		zend_lookup_class(
			Z_STRVAL_P(clazz), Z_STRLEN_P(clazz), &ce TSRMLS_CC);
		
		object_init_ex(worker, *ce);
		
		{
			zend_class_entry *scope = EG(scope);
			zend_function *constructor = NULL;
			zval *retval = NULL;
			
			EG(scope) = *ce;
			constructor = Z_OBJ_HT_P(worker)->get_constructor(worker TSRMLS_CC);
			EG(scope) = scope;
			
			if (constructor) {
				zend_fcall_info fci;
				zend_fcall_info_cache fcc;
				
				memset(&fci, 0, sizeof(zend_fcall_info));
				memset(&fcc, 0, sizeof(zend_fcall_info));
				
				fci.size = sizeof(zend_fcall_info);
				fci.function_table = EG(function_table);
				fci.object_ptr = worker;
				fci.retval_ptr_ptr = &retval;
				fci.no_separation = 1;
				
				fcc.initialized = 1;
				fcc.function_handler = constructor;
				fcc.calling_scope = EG(scope);
				fcc.called_scope = Z_OBJCE_P(worker);
				fcc.object_ptr = worker;
				
				if (ctor)
					zend_fcall_info_args(&fci, ctor TSRMLS_CC);
				
				zend_call_function(&fci, &fcc TSRMLS_CC);
				
				if (ctor)
					zend_fcall_info_args_clear(&fci, 1);
				
				if (retval) 
					zval_ptr_dtor(&retval);
			}
			
			zend_call_method(&worker, Z_OBJCE_P(worker), NULL, ZEND_STRL("start"), NULL, 0, NULL, NULL TSRMLS_CC);
		}
		
		zend_hash_index_update(
			Z_ARRVAL_P(workers), Z_LVAL_P(last), 
			(void**)&worker, sizeof(zval*), (void**)&selected);
		Z_OBJ_HT_P(worker)->add_ref(worker TSRMLS_CC);
	}
	
	zend_hash_next_index_insert(
		Z_ARRVAL_P(work), (void**) &task, sizeof(zval*), (void**)&working);
	Z_SET_ISREF_P(task);
	Z_ADDREF_P(task);
	
	zend_call_method(selected, Z_OBJCE_PP(selected), NULL, ZEND_STRL("stack"), &return_value, 1, task, NULL TSRMLS_CC);
	Z_LVAL_P(last)++;
	
} /* }}} */

/* {{{ proto void Pool::collect(Callable collector)
	Shall execute the collector on each of the tasks in the working set 
		removing the task if the collector returns positively
		the collector should be a function accepting a single task */
PHP_METHOD(Pool, collect) {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zval *work = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f", &fci, &fcc) != SUCCESS) {
		return;
	}
	
	work = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("work"), 1 TSRMLS_CC);
	
	if (Z_TYPE_P(work) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(work))) {
		HashPosition position;
		zval **task = NULL;
		
		for (zend_hash_internal_pointer_reset_ex(Z_ARRVAL_P(work), &position);
			zend_hash_get_current_data_ex(Z_ARRVAL_P(work), (void**)&task, &position) == SUCCESS;
			zend_hash_move_forward_ex(Z_ARRVAL_P(work), &position)) {
			zval *remove = NULL;
			zend_ulong index = 0L;
			
			fci.retval_ptr_ptr = &remove;
			
			zend_fcall_info_argn(&fci TSRMLS_CC, 1, task);
			
			if (zend_call_function(&fci, &fcc TSRMLS_CC) == SUCCESS) {
				
				if (remove) {
					convert_to_boolean(remove);
				
					if (Z_BVAL_P(remove)) {
						if (zend_hash_get_current_key_ex(Z_ARRVAL_P(work), NULL, NULL, &index, 0, &position) != HASH_KEY_IS_STRING) {
							zend_hash_index_del(
								Z_ARRVAL_P(work), index);
						}
					}
				}
			}
			
			zend_fcall_info_argn(&fci TSRMLS_CC, 0);
			
			if (remove) 
				zval_ptr_dtor(&remove);
		}
	}
} /* }}} */

/* {{{ */
static inline int pthreads_pool_shutdown(void *data TSRMLS_DC) {
	zval **worker = (zval**) data;
	
	zend_call_method(
		worker, Z_OBJCE_PP(worker), NULL, ZEND_STRL("shutdown"), NULL, 0, NULL, NULL TSRMLS_CC);
	
	return ZEND_HASH_APPLY_REMOVE;
} /* }}} */

/* {{{ proto void Pool::shutdown(void)
	Will cause all the workers to finish executing their stacks and shutdown */
PHP_METHOD(Pool, shutdown) {
	zval *workers = NULL;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1 TSRMLS_CC);
	
	if (Z_TYPE_P(workers) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(workers))) {
		zend_hash_apply(Z_ARRVAL_P(workers), pthreads_pool_shutdown TSRMLS_CC);
	}
	
	zend_hash_clean(Z_ARRVAL_P(workers));
} /* }}} */

/* {{{ proto void Pool::__destruct()
	Will shutdown all workers and destroy all references held to work */
PHP_METHOD(Pool, __destruct) {
	zval *workers = NULL;
	zval *work = NULL;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	workers = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("workers"), 1 TSRMLS_CC);
	work    = zend_read_property(Z_OBJCE_P(getThis()), getThis(), ZEND_STRL("work"),	1 TSRMLS_CC);
	
	if (Z_TYPE_P(workers) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(workers))) {
		zend_hash_apply(Z_ARRVAL_P(workers), pthreads_pool_shutdown TSRMLS_CC);
	}
	
	zend_hash_clean(Z_ARRVAL_P(workers));
	zend_hash_clean(Z_ARRVAL_P(work));
} /* }}} */
#	endif
#endif
