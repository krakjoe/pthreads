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
#ifndef HAVE_PTHREADS_HANDLERS
#define HAVE_PTHREADS_HANDLERS

#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <src/handlers.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#define IN_GET      (1<<0)
#define IN_SET      (1<<1)
#define IN_UNSET    (1<<2)
#define IN_ISSET    (1<<3)

typedef uint32_t zend_guard;
#define pthreads_get_guard(o, m) \
	((Z_TYPE_P(m) == IS_STRING) ? zend_get_property_guard(o, Z_STR_P(m)) : NULL)
/* }}} */

/* {{{ */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D) {
	return pthreads_store_count(PTHREADS_COUNT_PASSTHRU_C);
} /* }}} */

/* {{{ */
int pthreads_count_properties_disallow(PTHREADS_COUNT_PASSTHRU_D) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));

	return -1;
} /* }}} */

/* {{{ */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D) {
	HashTable *table = emalloc(sizeof(HashTable));
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_hash_init(table, 8, NULL, ZVAL_PTR_DTOR, 0);
	*is_temp = 1;

	if (!PTHREADS_IS_SOCKET(threaded)) {
		pthreads_store_tohash(object, table);
	}

	return table;
} /* }}} */

/* {{{ */
HashTable* pthreads_read_properties(PTHREADS_READ_PROPERTIES_PASSTHRU_D) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	pthreads_store_tohash(
		object, threaded->std.properties);
		
	return threaded->std.properties;
} /* }}} */

/* {{{ */
HashTable* pthreads_read_properties_disallow(PTHREADS_READ_PROPERTIES_PASSTHRU_D) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));

	return threaded->std.properties;
} /* }}} */

/* {{{ */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zend_guard *guard = NULL;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	if (Z_OBJCE_P(object)->__get && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_GET)) {
		zend_fcall_info fci = empty_fcall_info;
        zend_fcall_info_cache fcc = empty_fcall_info_cache;
		
		fci.size = sizeof(zend_fcall_info);
		fci.retval = rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
#if PHP_VERSION_ID < 70300
		fcc.initialized = 1;
#endif
		fcc.function_handler = Z_OBJCE_P(object)->__get;
		fcc.object = &threaded->std;
		
		(*guard) |= IN_GET;
		zend_call_function(&fci, &fcc);
		(*guard) &= ~IN_GET;

		zend_fcall_info_args_clear(&fci, 1);
	} else {
		pthreads_store_read(object, member, type, rv);
	}
	
	return rv;
} 

zval* pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D) { return pthreads_read_property(PTHREADS_READ_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
zval * pthreads_read_property_disallow (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));
	
	return &EG(uninitialized_zval);
} 

zval* pthreads_read_dimension_disallow(PTHREADS_READ_DIMENSION_PASSTHRU_D) { return pthreads_read_property_disallow(PTHREADS_READ_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

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
			zend_guard *guard = NULL;
			if ((member && Z_TYPE_P(member) != IS_NULL) && 
				Z_OBJCE_P(object)->__set && 
				(guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_SET)) {
				zend_fcall_info fci = empty_fcall_info;
				zend_fcall_info_cache fcc = empty_fcall_info_cache;
				zval rv;

				ZVAL_UNDEF(&rv);

				fci.size = sizeof(zend_fcall_info);
				fci.retval = &rv;
				fci.object = &threaded->std;
				zend_fcall_info_argn(&fci, 2, member, value);
#if PHP_VERSION_ID < 70300
				fcc.initialized = 1;
#endif
				fcc.function_handler = Z_OBJCE_P(object)->__set;
				fcc.object = &threaded->std;

				(*guard) |= IN_SET;
				zend_call_function(&fci, &fcc);
				(*guard) &= ~IN_SET;

				if (Z_TYPE(rv) != IS_UNDEF)
					zval_dtor(&rv);
				zend_fcall_info_args_clear(&fci, 1);
			} else {
				pthreads_store_write(object, member, value);
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
}

void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) { pthreads_write_property(PTHREADS_WRITE_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
void pthreads_write_property_disallow(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));
}

void pthreads_write_dimension_disallow(PTHREADS_WRITE_DIMENSION_PASSTHRU_D) { pthreads_write_property_disallow(PTHREADS_WRITE_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	int isset = 0;
	zend_guard *guard = NULL;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	cache = NULL;

	if (Z_OBJCE_P(object)->__isset && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_ISSET)) {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		zval rv;

		ZVAL_UNDEF(&rv);

		fci.size = sizeof(zend_fcall_info);
		fci.retval = &rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
#if PHP_VERSION_ID < 70300
		fcc.initialized = 1;
#endif
		fcc.function_handler = Z_OBJCE_P(object)->__isset;
		fcc.object = &threaded->std;

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
		isset = pthreads_store_isset(object, member, has_set_exists);
	}

	return isset;
}
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D) { return pthreads_has_property(PTHREADS_HAS_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
int pthreads_has_property_disallow(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));

	return 0;
}

