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

#define IN_GET      (1<<0)
#define IN_SET      (1<<1)
#define IN_UNSET    (1<<2)
#define IN_ISSET    (1<<3)

static inline void pthreads_guard_dtor(zval *el) {
	efree(Z_PTR_P(el));
}

static inline zend_long *pthreads_get_guard(zend_object *zobj, zend_string *member) /* {{{ */
{
    HashTable *guards;
    zend_long stub, *guard;
    zval tmp;

    ZEND_ASSERT(GC_FLAGS(zobj) & IS_OBJ_USE_GUARDS);
    if (GC_FLAGS(zobj) & IS_OBJ_HAS_GUARDS) {
        guards = Z_PTR(zobj->properties_table[zobj->ce->default_properties_count]);
        ZEND_ASSERT(guards != NULL);
        if ((guard = (zend_long *)zend_hash_find_ptr(guards, member)) != NULL) {
            return guard;
        }
    } else {
        ALLOC_HASHTABLE(guards);
        zend_hash_init(guards, 8, NULL, pthreads_guard_dtor, 0);
        ZVAL_PTR(&tmp, guards);
        Z_PTR(zobj->properties_table[zobj->ce->default_properties_count]) = guards;
        GC_FLAGS(zobj) |= IS_OBJ_HAS_GUARDS;
    }

    stub = 0;
    return (zend_long *)zend_hash_add_mem(guards, member, &stub, sizeof(zend_ulong));
}
/* }}} */

/* {{{ counts properties of object */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D) {
    return pthreads_store_count(PTHREADS_COUNT_PASSTHRU_C);
} /* }}} */

/* {{{ reads properties from storage for debug only */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D) {
	HashTable *table = emalloc(sizeof(HashTable));
	zend_hash_init(table, 8, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;
	pthreads_store_tohash(
		(PTHREADS_FETCH_FROM(Z_OBJ_P(object)))->store,
		table
	);
	return table;
} /* }}} */

/* {{{ reads properties from storage */
HashTable* pthreads_read_properties(PTHREADS_READ_PROPERTIES_PASSTHRU_D) {
	PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

#if PHP_VERSION_ID > 50399
	rebuild_object_properties(&pobject->std);
#endif

	pthreads_store_tohash(
		pobject->store, 
		pobject->std.properties
	);
			
	return pobject->std.properties;
} /* }}} */

/* {{ reads a property from a thread, wherever it is available */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zval mstring;
	PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	ZVAL_UNDEF(&mstring);
	
	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_STR(&mstring, zval_get_string(member));
		member = &mstring;
		cache = NULL;
	}

	if (Z_TYPE_P(member)==IS_STRING) {
		zend_long *guard = NULL;
		if (Z_OBJCE_P(object)->__get && (guard = pthreads_get_guard(&pthreads->std, Z_STR_P(member))) && !((*guard) & IN_GET)) {
			zend_fcall_info fci = empty_fcall_info;
	        	zend_fcall_info_cache fcc = empty_fcall_info_cache;			
			
			fci.size = sizeof(zend_fcall_info);
			fci.retval = rv;
			fci.object = &pthreads->std;
			zend_fcall_info_argn(&fci, 1, member);
			fcc.initialized = 1;
			fcc.function_handler = Z_OBJCE_P(object)->__get;
			fcc.object = &pthreads->std;
			
			(*guard) |= IN_GET;
			zend_call_function(&fci, &fcc);
			(*guard) &= ~IN_GET;

			zend_fcall_info_args_clear(&fci, 1);		
		} else {
			if (pthreads_store_read(pthreads->store, Z_STR_P(member), rv) != SUCCESS) {
				zend_throw_exception_ex(
				    spl_ce_RuntimeException, 0, 
				    "pthreads failed to read member %s::$%s", 
				    Z_OBJCE_P(object)->name, Z_STRVAL_P(member));
			}
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads detected an attempt to use an unsupported key type %s", Z_OBJCE_P(object)->name);
	}

	if (Z_TYPE(mstring) != IS_UNDEF) {
		zval_ptr_dtor(&mstring);
	}
	
	return rv;
} 

