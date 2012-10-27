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
#ifndef HAVE_PTHREADS_HANDLERS
#define HAVE_PTHREADS_HANDLERS

#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <src/handlers.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#ifndef HAVE_PTHREADS_SERIAL_H
#	include <src/serial.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

/* {{ reads a property from a thread, wherever it is available */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	int acquire = 0;
	zval *prop = NULL;
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);

		if (acquire == SUCCESS || acquire == EDEADLK) {
			pthreads_serial line;
			
			if(pthreads_serial_contains(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), &line TSRMLS_CC)) {
				if (pthreads_serial_read(line, Z_STRVAL_P(member), Z_STRLEN_P(member), &prop TSRMLS_CC)!=SUCCESS) {
					zend_error_noreturn(E_WARNING, "pthreads has experienced an internal error while reading %s::$%s (%lu)", Z_OBJCE_P(object)->name, Z_STRVAL_P(member), thread->tid);
				} else {
					zend_hash_update(
						Z_OBJPROP_P(object), 
						Z_STRVAL_P(member), Z_STRLEN_P(member)+1, 
						(void**)&prop, sizeof(zval*), 
						NULL
					);
					Z_SET_REFCOUNT_P(prop, 1);
				}
			} else prop = zend_handlers->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error_noreturn(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return prop;
} /* }}} */

/* {{{ writes a property to a thread in the appropriate way */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	int acquire = 0;
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			switch(Z_TYPE_P(value)){
				case IS_STRING:
				case IS_LONG:
				case IS_ARRAY:
				case IS_OBJECT:
				case IS_NULL:
				case IS_DOUBLE:
				case IS_BOOL: {
					if (pthreads_serial_write(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), &value TSRMLS_CC)!=SUCCESS){
						zend_error_noreturn(E_WARNING, "pthreads failed to write member %s::$%s (%lu)", Z_OBJCE_P(object)->name, Z_STRVAL_P(member), thread->tid);
					}
				} break;
				
				default: zend_handlers->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
			}
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ check if a thread has a property set, wherever it is available */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0, result = -1;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			pthreads_serial line;
			
			if (pthreads_serial_contains(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), &line TSRMLS_CC)) {
				result = pthreads_serial_isset(line, Z_STRVAL_P(member), Z_STRLEN_P(member), has_set_exists TSRMLS_CC);
			} else result = zend_handlers->has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return result;
} /* }}} */

/* {{{ unset an object property */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			pthreads_serial line;
			
			if (pthreads_serial_contains(thread->store, Z_STRVAL_P(member), Z_STRLEN_P(member), &line TSRMLS_CC)) {
				if (pthreads_serial_delete(line, Z_STRVAL_P(member), Z_STRLEN_P(member) TSRMLS_CC)!=SUCCESS){
					zend_error_noreturn(E_WARNING, "pthreads has experienced an internal error while deleting %s from %s (%lu)", Z_STRVAL_P(member), Z_OBJCE_P(object)->name, thread->tid);
				}
			}
			
			zend_handlers->unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ pthreads_get_method will attempt to apply pthreads specific modifiers */
zend_function * pthreads_get_method(PTHREADS_GET_METHOD_PASSTHRU_D) {
	zend_class_entry *scope;
	zend_function *call;
	zend_function *callable;
	char *lcname;
	
	PTHREAD thread = PTHREADS_FETCH_FROM(*pobject);
	if (thread) {
		switch(pthreads_modifiers_get(thread->modifiers, method TSRMLS_CC)){
			case ZEND_ACC_PRIVATE:
			case ZEND_ACC_PROTECTED:
				scope = Z_OBJCE_PP(pobject);
				lcname =  (char*) calloc(1, methodl+1);
				zend_str_tolower_copy(lcname, method, methodl);
				if (zend_hash_find(&scope->function_table, lcname, methodl+1, (void**)&call)==SUCCESS) {
					callable = (zend_function*) emalloc(sizeof(zend_function));
					callable->type = ZEND_OVERLOADED_FUNCTION;
					callable->common.function_name = call->common.function_name;
					callable->common.fn_flags = ZEND_ACC_PUBLIC;
					callable->common.scope = scope;
					callable->common.arg_info = call->common.arg_info;
					callable->common.num_args = call->common.num_args;
					callable->common.required_num_args = call->common.required_num_args;
#if PHP_VERSION_ID < 50400
					callable->common.pass_rest_by_reference = call->common.pass_rest_by_reference;
					callable->common.return_reference = call->common.return_reference;
#endif
				}
				free(lcname);
			return callable;
			
			default: call = zend_handlers->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
		}
		
	} else call = zend_handlers->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
	
	return call;
} /* }}} */