int pthreads_has_dimension_disallow(PTHREADS_HAS_DIMENSION_PASSTHRU_D) { return pthreads_has_property_disallow(PTHREADS_HAS_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	zend_guard *guard = NULL;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	cache = NULL;

	rebuild_object_properties(&threaded->std);

	if (Z_OBJCE_P(object)->__unset && (guard = pthreads_get_guard(&threaded->std, member)) && !((*guard) & IN_UNSET)) {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;
		zval rv;

		ZVAL_UNDEF(&rv);

		fci.size = sizeof(zend_fcall_info);
		fci.retval = &rv;
		fci.object = &threaded->std;
		zend_fcall_info_argn(&fci, 1, member);
#if PHP_VERSION_ID < 70300
		fcc.initialized = 1;
#endif
		fcc.function_handler = Z_OBJCE_P(object)->__unset;
		fcc.object = &threaded->std;

		(*guard) |= IN_UNSET;
		zend_call_function(&fci, &fcc);
		(*guard) &= ~IN_UNSET;
	
		if (Z_TYPE(rv) != IS_UNDEF) {
			zval_dtor(&rv);
		}
		zend_fcall_info_args_clear(&fci, 1);
	} else {
		pthreads_store_delete(object, member);
	}
}
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) { pthreads_unset_property(PTHREADS_UNSET_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
void pthreads_unset_property_disallow(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s objects are not allowed to have properties",
		ZSTR_VAL(threaded->std.ce->name));
}
void pthreads_unset_dimension_disallow(PTHREADS_UNSET_DIMENSION_PASSTHRU_D) { pthreads_unset_property_disallow(PTHREADS_UNSET_DIMENSION_PASSTHRU_C); }
/* }}} */

/* {{{ */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(from));
	if (PTHREADS_IS_SOCKET(threaded)) {
		if (type == IS_LONG) {
			ZVAL_LONG(to, 
				(int) threaded->options);
			return SUCCESS;
		}
		return FAILURE;
	}

    switch (type) {
        case IS_ARRAY: {
            pthreads_store_tohash(from, Z_ARRVAL_P(to));
            return SUCCESS;
        } break;
    }
    
    return zend_handlers->cast_object(PTHREADS_CAST_PASSTHRU_C);
} /* }}} */

/* {{{ */
zend_object* pthreads_clone_object(PTHREADS_CLONE_PASSTHRU_D)
{
	zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads objects cannot be cloned");
	
	return NULL;
} /* }}} */

/* {{{ */
int pthreads_compare_objects(PTHREADS_COMPARE_PASSTHRU_D) {
	pthreads_object_t *left = PTHREADS_FETCH_FROM(Z_OBJ_P(op1));
	pthreads_object_t *right = PTHREADS_FETCH_FROM(Z_OBJ_P(op2));

	/* comparing property tables is not useful or efficient for threaded objects */
	/* in addition, it might be useful to know if two variables are infact the same physical threaded object */
	if (left->monitor == right->monitor) {
		return 0;
	}

	return 1;
} /* }}} */

static void pthreads_call_getter(zval *object, zval *member, zval *retval) /* {{{ */
{
	zend_class_entry *ce = Z_OBJCE_P(object);
	zend_class_entry *orig_fake_scope = EG(fake_scope);

	EG(fake_scope) = NULL;

	/* __get handler is called with one argument:
	      property name

	   it should return whether the call was successful or not
	*/
	zend_call_method_with_1_params(object, ce, &ce->__get, ZEND_GET_FUNC_NAME, retval, member);

	EG(fake_scope) = orig_fake_scope;
}
/* }}} */