zval* pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D) { return pthreads_read_property(PTHREADS_READ_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ writes a property to a thread in the appropriate way */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zval mstring;
	zend_bool nulled = 0;
	zend_bool locked;
	
	ZVAL_UNDEF(&mstring);	

	if (member == NULL || Z_TYPE_P(member) == IS_NULL) {
	    /* for anonymous members,
	        we acquire the lock and increment a counter */
		pthreads_lock_acquire(pthreads->store->lock, &locked);
		{
			ZVAL_LONG(
				&mstring, pthreads->store->next++);
			member = &mstring;
		}
		pthreads_lock_release(pthreads->store->lock, locked);
	}

	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_STR(&mstring, zval_get_string(member));
		member = &mstring;
		cache = NULL;
	}

	switch(Z_TYPE_P(value)){
		case IS_UNDEF:
		case IS_STRING:
		case IS_LONG:
		case IS_ARRAY:
		case IS_OBJECT:
		case IS_NULL:
		case IS_DOUBLE:
		case IS_RESOURCE:
		case IS_TRUE:
		case IS_FALSE: {
			zend_long *guard = NULL;
			if (Z_OBJCE_P(object)->__set && (guard = pthreads_get_guard(&pthreads->std, Z_STR_P(member))) && !((*guard) & IN_SET)) {
				zend_fcall_info fci = empty_fcall_info;
				zend_fcall_info_cache fcc = empty_fcall_info_cache;			
				zval rv;

				ZVAL_UNDEF(&rv);

				fci.size = sizeof(zend_fcall_info);
				fci.retval = &rv;
				fci.object = &pthreads->std;
				zend_fcall_info_argn(&fci, 2, member, value);
				fcc.initialized = 1;
				fcc.function_handler = Z_OBJCE_P(object)->__set;
				fcc.object = &pthreads->std;

				(*guard) |= IN_SET;
				zend_call_function(&fci, &fcc);
				(*guard) &= ~IN_SET;

				if (Z_TYPE(rv) != IS_UNDEF)
					zval_dtor(&rv);
				zend_fcall_info_args_clear(&fci, 1);
			} else if (pthreads_store_write(pthreads->store, Z_STR_P(member), value) != SUCCESS) {
				zend_throw_exception_ex(
					spl_ce_RuntimeException, 0, 
					"pthreads failed to write member %s::$%s", 
					ZSTR_VAL(Z_OBJCE_P(object)->name), Z_STRVAL_P(member));
			}
		} break;
	
		default: {
			zend_throw_exception_ex(
				spl_ce_RuntimeException, 0,
				"pthreads detected an attempt to use unsupported data (%s) for %s::$%s", 
				zend_get_type_by_const(Z_TYPE_P(value)), 
				ZSTR_VAL(Z_OBJCE_P(object)->name), Z_STRVAL_P(member));
		}
	}

	if (Z_TYPE(mstring) != IS_UNDEF) {
		zval_ptr_dtor(&mstring);
	}
}

