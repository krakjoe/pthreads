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

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

/* {{{ reads properties from storage for debug only */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D) {
	HashTable *table = emalloc(sizeof(HashTable));
	zend_hash_init(table, 8, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;
	pthreads_store_tohash(
		(PTHREADS_FETCH_FROM(object))->store, 
		table TSRMLS_CC
	);
	return table;
} /* }}} */

/* {{ reads a property from a thread, wherever it is available */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zval *value = NULL, *mstring = NULL;
	PTHREAD pthreads = PTHREADS_FETCH_FROM(object);
	
	if (Z_TYPE_P(member) != IS_STRING) {
		ALLOC_ZVAL(mstring);
		*mstring = *member;
		zval_copy_ctor(
			mstring
		);
		INIT_PZVAL(mstring);
		convert_to_string(mstring);
		member = mstring;
#if PHP_VERSION_ID > 50399
		key = NULL;
#endif
	}

	if (Z_TYPE_P(member)==IS_STRING) {
		if (pthreads_store_read(
			pthreads->store, 
			Z_STRVAL_P(member), Z_STRLEN_P(member), 
			&value TSRMLS_CC
		)!=SUCCESS) {	
			value = zend_handlers->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);
		}
	} else value = zend_handlers->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);

	if (mstring != NULL) {
		zval_ptr_dtor(&mstring);
	}
	
	return value;
} 
zval * pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D) { return pthreads_read_property(PTHREADS_READ_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ writes a property to a thread in the appropriate way */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREAD pthreads = PTHREADS_FETCH_FROM(object);
	zval *mstring = NULL;

	if (member == NULL) {
		zend_error(E_WARNING, "pthreads cannot write an anonymous index, identify the member explicitly");
		return;
	}

	if (Z_TYPE_P(member) != IS_STRING) {
		ALLOC_ZVAL(mstring);
		*mstring = *member;
		zval_copy_ctor(
			mstring
		);
		INIT_PZVAL(mstring);
		convert_to_string(mstring);
		member = mstring;
#if PHP_VERSION_ID > 50399
		key = NULL;
#endif
	}

	if (Z_TYPE_P(member)==IS_STRING) {
		switch(Z_TYPE_P(value)){
			case IS_STRING:
			case IS_LONG:
			case IS_ARRAY:
			case IS_OBJECT:
			case IS_NULL:
			case IS_DOUBLE:
			case IS_RESOURCE:
			case IS_BOOL: {
				if (pthreads_store_write(pthreads->store, Z_STRVAL_P(member), Z_STRLEN_P(member), &value TSRMLS_CC)!=SUCCESS){
					zend_error_noreturn(
						E_WARNING, 
						"pthreads failed to write member %s::$%s", 
						Z_OBJCE_P(object)->name, Z_STRVAL_P(member)
					);
				} else {
					zend_handlers->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
				}
			} break;
			
			default: zend_handlers->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
		}
	} else zend_handlers->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);

	if (mstring != NULL) {
		zval_ptr_dtor(&mstring);
	}
} 
void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) { pthreads_write_property(PTHREADS_WRITE_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ check if a thread has a property set, wherever it is available */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	int isset = 0;
	zval *mstring = NULL;

	PTHREAD pthreads = PTHREADS_FETCH_FROM(object);

	if (Z_TYPE_P(member) != IS_STRING) {
		ALLOC_ZVAL(mstring);
		*mstring = *member;
		zval_copy_ctor(
			mstring
		);
		INIT_PZVAL(mstring);
		convert_to_string(mstring);
		member = mstring;
#if PHP_VERSION_ID > 50399
		key = NULL;
#endif
	}

	if (!(isset = pthreads_store_isset(
		pthreads->store, 
		Z_STRVAL_P(member), Z_STRLEN_P(member), 
		has_set_exists TSRMLS_CC
	))) {
		isset = zend_handlers->has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_C);
	}
	
	if (mstring != NULL) {
		zval_ptr_dtor(&mstring);
	}

	return isset;
} 
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D) { return pthreads_has_property(PTHREADS_HAS_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ unset an object property */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	zval *mstring = NULL;
	PTHREAD pthreads = PTHREADS_FETCH_FROM(object);

	if (Z_TYPE_P(member) != IS_STRING) {
		ALLOC_ZVAL(mstring);
		*mstring = *member;
		zval_copy_ctor(
			mstring
		);
		INIT_PZVAL(mstring);
		convert_to_string(mstring);
		member = mstring;
#if PHP_VERSION_ID > 50399
		key = NULL;
#endif
	}

	if (pthreads_store_delete(pthreads->store, Z_STRVAL_P(member), Z_STRLEN_P(member) TSRMLS_CC)!=SUCCESS){
		zend_error_noreturn(
			E_WARNING, 
			"pthreads has experienced an internal error while deleting %s::$%s", 
			Z_OBJCE_P(object)->name, Z_STRVAL_P(member)
		);
	}
	zend_handlers->unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_C);

	if (mstring != NULL) {
		zval_ptr_dtor(&mstring);
	}
} 
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) { pthreads_unset_property(PTHREADS_UNSET_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ pthreads_get_method will attempt to apply pthreads specific modifiers */
zend_function * pthreads_get_method(PTHREADS_GET_METHOD_PASSTHRU_D) {
	zend_class_entry *scope;
	zend_function *call;
	zend_function *callable;
	char *lcname;
	int access = 0;
	PTHREAD thread = PTHREADS_FETCH_FROM(*pobject);
	
	if (thread) {
		switch((access=pthreads_modifiers_get(thread->modifiers, method TSRMLS_CC))){
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
	int 					called = -1, argc = ZEND_NUM_ARGS(), access = ZEND_ACC_PUBLIC, mlength = 0;
	char					*lcname;
	zend_bool				unprotect;
	
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
								if (access != ZEND_ACC_PROTECTED || pthreads_modifiers_protect(thread->modifiers, method, &unprotect TSRMLS_CC)) {
								
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
									cache.calling_scope = EG(scope);
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
										{
											zend_op_array *ops = (zend_op_array*) call;
										
											if (ops) {
												if (ops->run_time_cache) {
													efree(ops->run_time_cache);
													ops->run_time_cache = NULL;
												}
											}
										}
#endif
										if (!return_value_used) {
											zval_ptr_dtor(&zresult);
										} else {
											ZVAL_ZVAL(return_value, zresult, 1, 1);
										}
									}
									
									if (access == ZEND_ACC_PROTECTED) {
										pthreads_modifiers_unprotect(thread->modifiers, method, unprotect TSRMLS_CC);
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