static void pthreads_call_issetter(zval *object, zval *member, zval *retval) /* {{{ */
{
	zend_class_entry *ce = Z_OBJCE_P(object);
	zend_class_entry *orig_fake_scope = EG(fake_scope);

	EG(fake_scope) = NULL;

	/* __isset handler is called with one argument:
	      property name

	   it should return whether the property is set or not
	*/

	if (Z_REFCOUNTED_P(member)) Z_ADDREF_P(member);

	zend_call_method_with_1_params(object, ce, &ce->__isset, ZEND_ISSET_FUNC_NAME, retval, member);

	zval_ptr_dtor(member);

	EG(fake_scope) = orig_fake_scope;
}
/* }}} */

static void pthreads_call_setter(zval *object, zval *member, zval *value) /* {{{ */
{
	zend_class_entry *ce = Z_OBJCE_P(object);
	zend_class_entry *orig_fake_scope = EG(fake_scope);

	EG(fake_scope) = NULL;

	/* __set handler is called with two arguments:
	     property name
	     value to be set
	*/
	zend_call_method_with_2_params(object, ce, &ce->__set, ZEND_SET_FUNC_NAME, NULL, member, value);

	EG(fake_scope) = orig_fake_scope;
}
/* }}} */

static void pthreads_call_unsetter(zval *object, zval *member) /* {{{ */
{
	zend_class_entry *ce = Z_OBJCE_P(object);
	zend_class_entry *orig_fake_scope = EG(fake_scope);

	EG(fake_scope) = NULL;

	/* __unset handler is called with one argument:
	      property name
	*/

	if (Z_REFCOUNTED_P(member)) Z_ADDREF_P(member);

	zend_call_method_with_1_params(object, ce, &ce->__unset, ZEND_UNSET_FUNC_NAME, NULL, member);

	zval_ptr_dtor(member);

	EG(fake_scope) = orig_fake_scope;
}
/* }}} */

/* {{{ */
zval * pthreads_concurrent_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	zend_guard *guard = NULL;
	zend_object *zobj;
	zval tmp_member, tmp_object;
	zval *retval = NULL;
	uint32_t property_offset = ZEND_WRONG_PROPERTY_OFFSET;
	zend_property_info *property_info;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	zobj = Z_OBJ_P(object);

	ZVAL_UNDEF(&tmp_member);
	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_STR(&tmp_member, zval_get_string(member));
		member = &tmp_member;
	}

	/* make zend_get_property_info silent if we have getter - we may want to use it */
	property_info = zend_get_property_info(zobj->ce, Z_STR_P(member), (type == BP_VAR_IS) || (zobj->ce->__get != NULL));

	if(property_info == NULL) {
		property_offset = ZEND_DYNAMIC_PROPERTY_OFFSET;
	} else {
		property_offset = property_info->offset;
	}

	if (EXPECTED(property_info != ZEND_WRONG_PROPERTY_INFO)) {
		if(pthreads_is_property_threadlocal(property_info)) {
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				retval = OBJ_PROP(zobj, property_offset);
				if (EXPECTED(Z_TYPE_P(retval) != IS_UNDEF)) {
					goto exit;
				}
			} else if (EXPECTED(zobj->properties != NULL)) {
				retval = zend_hash_find(zobj->properties, Z_STR_P(member));
				if (EXPECTED(retval)) goto exit;
			}
		} else {
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				pthreads_store_read(object, member, type, rv);
				retval = rv;
				if (EXPECTED(retval)) {
					goto exit;
				}
			} else if (EXPECTED(zobj->properties != NULL)) {
				retval = zend_hash_find(zobj->properties, Z_STR_P(member));
				if (EXPECTED(retval)) {
					pthreads_store_read(object, member, type, rv);
					retval = rv;
					goto exit;
				}
			}
		}

	} else if (UNEXPECTED(EG(exception))) {
		retval = &EG(uninitialized_zval);
		goto exit;
	}

	ZVAL_UNDEF(&tmp_object);

	/* magic isset */
	if ((type == BP_VAR_IS) && zobj->ce->__isset) {
		zval tmp_result;
		guard = pthreads_get_guard(&threaded->std, member);

		if (!((*guard) & IN_ISSET)) {
			if (Z_TYPE(tmp_member) == IS_UNDEF) {
				ZVAL_COPY(&tmp_member, member);
				member = &tmp_member;
			}
			ZVAL_COPY(&tmp_object, object);
			ZVAL_UNDEF(&tmp_result);

			*guard |= IN_ISSET;
			pthreads_call_issetter(&tmp_object, member, &tmp_result);
			*guard &= ~IN_ISSET;

			if (!zend_is_true(&tmp_result)) {
				retval = &EG(uninitialized_zval);
				zval_ptr_dtor(&tmp_object);
				zval_ptr_dtor(&tmp_result);
				goto exit;
			}

			zval_ptr_dtor(&tmp_result);
		}
	}

	/* magic get */
	if (zobj->ce->__get) {
		if(guard == NULL) {
			guard = pthreads_get_guard(&threaded->std, member);
		}

		if (!((*guard) & IN_GET)) {
			/* have getter - try with it! */
			if (Z_TYPE(tmp_object) == IS_UNDEF) {
				ZVAL_COPY(&tmp_object, object);
			}
			*guard |= IN_GET; /* prevent circular getting */
			pthreads_call_getter(&tmp_object, member, rv);
			*guard &= ~IN_GET;

			if (Z_TYPE_P(rv) != IS_UNDEF) {
				retval = rv;
				if (!Z_ISREF_P(rv) &&
					(type == BP_VAR_W || type == BP_VAR_RW  || type == BP_VAR_UNSET)) {
					SEPARATE_ZVAL(rv);
					if (UNEXPECTED(Z_TYPE_P(rv) != IS_OBJECT)) {
						zend_error(E_NOTICE, "Indirect modification of overloaded property %s::$%s has no effect", ZSTR_VAL(zobj->ce->name), Z_STRVAL_P(member));
					}
				}
			} else {
				retval = &EG(uninitialized_zval);
			}
			zval_ptr_dtor(&tmp_object);
			goto exit;
		} else if (Z_STRVAL_P(member)[0] == '\0' && Z_STRLEN_P(member) != 0) {
			//zval_ptr_dtor(&tmp_object);
			zend_throw_error(NULL, "Cannot access property started with '\\0'");
			retval = &EG(uninitialized_zval);
			goto exit;
		}
	}

	zval_ptr_dtor(&tmp_object);

	if ((type != BP_VAR_IS)) {
		zend_error(E_NOTICE,"Undefined property: %s::$%s", ZSTR_VAL(zobj->ce->name), Z_STRVAL_P(member));
	}
	retval = &EG(uninitialized_zval);