void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) { pthreads_write_property(PTHREADS_WRITE_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ check if a thread has a property set, wherever it is available */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	int isset = 0;
	zval mstring;

	PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	ZVAL_UNDEF(&mstring);

	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_STR(&mstring, zval_get_string(member));
		member = &mstring;
		cache = NULL;
	}

	if (Z_TYPE_P(member) == IS_STRING) {
		zend_long *guard = NULL;
		if (Z_OBJCE_P(object)->__isset && (guard = pthreads_get_guard(&pthreads->std, Z_STR_P(member))) && !((*guard) & IN_ISSET)) {
			zend_fcall_info fci = empty_fcall_info;
			zend_fcall_info_cache fcc = empty_fcall_info_cache;			
			zval rv;

			ZVAL_UNDEF(&rv);

			fci.size = sizeof(zend_fcall_info);
			fci.retval = &rv;
			fci.object = &pthreads->std;
			zend_fcall_info_argn(&fci, 1, member);
			fcc.initialized = 1;
			fcc.function_handler = Z_OBJCE_P(object)->__isset;
			fcc.object = &pthreads->std;

			(*guard) |= IN_ISSET;
			zend_call_function(&fci, &fcc);
			(*guard) &= ~IN_ISSET;
		
			if (Z_TYPE(rv) != IS_UNDEF) {
				isset = 
					zend_is_true(&rv);
				zval_dtor(&rv);
			}
			zend_fcall_info_args_clear(&fci, 1);
		} else {
			isset = pthreads_store_isset(
		        	pthreads->store, 
		        	Z_STR_P(member), 
		        	has_set_exists);
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads detected an attempt to use an unsupported key type %s", Z_OBJCE_P(object)->name);
	}

	if (Z_TYPE(mstring) != IS_UNDEF) {
		zval_ptr_dtor(&mstring);
	}

	return isset;
}
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D) { return pthreads_has_property(PTHREADS_HAS_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ unset an object property */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	zval mstring;
	PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	ZVAL_UNDEF(&mstring);
	
	if (Z_TYPE_P(member) != IS_STRING) {
		ZVAL_STR(&mstring, zval_get_string(member));
		member = &mstring;
		cache = NULL;
	}

	if (Z_TYPE_P(member) == IS_STRING) {
		zend_long *guard = NULL;	
		if (Z_OBJCE_P(object)->__unset && (guard = pthreads_get_guard(&pthreads->std, Z_STR_P(member))) && !((*guard) & IN_UNSET)) {
			zend_fcall_info fci = empty_fcall_info;
			zend_fcall_info_cache fcc = empty_fcall_info_cache;			
			zval rv;

			ZVAL_UNDEF(&rv);

			fci.size = sizeof(zend_fcall_info);
			fci.retval = &rv;
			fci.object = &pthreads->std;
			zend_fcall_info_argn(&fci, 1, member);
			fcc.initialized = 1;
			fcc.function_handler = Z_OBJCE_P(object)->__unset;
			fcc.object = &pthreads->std;

			(*guard) |= IN_UNSET;
			zend_call_function(&fci, &fcc);
			(*guard) &= ~IN_UNSET;
		
			if (Z_TYPE(rv) != IS_UNDEF) {
				zval_dtor(&rv);
			}
			zend_fcall_info_args_clear(&fci, 1);
		} else if (pthreads_store_delete(pthreads->store, Z_STR_P(member)) != SUCCESS){
			zend_throw_exception_ex(
				spl_ce_RuntimeException, 0, 
				"pthreads failed to delete member %s::$%s", 
				Z_OBJCE_P(object)->name, Z_STRVAL_P(member));
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads detected an attempt to use an unsupported key type %s", Z_OBJCE_P(object)->name);
	}
	
	if (Z_TYPE(mstring) != IS_UNDEF) {
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
	zend_string *lcname;
	int access = 0;
	PTHREAD thread = PTHREADS_FETCH_FROM(*object);
	
	if (thread) {
		switch((access=pthreads_modifiers_get(thread->modifiers, method))){
			case ZEND_ACC_PRIVATE:
			case ZEND_ACC_PROTECTED:
				scope = thread->std.ce;
				lcname =  zend_string_tolower(method);
				
				if (!(call = zend_hash_find_ptr(&scope->function_table, lcname))) {
					callable = (zend_function*) emalloc(sizeof(zend_function));
					callable->type = ZEND_OVERLOADED_FUNCTION;
					callable->common.function_name = call->common.function_name;
					callable->common.fn_flags = ZEND_ACC_PUBLIC;
					callable->common.scope = scope;
					callable->common.arg_info = call->common.arg_info;
					callable->common.num_args = call->common.num_args;
					callable->common.required_num_args = call->common.required_num_args;

					zend_string_release(lcname);
					return callable;
				}
				zend_string_release(lcname);
			break;

			default:
				call = zend_handlers->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
		}
	} else call = zend_handlers->get_method(PTHREADS_GET_METHOD_PASSTHRU_C);
	
	return call;
} /* }}} */


/* {{{ pthreads_call_method */
int pthreads_call_method(PTHREADS_CALL_METHOD_PASSTHRU_D) {
	zval 					*argv = NULL, zmethod, zresult;
	zend_function 			*call = NULL;
	zend_fcall_info 		info;
	zend_fcall_info_cache	cache;
	zend_class_entry		*scope;
	int 					called = -1, argc = ZEND_NUM_ARGS(), access = ZEND_ACC_PUBLIC, mlength = 0;
	zend_string				*lcname;
	zend_bool				unprotect;
	PTHREAD                                 thread = PTHREADS_FETCH_FROM(object);
	
	if (thread) {
		switch((access=pthreads_modifiers_get(thread->modifiers, method))){
			case ZEND_ACC_PRIVATE:
			case ZEND_ACC_PROTECTED: {
				scope = Z_OBJCE_P(getThis());
				
				/*
				* Stop invalid private method calls
				*/
				if (access == ZEND_ACC_PRIVATE && !PTHREADS_IN_THREAD(thread)) {
					zend_throw_exception_ex(
						spl_ce_RuntimeException, 0, 
						"pthreads detected an attempt to call private "
						"method %s::%s from outside the threading context", 
						scope->name, method);
					return FAILURE;
				}
				
				

				/*
				* Get arguments from stack
				*/
				if (ZEND_NUM_ARGS()) 
				{
					argv = safe_emalloc(sizeof(zval), argc, 0);
					if (argv) {
						zend_get_parameters_array_ex(argc, argv);
					}
				}

				lcname =  zend_string_tolower(method);
				
				if ((call = zend_hash_find_ptr(&scope->function_table, lcname))) {
					if (access != ZEND_ACC_PROTECTED || pthreads_modifiers_protect(thread->modifiers, method, &unprotect)) {
						ZVAL_STR(&zmethod, method);
						ZVAL_UNDEF(&zresult);

						info.size = sizeof(info);
						info.object = &thread->std;
						info.function_name = zmethod;
						info.retval = &zresult;
						info.no_separation = 1;
						info.symbol_table = NULL;
						info.param_count = argc;
						info.params = argv;
					
						cache.initialized = 1;
						cache.function_handler = call;
						cache.calling_scope = EG(scope);
						cache.called_scope = scope;
						cache.object = &thread->std;
					
						if ((called=zend_call_function(&info, &cache))!=SUCCESS) {
							zend_throw_exception_ex(
								spl_ce_RuntimeException, 0, 
								"pthreads has experienced an internal error while "
								"calling %s method %s::%s and cannot continue", 
								(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
								scope->name, method);
							called = FAILURE;
						} else {
							if (Z_TYPE(zresult) != IS_UNDEF) {
							    ZVAL_ZVAL(return_value, &zresult, 1, 1);
							} else ZVAL_UNDEF(return_value);
						}
					
						if (access == ZEND_ACC_PROTECTED) {
							pthreads_modifiers_unprotect(thread->modifiers, method, unprotect);
						}
					} else {
						zend_throw_exception_ex(
							spl_ce_RuntimeException, 0, 
							"pthreads has experienced an internal error while "
							"calling %s method %s::%s and cannot continue", 
							(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
							scope->name, method);
						called = FAILURE;
					}
				} else {
					zend_throw_exception_ex(
						spl_ce_RuntimeException, 0, 
						"pthreads has experienced an internal error while "
						"finding %s method %s::%s and cannot continue", 
						(access == ZEND_ACC_PROTECTED) ? "protected" : "private",
						scope->name, method);
					called = FAILURE;
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
	
	switch (called) {
		case -1: 
			return zend_handlers->call_method(PTHREADS_CALL_METHOD_PASSTHRU_C);
			
		default: return called;
	}
	
} /* }}} */

/* {{{ pthreads_cast_object */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D) {
    switch (type) {
        case IS_ARRAY: {
            pthreads_store_tohash(
                (PTHREADS_FETCH_FROM(Z_OBJ_P(from)))->store, Z_ARRVAL_P(to));
            return SUCCESS;
        } break;
    }
    
    return zend_handlers->cast_object(PTHREADS_CAST_PASSTHRU_C);
} /* }}} */

/* {{{ clone object handler */
zend_object* pthreads_clone_object(PTHREADS_CLONE_PASSTHRU_D)
{
	zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads objects cannot be cloned");
	
	return NULL;
} /* }}} */

#endif
