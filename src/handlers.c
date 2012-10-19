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

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

#define PTHREADS_IMPORT_PROPERTY(t, m, l, p) \
	if (t->std.properties != NULL && zend_hash_find(t->std.properties, Z_STRVAL_P(m), Z_STRLEN_P(m)+1, (void**)&l)==SUCCESS) {\
		switch(Z_TYPE_PP(l)){\
			case IS_LONG:\
			case IS_BOOL:\
			case IS_STRING:\
			case IS_NULL:\
			case IS_ARRAY:\
				serial = pthreads_serialize(*l TSRMLS_CC);\
				if (serial) {\
					if ((p = pthreads_unserialize(serial TSRMLS_CC)) != NULL)\
						Z_SET_REFCOUNT_P(p, 0);\
					free(serial);\
				}\
			break;\
			\
			case IS_OBJECT:\
			case IS_RESOURCE:\
				zend_error(E_WARNING, "pthreads detected an attempt to fetch an unsupported symbol (%s)", Z_STRVAL_P(m));\
			break;\
		}\
	}\

/* {{ read_property */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	zval **lookup = NULL, *prop = NULL;
	char *serial = NULL;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		
		if (acquire == SUCCESS || acquire == EDEADLK) {
			if (!PTHREADS_IN_THREAD(thread)) {
				if (pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING)) {
					PTHREADS_IMPORT_PROPERTY(thread->sig, member, lookup, prop);
				}
			}
			
			if (prop == NULL)
				prop = zsh->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return prop;
} /* }}} */

/* {{{ write_property will never attempt to write any other object other than itself */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ has_property */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	int result = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			result = zsh->has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return result;
} /* }}} */

/* {{{ unset_property will never attempt to write any object other than itself */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ pthreads_get_method will attempt to apply pthreads specific modifiers */
zend_function * pthreads_get_method(PTHREADS_GET_METHOD_PASSTHRU_D) {
	zend_function *call;
	zend_function *callable;
	zend_op_array *ops;
	char *lcname;
	
	PTHREAD thread = PTHREADS_FETCH_FROM(*pobject);
	if (thread) {
		switch(pthreads_modifiers_get(thread->modifiers, method TSRMLS_CC)){
			case ZEND_ACC_PRIVATE:
			case ZEND_ACC_PROTECTED:
				lcname =  (char*) calloc(1, methodl+1);
				zend_str_tolower_copy(lcname, method, methodl);
				if (zend_hash_find(&Z_OBJCE_PP(pobject)->function_table, lcname, methodl+1, (void**)&call)==SUCCESS) {
					callable = (zend_function*) emalloc(sizeof(zend_function));
					callable->type = ZEND_OVERLOADED_FUNCTION;
					callable->common.function_name = call->common.function_name;
					callable->common.fn_flags = ZEND_ACC_PUBLIC;
					callable->common.scope = EG(called_scope);
					callable->common.arg_info = call->common.arg_info;
					callable->common.num_args = call->common.num_args;
					callable->common.required_num_args = call->common.required_num_args;
#if PHP_VERSION_ID < 50400
					callable->common.pass_rest_by_reference = call->common.pass_rest_by_reference;
					callable->common.return_reference = call->common.return_reference;
#endif					
					destroy_zend_function(call TSRMLS_CC);
					function_add_ref(call);
				}
				free(lcname);
			return callable;
			
			default: call = zsh->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
		}
		
	} else call = zsh->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
	
	return call;
} /* }}} */

/* {{{ pthreads_call_method */
int pthreads_call_method(PTHREADS_CALL_METHOD_PASSTHRU_D) {
	zval 					***argv, zmethod;
	zend_function 			*call = NULL;
	zend_fcall_info 		info;
	zend_fcall_info_cache	cache;
	int 					called = -1, acquire = 0, argc = ZEND_NUM_ARGS(), access = ZEND_ACC_PUBLIC, mlength = 0;
	char					*lcname;
	
	if (getThis()) {
		PTHREAD thread = PTHREADS_FETCH;
		if (thread) {
			switch((access=pthreads_modifiers_get(thread->modifiers, method TSRMLS_CC))){
				case ZEND_ACC_PRIVATE:
				case ZEND_ACC_PROTECTED: {
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
							
					if (access == ZEND_ACC_PRIVATE && !PTHREADS_IN_THREAD(thread)) {
						zend_error_noreturn(
							E_WARNING, 
							"pthreads detected an attempt to call private method %s::%s from outside the threading context", 
							Z_OBJCE_P(getThis())->name,
							method
						);
						if (argc) {
							printf("freeing %d args\n", argc);
							efree(argv);
						}
						return FAILURE;
					}
					
					mlength = strlen(method);
					lcname =  calloc(1, mlength+1);
					zend_str_tolower_copy(lcname, method, mlength);
					
					if (zend_hash_find(&Z_OBJCE_P(getThis())->function_table, lcname, mlength+1, (void**)&call)==SUCCESS) {
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
									info.retval_ptr_ptr = return_value_ptr;
									info.no_separation = 1;
									info.symbol_table = NULL;
									info.param_count = argc;
									info.params = argv;
									
									cache.initialized = 1;
									cache.function_handler = call;
									cache.calling_scope = Z_OBJCE_P(getThis());
									cache.called_scope = Z_OBJCE_P(getThis());
									cache.object_ptr = getThis();
									
									call->type = ZEND_USER_FUNCTION;
									if ((called=zend_call_function(&info, &cache TSRMLS_CC))!=SUCCESS) {
										zend_error_noreturn(
											E_ERROR, 
											"pthreads has experienced an internal error while calling %s method %s::%s and cannot continue", 
											(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
											Z_OBJCE_P(getThis())->name,
											method
										);
									}
									call->type = ZEND_OVERLOADED_FUNCTION;
									
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
										Z_OBJCE_P(getThis())->name,
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
								Z_OBJCE_P(getThis())->name,
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
			return zsh->call_method(PTHREADS_CALL_METHOD_PASSTHRU_C);
			
		default: return called;
	}
	
} /* }}} */

#endif