exit:
	if (UNEXPECTED(Z_REFCOUNTED(tmp_member))) {
		zval_ptr_dtor(&tmp_member);
	}

	return retval;
}
/* }}} */

/* {{{ */
void pthreads_concurrent_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	zend_object *zobj;
	zval tmp_member;
	zval *variable_ptr;
	uint32_t property_offset = ZEND_WRONG_PROPERTY_OFFSET;
	zend_property_info *property_info;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	zobj = Z_OBJ_P(object);

	ZVAL_UNDEF(&tmp_member);
	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_STR(&tmp_member, zval_get_string(member));
		member = &tmp_member;
	}
	property_info = zend_get_property_info(zobj->ce, Z_STR_P(member), (zobj->ce->__set != NULL));

	if(property_info == NULL) {
		property_offset = ZEND_DYNAMIC_PROPERTY_OFFSET;
	} else {
		property_offset = property_info->offset;
	}

	if (EXPECTED(property_info != ZEND_WRONG_PROPERTY_INFO)) {
		if(pthreads_is_property_threadlocal(property_info)) {
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				variable_ptr = OBJ_PROP(zobj, property_offset);
				if (Z_TYPE_P(variable_ptr) != IS_UNDEF) {
					goto found;
				}
			} else if (EXPECTED(zobj->properties != NULL)) {
				if (UNEXPECTED(GC_REFCOUNT(zobj->properties) > 1)) {
					if (EXPECTED(!(GC_FLAGS(zobj->properties) & IS_ARRAY_IMMUTABLE))) {
						GC_REFCOUNT(zobj->properties)--;
					}
					zobj->properties = zend_array_dup(zobj->properties);
				}
				if ((variable_ptr = zend_hash_find(zobj->properties, Z_STR_P(member))) != NULL) {
found:
					zend_assign_to_variable(variable_ptr, value, IS_CV);
					goto exit;
				}
			}
		} else {
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
					if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
						variable_ptr = OBJ_PROP(zobj, property_offset);
						if (Z_TYPE_P(variable_ptr) != IS_UNDEF) {
							goto found_threaded;
						}
					} else if (EXPECTED(zobj->properties != NULL)) {
						if ((variable_ptr = zend_hash_find(zobj->properties, Z_STR_P(member))) != NULL) {
found_threaded:
							pthreads_store_write(object, member, value);
							goto exit;
						}
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
			goto exit;
		}

	} else if (UNEXPECTED(EG(exception))) {
		goto exit;
	}

	/* magic set */
	if (zobj->ce->__set) {
		zend_guard *guard = pthreads_get_guard(&threaded->std, member);

		if (!((*guard) & IN_SET)) {
			zval tmp_object;

			ZVAL_COPY(&tmp_object, object);
			(*guard) |= IN_SET; /* prevent circular setting */
			pthreads_call_setter(&tmp_object, member, value);
			(*guard) &= ~IN_SET;
			zval_ptr_dtor(&tmp_object);
		} else if (EXPECTED(property_offset != ZEND_WRONG_PROPERTY_OFFSET)) {
			goto write_std_property;
		} else {
			if (Z_STRVAL_P(member)[0] == '\0' && Z_STRLEN_P(member) != 0) {
				zend_throw_error(NULL, "Cannot access property started with '\\0'");
				goto exit;
			}
		}
	} else if (EXPECTED(property_offset != ZEND_WRONG_PROPERTY_OFFSET)) {
		zval tmp;

write_std_property:
		if(pthreads_is_property_threadlocal(property_info)) {
			if (Z_REFCOUNTED_P(value)) {
				if (Z_ISREF_P(value)) {
					/* if we assign referenced variable, we should separate it */
					ZVAL_COPY(&tmp, Z_REFVAL_P(value));
					value = &tmp;
				} else {
					Z_ADDREF_P(value);
				}
			}
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				ZVAL_COPY_VALUE(OBJ_PROP(zobj, property_offset), value);
			} else {
				if (!zobj->properties) {
					rebuild_object_properties(zobj);
				}
				zend_hash_add_new(zobj->properties, Z_STR_P(member), value);
			}
		} else {
			pthreads_store_write(object, member, value);
		}
	}