/* {{{ pthreads_call_method */
int pthreads_call_method(PTHREADS_CALL_METHOD_PASSTHRU_D) {
	zval 					***argv = NULL, zmethod, *zresult;
	zend_function 			*call = NULL;
	zend_fcall_info 		info;
	zend_fcall_info_cache	cache;
	zend_class_entry		*scope;
	int 					called = -1, acquire = 0, argc = ZEND_NUM_ARGS(), access = ZEND_ACC_PUBLIC, mlength = 0;
	char					*lcname;
	
	if (getThis()) {
		PTHREAD thread = PTHREADS_FETCH;
		if (thread) {
			switch((access=pthreads_modifiers_get(thread->modifiers, method TSRMLS_CC))){
				case ZEND_ACC_PRIVATE:
				case ZEND_ACC_PROTECTED: {
					scope = Z_OBJCE_P(getThis());
					
					/*
					* Stop invalid private method calls
					*/
					if (access == ZEND_ACC_PRIVATE && !PTHREADS_IN_THREAD(thread)) {
						zend_error_noreturn(
							E_ERROR, 
							"pthreads detected an attempt to call private method %s::%s from outside the threading context", 
							scope->name,
							method
						);
						return FAILURE;
					}
					
					/*
					* Get arguments from stack
					*/
					if (ZEND_NUM_ARGS()) 
					{
						argv = safe_emalloc(sizeof(zval **), argc, 0);
						if (argv) {
							zend_get_parameters_array_ex(argc, argv);
						}
					}
							
					mlength = strlen(method);
					lcname =  calloc(1, mlength+1);
					zend_str_tolower_copy(lcname, method, mlength);
					
					if (zend_hash_find(&scope->function_table, lcname, mlength+1, (void**)&call)==SUCCESS) {
						if (call) {
							/*
							* Make protected method call
							*/
							{
								if (access == ZEND_ACC_PROTECTED) {
									acquire = pthreads_modifiers_protect(thread->modifiers, method TSRMLS_CC);
								} else acquire = EDEADLK;
								
								if (acquire == SUCCESS || acquire == EDEADLK) {
								
									ZVAL_STRINGL(&zmethod, method, strlen(method), 0);
									
									info.size = sizeof(info);
									info.object_ptr = getThis();
									info.function_name = &zmethod;
									info.retval_ptr_ptr = &zresult;
									info.no_separation = 1;
									info.symbol_table = NULL;
									info.param_count = argc;
									info.params = argv;
									
									cache.initialized = 1;
									cache.function_handler = call;
									cache.calling_scope = EG(called_scope);
									cache.called_scope = scope;
									cache.object_ptr = getThis();
									
									if ((called=zend_call_function(&info, &cache TSRMLS_CC))!=SUCCESS) {
										zend_error_noreturn(
											E_ERROR, 
											"pthreads has experienced an internal error while calling %s method %s::%s and cannot continue", 
											(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
											scope->name,
											method
										);
									} else {
#if PHP_VERSION_ID > 50399				
										zend_op_array *cops = &call->op_array;
										if (cops->run_time_cache) {
											efree(cops->run_time_cache);
											cops->run_time_cache = NULL;
										}
#endif								
										if (!return_value_used) {
											zval_ptr_dtor(&zresult);
										} else {
											ZVAL_ZVAL(return_value, zresult, 1, 1);
										}
									}
									
									if (access == ZEND_ACC_PROTECTED) {
										if (acquire != EDEADLK) {
											pthreads_modifiers_unprotect(thread->modifiers, method TSRMLS_CC);
										}
									}
								} else {
									zend_error_noreturn(
										E_ERROR, 
										"pthreads has experienced an internal error while calling %s method %s::%s and cannot continue", 
										(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
										scope->name,
										method
									);
									called = FAILURE;
								}
							}
						} else {
							zend_error_noreturn(
								E_ERROR, 
								"pthreads has experienced an internal error while finding %s method %s::%s and cannot continue", 
								(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
								scope->name,
								method
							);
							called = FAILURE;
						}
					}
					/*
					* Free unstacked arguments
					*/
					if (argc) {
						efree(argv);
					}
					free(lcname);
					return called;
				} break;
			}
		}
	}
	
	switch (called) {
		case -1: 
			return zend_handlers->call_method(PTHREADS_CALL_METHOD_PASSTHRU_C);
			
		default: return called;
	}
	
} /* }}} */

#endif