exit:
	if (UNEXPECTED(Z_REFCOUNTED(tmp_member))) {
		zval_ptr_dtor(&tmp_member);
	}
}
/* }}} */

/* {{{ */
int pthreads_concurrent_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	zend_object *zobj;
	int result;
	zval *value = NULL;
	zval tmp_member;
	uint32_t property_offset;
	zend_property_info *property_info;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	zobj = Z_OBJ_P(object);

	ZVAL_UNDEF(&tmp_member);
	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_STR(&tmp_member, zval_get_string(member));
		member = &tmp_member;
	}

	property_info = zend_get_property_info(zobj->ce, Z_STR_P(member), 1);

	if(property_info == NULL) {
		property_offset = ZEND_DYNAMIC_PROPERTY_OFFSET;
	} else {
		property_offset = property_info->offset;
	}

	if (EXPECTED(property_info != ZEND_WRONG_PROPERTY_INFO)) {
		if(pthreads_is_property_threadlocal(property_info)) {
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				value = OBJ_PROP(zobj, property_offset);
				if (Z_TYPE_P(value) != IS_UNDEF) {
					goto found;
				}
			} else if (EXPECTED(zobj->properties != NULL) &&
					   (value = zend_hash_find(zobj->properties, Z_STR_P(member))) != NULL) {
found:
				switch (has_set_exists) {
					case 0:
						ZVAL_DEREF(value);
						result = (Z_TYPE_P(value) != IS_NULL);
						break;
					default:
						result = zend_is_true(value);
						break;
					case 2:
						result = 1;
						break;
				}
				goto exit;
			}
		} else {
			result = pthreads_store_isset(object, member, has_set_exists);
			goto exit;
		}
	} else if (UNEXPECTED(EG(exception))) {
		result = 0;
		goto exit;
	}

	result = 0;
	if ((has_set_exists != 2) && zobj->ce->__isset) {
		zend_guard *guard = pthreads_get_guard(&threaded->std, member);

		if (!((*guard) & IN_ISSET)) {
			zval rv;
			zval tmp_object;

			/* have issetter - try with it! */
			if (Z_TYPE(tmp_member) == IS_UNDEF) {
				ZVAL_COPY(&tmp_member, member);
				member = &tmp_member;
			}
			ZVAL_COPY(&tmp_object, object);
			(*guard) |= IN_ISSET; /* prevent circular getting */
			pthreads_call_issetter(&tmp_object, member, &rv);
			if (Z_TYPE(rv) != IS_UNDEF) {
				result = zend_is_true(&rv);
				zval_ptr_dtor(&rv);
				if (has_set_exists && result) {
					if (EXPECTED(!EG(exception)) && zobj->ce->__get && !((*guard) & IN_GET)) {
						(*guard) |= IN_GET;
						pthreads_call_getter(&tmp_object, member, &rv);
						(*guard) &= ~IN_GET;
						if (Z_TYPE(rv) != IS_UNDEF) {
							result = i_zend_is_true(&rv);
							zval_ptr_dtor(&rv);
						} else {
							result = 0;
						}
					} else {
						result = 0;
					}
				}
			}
			(*guard) &= ~IN_ISSET;
			zval_ptr_dtor(&tmp_object);
		}
	}

exit:
	if (UNEXPECTED(Z_REFCOUNTED(tmp_member))) {
		zval_ptr_dtor(&tmp_member);
	}
	return result;
}
/* }}} */

/* {{{ */
void pthreads_concurrent_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	zend_object *zobj;
	zval tmp_member;
	uint32_t property_offset;
	zend_property_info *property_info;
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	zobj = Z_OBJ_P(object);

	ZVAL_UNDEF(&tmp_member);
	if (UNEXPECTED(Z_TYPE_P(member) != IS_STRING)) {
		ZVAL_STR(&tmp_member, zval_get_string(member));
		member = &tmp_member;
	}

	property_info = zend_get_property_info(zobj->ce, Z_STR_P(member), (zobj->ce->__unset != NULL));

	if(property_info == NULL) {
		property_offset = ZEND_DYNAMIC_PROPERTY_OFFSET;
	} else {
		property_offset = property_info->offset;
	}

	if (EXPECTED(property_info != ZEND_WRONG_PROPERTY_INFO)) {
		if(pthreads_is_property_threadlocal(property_info)) {
			if (EXPECTED(property_offset != ZEND_DYNAMIC_PROPERTY_OFFSET)) {
				zval *slot = OBJ_PROP(zobj, property_offset);

				if (Z_TYPE_P(slot) != IS_UNDEF) {
					zval_ptr_dtor(slot);
					ZVAL_UNDEF(slot);
					if (zobj->properties) {
						zobj->properties->u.v.flags |= HASH_FLAG_HAS_EMPTY_IND;
					}
					goto exit;
				}
			} else if (EXPECTED(zobj->properties != NULL)) {
				if (UNEXPECTED(GC_REFCOUNT(zobj->properties) > 1)) {
					if (EXPECTED(!(GC_FLAGS(zobj->properties) & IS_ARRAY_IMMUTABLE))) {
						GC_REFCOUNT(zobj->properties)--;
					}
					zobj->properties = zend_array_dup(zobj->properties);
				}
				if (EXPECTED(zend_hash_del(zobj->properties, Z_STR_P(member)) != FAILURE)) {
					goto exit;
				}
			}
		} else {
			pthreads_store_delete(object, member);
			goto exit;
		}

	} else if (UNEXPECTED(EG(exception))) {
		goto exit;
	}

	/* magic unset */
	if (zobj->ce->__unset) {
		zend_guard *guard = pthreads_get_guard(&threaded->std, member);

		if (!((*guard) & IN_UNSET)) {
			zval tmp_object;

			/* have unseter - try with it! */
			ZVAL_COPY(&tmp_object, object);
			(*guard) |= IN_UNSET; /* prevent circular unsetting */
			pthreads_call_unsetter(&tmp_object, member);
			(*guard) &= ~IN_UNSET;
			zval_ptr_dtor(&tmp_object);
		} else {
			if (Z_STRVAL_P(member)[0] == '\0' && Z_STRLEN_P(member) != 0) {
				zend_throw_error(NULL, "Cannot access property started with '\\0'");
				goto exit;
			}
		}
	}

exit:
	if (UNEXPECTED(Z_REFCOUNTED(tmp_member))) {
		zval_ptr_dtor(&tmp_member);
	}
}
/* }}} */

#endif
