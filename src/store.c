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
#ifndef HAVE_PTHREADS_STORE
#define HAVE_PTHREADS_STORE

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

#ifndef HAVE_PTHREADS_COPY_H
#	include <src/copy.h>
#endif

typedef struct _pthreads_storage {
	zend_uchar 	type;
	size_t 	length;
	zend_bool 	exists;
	union {
	    zend_long   lval;
	    double     dval;
	} simple;
	void    	*data;
} pthreads_storage;

#define PTHREADS_STORAGE_EMPTY {0, 0, 0, 0, NULL}

/* {{{ */
static pthreads_storage* pthreads_store_create(zval *pzval, zend_bool complex);
static int pthreads_store_convert(pthreads_storage *storage, zval *pzval);
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex);
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength);
static void pthreads_store_storage_dtor (pthreads_storage *element);
/* }}} */

/* {{{ */
static inline void pthreads_store_storage_table_dtor (zval *element) {
	pthreads_store_storage_dtor(Z_PTR_P(element));
} /* }}} */

/* {{{ */
pthreads_store_t* pthreads_store_alloc() {
	pthreads_store_t *store = (pthreads_store_t*) calloc(1, sizeof(pthreads_store_t));

	if (store) {
		zend_hash_init(
			store, 8, NULL, 
			(dtor_func_t) pthreads_store_storage_table_dtor, 1);
	}

	return store;
} /* }}} */

void pthreads_store_sync(zval *object) { /* {{{ */
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_ulong idx;
	zend_string *name;

	rebuild_object_properties(&threaded->std);

	ZEND_HASH_FOREACH_KEY(threaded->std.properties, idx, name) {
		if (!name) {
			if (!zend_hash_index_exists(threaded->store.props, idx))
				zend_hash_index_del(threaded->std.properties, idx);
		} else {
			if (!zend_hash_exists(threaded->store.props, name))
				zend_hash_del(threaded->std.properties, name);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

static inline zend_bool pthreads_store_coerce(HashTable *table, zval *key, zval *member) {
	if (!key || Z_TYPE_P(key) == IS_NULL) {
		ZVAL_LONG(member, zend_hash_next_free_element(table));
		return 0;
	}

	switch (Z_TYPE_P(key)) {
		case IS_STRING:
		case IS_LONG:
			ZVAL_ZVAL(member, key, 0, 0);
			return 0;

		default:
			ZVAL_STR(member, zval_get_string(key));
			return 1;
	}
}

/* {{{ */
static inline zend_bool pthreads_store_is_immutable(zval *object, zval *key) {	
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_storage *storage;

	if (IS_PTHREADS_VOLATILE(object)) {
		return 0;
	}

	if (Z_TYPE_P(key) == IS_LONG) {
		storage = zend_hash_index_find_ptr(threaded->store.props, Z_LVAL_P(key));
	} else storage = zend_hash_find_ptr(threaded->store.props, Z_STR_P(key));

	if ((storage) && (storage->type == IS_PTHREADS)) {
		if (Z_TYPE_P(key) == IS_LONG) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Threaded members previously set to Threaded objects are immutable, cannot overwrite %ld",
			Z_LVAL_P(key));	
		} else zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Threaded members previously set to Threaded objects are immutable, cannot overwrite %s",
			Z_STRVAL_P(key));
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ */
int pthreads_store_delete(zval *object, zval *key) {
	int result = FAILURE;
	zval member, *property = NULL;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_bool coerced = pthreads_store_coerce(threaded->store.props, key, &member);	

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(threaded->monitor)) {
		if (!pthreads_store_is_immutable(object, &member)) {
			if (Z_TYPE_P(key) == IS_LONG) {
				result = zend_hash_index_del(threaded->store.props, Z_LVAL(member));
			} else result = zend_hash_del(threaded->store.props, Z_STR(member));
		}
		pthreads_monitor_unlock(threaded->monitor);
	} else result = FAILURE;
	
	if (result == SUCCESS) {
		if (Z_TYPE(member) == IS_LONG) {
			zend_hash_index_del(threaded->std.properties, Z_LVAL(member));
		} else zend_hash_del(threaded->std.properties, Z_STR(member));
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return result;
}
/* }}} */

/* {{{ */
zend_bool pthreads_store_isset(zval *object, zval *key, int has_set_exists) {
	zend_bool isset = 0;
	zval member;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_bool coerced = pthreads_store_coerce(threaded->store.props, key, &member);

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage;

		if (Z_TYPE(member) == IS_LONG) {
			storage = zend_hash_index_find_ptr(threaded->store.props, Z_LVAL(member));
		} else storage = zend_hash_find_ptr(threaded->store.props, Z_STR(member));

		isset = storage != NULL;

		if (has_set_exists && storage) {
		    switch (storage->type) {
				case IS_LONG:
				case IS_TRUE:
				case IS_FALSE:
					if (storage->simple.lval == 0)
					isset = 0;
				break;
		
				case IS_ARRAY:
					if (storage->exists == 0)
					isset = 0;
				break;
		
				case IS_STRING: switch (storage->length) {
					case 0:
					isset = 0;
					break;
					
					case 1:
					if (memcmp(storage->data, "0", 1) == SUCCESS)
						isset = 0;
					break;
				} break;
		
				case IS_DOUBLE:
					if (storage->simple.dval == 0.0)
					isset = 0;
				break;
					
				case IS_NULL:
					isset = 0;
				break;
		    }
		} else if (isset) {
			switch (storage->type) {
			    case IS_NULL:
				isset = 0;
			    break;
			}
		}
		pthreads_monitor_unlock(threaded->monitor);
	}

	if (coerced) 
		zval_ptr_dtor(&member);

	return isset;
} /* }}} */

/* {{{ */
int pthreads_store_read(zval *object, zval *key, int type, zval *read) {
	int result = FAILURE;
	zval member, *property = NULL;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_bool coerced = pthreads_store_coerce(threaded->store.props, key, &member);

	rebuild_object_properties(&threaded->std);

	if (Z_TYPE(member) == IS_LONG) {
		property = zend_hash_index_find(threaded->std.properties, Z_LVAL(member));
	} else property = zend_hash_find(threaded->std.properties, Z_STR(member));

	if (property && IS_PTHREADS_VOLATILE(object)) {
		if (pthreads_monitor_lock(threaded->monitor)) {
			pthreads_storage *storage;

			if (Z_TYPE(member) == IS_LONG) {
				storage = zend_hash_index_find_ptr(threaded->store.props, Z_LVAL(member));
			} else storage = zend_hash_find_ptr(threaded->store.props, Z_STR(member));

			if (storage && storage->type == IS_PTHREADS) {
				pthreads_object_t* threadedStorage = PTHREADS_FETCH_FROM(storage->data);
				pthreads_object_t *threadedProperty = PTHREADS_FETCH_FROM(Z_OBJ_P(property));

				if (threadedStorage->monitor != threadedProperty->monitor) {
					property = NULL;
				}
			} else {
				property = NULL;
			}
			pthreads_monitor_unlock(threaded->monitor);
		}
	}

	if (property && IS_PTHREADS_OBJECT(property)) {
		ZVAL_COPY(read, property);
		if (coerced) {
			zval_ptr_dtor(&member);
		}
		return SUCCESS;
	}

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage;

		/* synchronize property stores */
		pthreads_store_sync(object);

		if (Z_TYPE(member) == IS_LONG) {
			storage = zend_hash_index_find_ptr(threaded->store.props, Z_LVAL(member));
		} else storage = zend_hash_find_ptr(threaded->store.props, Z_STR(member));
		
		if (storage) {
			result = pthreads_store_convert(storage, read);
		}
		pthreads_monitor_unlock(threaded->monitor);
	}

	if (result != SUCCESS) {
		ZVAL_NULL(read);
	} else {
		if (IS_PTHREADS_OBJECT(read)) {
			rebuild_object_properties(&threaded->std);
			if (Z_TYPE(member) == IS_LONG) {
				zend_hash_index_update(threaded->std.properties, Z_LVAL(member), read);
			} else zend_hash_update(threaded->std.properties, Z_STR(member), read);
			Z_ADDREF_P(read);
		}
	}

	if (coerced)
		zval_ptr_dtor(&member);

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_write(zval *object, zval *key, zval *write) {
	int result = FAILURE;
	pthreads_storage *storage;
	zval vol, member, *property = NULL, *read = NULL;
	pthreads_object_t *threaded = 
		PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_bool coerced = 0;

	if (Z_TYPE_P(write) == IS_ARRAY) {
		if (!pthreads_check_opline_ex(EG(current_execute_data), -1, ZEND_CAST, IS_ARRAY) &&
			!pthreads_check_opline_ex(EG(current_execute_data), -2, ZEND_CAST, IS_ARRAY)) {
			/* coerce arrays into volatile objects unless explicitly cast as array */
			object_init_ex(
				&vol, pthreads_volatile_entry);
			pthreads_store_merge(&vol, write, 1);
			/* this will be addref'd when caching the object */
			Z_SET_REFCOUNT(vol, 0);
			write = &vol;
		}
	}

	if (Z_TYPE_P(write) == IS_OBJECT) {
		/* when we copy out in another context, we want properties table
			to be initialized */
		rebuild_object_properties(Z_OBJ_P(write));
	}

	storage = pthreads_store_create(write, 1);

	if (pthreads_monitor_lock(threaded->monitor)) {
		coerced = pthreads_store_coerce(threaded->store.props, key, &member);
		if (!pthreads_store_is_immutable(object, &member)) {
			if (Z_TYPE(member) == IS_LONG) {
				if (zend_hash_index_update_ptr(threaded->store.props, Z_LVAL(member), storage))
					result = SUCCESS;
			} else {
				/* we can't use global strings here */
				zend_string *keyed = zend_string_dup(Z_STR(member), 1);

				if (zend_hash_update_ptr(threaded->store.props, keyed, storage)) {
					result = SUCCESS;
				}
				zend_string_release(keyed);
			}
		}
		pthreads_monitor_unlock(threaded->monitor);
	}

	if (result != SUCCESS) {
		pthreads_store_storage_dtor(storage);
	} else {
		if (IS_PTHREADS_OBJECT(write) || IS_PTHREADS_CLOSURE(write)) {
			/*
				This could be a volatile object, but, we don't want to break
				normal refcounting, we'll read the reference only at volatile objects
			*/
			rebuild_object_properties(&threaded->std);
			
			if(IS_PTHREADS_VOLATILE(object)) {
				pthreads_store_sync(object);
			}

			if (Z_TYPE(member) == IS_LONG) {
				zend_hash_index_update(threaded->std.properties, Z_LVAL(member), write);
			} else {
				zend_string *keyed = zend_string_dup(Z_STR(member), 1);
				if (zend_hash_update(
					threaded->std.properties, keyed, write)) {
					result = SUCCESS;
				}
				zend_string_release(keyed);
			}
			Z_ADDREF_P(write);
		}
	}
	
	if (coerced)
		zval_ptr_dtor(&member);
	
	return result;
} /* }}} */

/* {{{ */
int pthreads_store_separate(zval * pzval, zval *separated, zend_bool complex) {
	int result = FAILURE;
	pthreads_storage *storage;
	
	if (Z_TYPE_P(pzval) != IS_NULL) {
		storage = pthreads_store_create(pzval, complex);
		result = pthreads_store_convert(storage, separated);
		pthreads_store_storage_dtor(storage);
	} else ZVAL_NULL(separated);

	return result;
} /* }}} */

/* {{{ */
void pthreads_store_separate_zval(zval *zv) {
	pthreads_storage *storage;
	zval in = *zv;

	if (Z_TYPE(in) != IS_NULL) {
        	storage = pthreads_store_create(&in, 1);
		pthreads_store_convert(storage, zv);
	    pthreads_store_storage_dtor(storage);
	}
} /* }}} */

/* {{{ */
int pthreads_store_count(zval *object, zend_long *count) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		(*count) = zend_hash_num_elements(threaded->store.props);
		pthreads_monitor_unlock(threaded->monitor);
	} else (*count) = 0L;
   
	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_shift(zval *object, zval *member) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(threaded->monitor)) {
		zval key;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_reset_ex(threaded->store.props, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(threaded->store.props, &position))) {
			zend_hash_get_current_key_zval_ex(threaded->store.props, &key, &position);
			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, member);
				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_del(threaded->store.props, Z_LVAL(key));
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				} else {
					zend_hash_del(threaded->store.props, Z_STR(key));
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
			}
		} else ZVAL_NULL(member);
		pthreads_monitor_unlock(threaded->monitor);

		return SUCCESS;
	}
	
    return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_chunk(zval *object, zend_long size, zend_bool preserve, zval *chunk) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(threaded->monitor)) {
		HashPosition position;
		pthreads_storage *storage;

		array_init(chunk);
		zend_hash_internal_pointer_reset_ex(threaded->store.props, &position);
		while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
			(storage = zend_hash_get_current_data_ptr_ex(threaded->store.props, &position))) {
			zval key, zv;

			zend_hash_get_current_key_zval_ex(threaded->store.props, &key, &position);		

			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, &zv);
				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_update(
						Z_ARRVAL_P(chunk), Z_LVAL(key), &zv);
					zend_hash_index_del(threaded->store.props, Z_LVAL(key));
					zend_hash_index_del(threaded->std.properties, Z_LVAL(key));
				} else {
					zend_hash_update(
						Z_ARRVAL_P(chunk), Z_STR(key), &zv);
					zend_hash_del(threaded->store.props, Z_STR(key));
					zend_hash_del(threaded->std.properties, Z_STR(key));
				}
			} else break;

			zend_hash_internal_pointer_reset_ex(threaded->store.props, &position);
		}
		pthreads_monitor_unlock(threaded->monitor);

		return SUCCESS;
	}

    return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_pop(zval *object, zval *member) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);
   
	if (pthreads_monitor_lock(threaded->monitor)) {
		zval key;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_end_ex(threaded->store.props, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(threaded->store.props, &position))) {
			zend_hash_get_current_key_zval_ex(threaded->store.props, &key, &position);

			if (!pthreads_store_is_immutable(object, &key)) {
				pthreads_store_convert(storage, member);

				if (Z_TYPE(key) == IS_LONG) {
					zend_hash_index_del(
						threaded->store.props, Z_LVAL(key));
					zend_hash_index_del(
						threaded->std.properties, Z_LVAL(key));	
				} else {
					zend_hash_del(
						threaded->store.props, Z_STR(key));	
					zend_hash_del(
						threaded->std.properties, Z_STR(key));			
				}
			}
		} else ZVAL_NULL(member);

		pthreads_monitor_unlock(threaded->monitor);

		return SUCCESS;
	}
   
   return FAILURE;
} /* }}} */

/* {{{ */
void pthreads_store_tohash(zval *object, HashTable *hash) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	rebuild_object_properties(&threaded->std);

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_string *name = NULL;
		zend_ulong idx;
		pthreads_storage *storage;

		pthreads_store_sync(object);

		ZEND_HASH_FOREACH_KEY_PTR(threaded->store.props, idx, name, storage) {
			zval pzval;
			zend_string *rename;

			/* don't overwrite pthreads objects for non volatile objects */
			if (!IS_PTHREADS_VOLATILE(object) && storage->type == IS_PTHREADS) {
				if (!name) {
					if (zend_hash_index_exists(hash, idx))
						continue;
				} else {
					if (zend_hash_exists(hash, name))
						continue;
				}
			}

			if (pthreads_store_convert(storage, &pzval)!=SUCCESS) {
				continue;
			}

			if (!name) {
				if (!zend_hash_index_update(hash, idx, &pzval)) {
					zval_ptr_dtor(&pzval);
				}
			} else {
				rename = zend_string_init(name->val, name->len, 0);
				if (!zend_hash_update(hash, rename, &pzval))
					zval_ptr_dtor(&pzval);
				zend_string_release(rename);
			}
		} ZEND_HASH_FOREACH_END();

		pthreads_monitor_unlock(threaded->monitor);
	}
} /* }}} */

/* {{{ */
void pthreads_store_free(pthreads_store_t *store){
	zend_hash_destroy(store);
	free(store);
} /* }}} */

/* {{{ */
static pthreads_storage* pthreads_store_create(zval *unstore, zend_bool complex){					
	pthreads_storage *storage = NULL;

	if (Z_TYPE_P(unstore) == IS_INDIRECT)
		return pthreads_store_create(Z_INDIRECT_P(unstore), complex);
	if (Z_TYPE_P(unstore) == IS_REFERENCE)
		return pthreads_store_create(&Z_REF_P(unstore)->val, complex);

	storage = (pthreads_storage*) calloc(1, sizeof(pthreads_storage));
	
	switch((storage->type = Z_TYPE_P(unstore))){
		case IS_NULL: /* do nothing */ break;
		case IS_TRUE: storage->simple.lval = 1; break;
		case IS_FALSE: storage->simple.lval = 0; break;
		case IS_DOUBLE: storage->simple.dval = Z_DVAL_P(unstore); break;
		case IS_LONG: storage->simple.lval = Z_LVAL_P(unstore); break;

		case IS_STRING: if ((storage->length = Z_STRLEN_P(unstore))) {
			storage->data = 
				(char*) malloc(storage->length+1);
			memcpy(storage->data, Z_STRVAL_P(unstore), storage->length);
			((char *)storage->data)[storage->length] = 0;
		} break;

		case IS_RESOURCE: {
			if (complex) {
				pthreads_resource resource = malloc(sizeof(*resource));
				if (resource) {
					resource->original = Z_RES_P(unstore);
					resource->ls = TSRMLS_CACHE;
					
					storage->data = resource;
					Z_ADDREF_P(unstore);
				}
			} else storage->type = IS_NULL;
		} break;
		
		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(unstore), zend_ce_closure)) {
				const zend_function *def = 
				    zend_get_closure_method_def(unstore);
				storage->type = IS_CLOSURE;
				storage->data = 
				    (zend_function*) malloc(sizeof(zend_op_array));
				memcpy(storage->data, def, sizeof(zend_op_array));
				break;
			}

			if (instanceof_function(Z_OBJCE_P(unstore), pthreads_threaded_entry)) {
				storage->type = IS_PTHREADS;
				storage->data = Z_OBJ_P(unstore);
				break;
			}

			if (!complex) {
				storage->type = IS_NULL;
				break;
			}

		/* break intentionally omitted */
		case IS_ARRAY: if (pthreads_store_tostring(unstore, (char**) &storage->data, &storage->length, complex)==SUCCESS) {
			if (Z_TYPE_P(unstore) == IS_ARRAY)
				storage->exists = zend_hash_num_elements(Z_ARRVAL_P(unstore));
		} break;

	}
	return storage;
}
/* }}} */

/* {{{ */
static int pthreads_store_convert(pthreads_storage *storage, zval *pzval){
	int result = SUCCESS;

	switch(storage->type) {
		case IS_NULL: ZVAL_NULL(pzval); break;

		case IS_STRING:
			if (storage->data && storage->length) {
				ZVAL_STRINGL(pzval, (char*)storage->data, storage->length);
			} else ZVAL_EMPTY_STRING(pzval);
		break;
		
		case IS_FALSE:
		case IS_TRUE: ZVAL_BOOL(pzval, storage->simple.lval); break;

		case IS_LONG: ZVAL_LONG(pzval, storage->simple.lval); break;
		case IS_DOUBLE: ZVAL_DOUBLE(pzval, storage->simple.dval); break;
		case IS_RESOURCE: {	
			pthreads_resource stored = (pthreads_resource) storage->data;

			if (stored->ls != TSRMLS_CACHE) {
				zval *search = NULL;
				zend_ulong index = 0;
				zend_string *name = NULL;
				zend_resource *resource, *found = NULL;

				ZEND_HASH_FOREACH_KEY_VAL(&EG(regular_list), index, name, search) {
					resource = Z_RES_P(search);
					if (resource->ptr == stored->original->ptr) {
						found = resource;
						break;
					}
				} ZEND_HASH_FOREACH_END();

				if (!found) {
					ZVAL_RES(pzval, stored->original);
					if (zend_hash_next_index_insert(&EG(regular_list), pzval)) {
					    pthreads_resources_keep(stored);
					} else ZVAL_NULL(pzval);
					Z_ADDREF_P(pzval);
				} else ZVAL_COPY(pzval, search);
			} else {
				ZVAL_RES(pzval, stored->original);
				Z_ADDREF_P(pzval);
			}
		} break;
		
		case IS_CLOSURE: {
			char *name;
			size_t name_len;
			zend_function *closure = pthreads_copy_function((zend_function*) storage->data);

			zend_create_closure(pzval, closure, zend_get_executed_scope(), closure->common.scope, NULL);

			name_len = spprintf(&name, 0, "Closure@%p", zend_get_closure_method_def(pzval));

			if (!zend_hash_str_update_ptr(EG(function_table), name, name_len, closure)) {
				result = FAILURE;
				zval_dtor(pzval);
			} else result = SUCCESS;
			efree(name);
		} break;

		case IS_PTHREADS: {
			pthreads_object_t* threaded = PTHREADS_FETCH_FROM(storage->data);

			if (pthreads_check_opline_ex(EG(current_execute_data), 1, ZEND_CAST, IS_OBJECT)) {
				ZVAL_OBJ(pzval, &threaded->std);
				Z_ADDREF_P(pzval);
				break;
			}

			if (!pthreads_globals_object_connect((zend_ulong)threaded, NULL, pzval)) {
				zend_throw_exception_ex(
					spl_ce_RuntimeException, 0,
					"pthreads detected an attempt to connect to an object which has already been destroyed");
				result = FAILURE;
			}
		} break;

		case IS_OBJECT:
		case IS_ARRAY:
			result = pthreads_store_tozval(pzval, (char*) storage->data, storage->length);
		break;

		default: ZVAL_NULL(pzval);
	}

	if (result == FAILURE) {
	    ZVAL_NULL(pzval);
	}

	return result;
}
/* }}} */

/* {{{ */
static inline int pthreads_store_remove_complex(zval *pzval) {
	switch (Z_TYPE_P(pzval)) {
		case IS_ARRAY: {
			HashTable *tmp = zend_array_dup(Z_ARRVAL_P(pzval));

			zend_hash_apply(tmp, pthreads_store_remove_complex);

			zval_ptr_dtor(pzval);

			ZVAL_ARR(pzval, tmp);
		} break;

		case IS_OBJECT:
			if (instanceof_function(Z_OBJCE_P(pzval), pthreads_threaded_entry))
				break;

		case IS_RESOURCE:
			return ZEND_HASH_APPLY_REMOVE; 
	}

	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex) {
	int result = FAILURE;
	if (pzval && (Z_TYPE_P(pzval) != IS_NULL)) {
		smart_str smart;
		HashTable *tmp = NULL;
		zval ztmp;

		memset(&smart, 0, sizeof(smart_str));

		if (Z_TYPE_P(pzval) == IS_ARRAY && !complex) {
			tmp = zend_array_dup(Z_ARRVAL_P(pzval));

			ZVAL_ARR(&ztmp, tmp);
			pzval = &ztmp;
			
			zend_hash_apply(tmp, pthreads_store_remove_complex);
		}

		if ((Z_TYPE_P(pzval) != IS_OBJECT) ||
			(Z_OBJCE_P(pzval)->serialize != zend_class_serialize_deny)) {
			php_serialize_data_t vars;

			PHP_VAR_SERIALIZE_INIT(vars);
			php_var_serialize(&smart, pzval, &vars);
			PHP_VAR_SERIALIZE_DESTROY(vars);

			if (EG(exception)) {
				smart_str_free(&smart);

				if (tmp) {
					zval_dtor(&ztmp);
				}

				*pstring = NULL;
				*slength = 0;
				return FAILURE;			
			}
		}

		if (tmp) {
			zval_dtor(&ztmp);
		}

		if (smart.s) {
			*slength = smart.s->len;
			if (*slength) {
				*pstring = malloc(*slength+1);
				if (*pstring) {
					memcpy(
						(char*) *pstring, (const void*) smart.s->val, smart.s->len
					);
					(*pstring)[*slength] = 0;
					result = SUCCESS;
				}
			} else *pstring = NULL;
		}
		
		smart_str_free(&smart);	
	} else {
	    *slength = 0;
	    *pstring = NULL;
	}
	return result;
} /* }}} */

/* {{{ */
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength) {
	int result = SUCCESS;
	
	if (pstring) {
		const unsigned char* pointer = (const unsigned char*) pstring;

		if (pointer) {
			php_unserialize_data_t vars;

			PHP_VAR_UNSERIALIZE_INIT(vars);
			if (!php_var_unserialize(pzval, &pointer, pointer+slength, &vars)) {
				result = FAILURE;
			}
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		} else result = FAILURE;
	} else result = FAILURE;
	
	return result;
} /* }}} */

/* {{{ */
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite) {
    if (Z_TYPE_P(from) != IS_ARRAY && 
        Z_TYPE_P(from) != IS_OBJECT) {
        return FAILURE;
    }

    switch (Z_TYPE_P(from)) {
        case IS_OBJECT: {
            if (IS_PTHREADS_OBJECT(from)) {
                pthreads_object_t* threaded[2] = {PTHREADS_FETCH_FROM(Z_OBJ_P(destination)), PTHREADS_FETCH_FROM(Z_OBJ_P(from))};
                
                if (pthreads_monitor_lock(threaded[0]->monitor)) {
                    if (pthreads_monitor_lock(threaded[1]->monitor)) {
                        HashPosition position;
                        pthreads_storage *storage;
                        HashTable *tables[2] = {threaded[0]->store.props, threaded[1]->store.props};
						zval key;

                        for (zend_hash_internal_pointer_reset_ex(tables[1], &position);
                             (storage = zend_hash_get_current_data_ptr_ex(tables[1], &position));
                             zend_hash_move_forward_ex(tables[1], &position)) {
							zend_hash_get_current_key_zval_ex(tables[1], &key, &position);

                            if (!overwrite) {
								if (Z_TYPE(key) == IS_LONG) {
									if (zend_hash_index_exists(tables[0], Z_LVAL(key)))
										continue;
								} else {
									if (zend_hash_exists(tables[0], Z_STR(key)))
										continue;		
								}
                            }

							if (pthreads_store_is_immutable(destination, &key)) {
								break;
							}
							
                            if (storage->type != IS_RESOURCE) {
                                pthreads_storage *copy = malloc(sizeof(pthreads_storage));

								memcpy(copy, storage, sizeof(pthreads_storage));

                                switch (copy->type) {
                                    case IS_STRING:
                                    case IS_OBJECT:
			        				case IS_ARRAY: if (storage->length) {
                                        copy->data = (char*) malloc(copy->length+1);
										if (!copy->data) {
											break;
										}
										memcpy(copy->data, (const void*) storage->data, copy->length);
										((char *)copy->data)[copy->length] = 0;
                                	} break;
                                }

                                if (Z_TYPE(key) == IS_LONG) {
									zend_hash_index_update_ptr(tables[0], Z_LVAL(key), copy);
								} else zend_hash_update_ptr(tables[0], Z_STR(key), copy);
                            }
                        }

                        pthreads_monitor_unlock(threaded[1]->monitor);
                    }
                    
                    pthreads_monitor_unlock(threaded[0]->monitor);
                    
                    return SUCCESS;
                    
                } else return FAILURE;
            }
        }
        
        /* fall through on purpose to handle normal objects and arrays */
        
        default: {
           pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(destination));
           
           if (pthreads_monitor_lock(threaded->monitor)) {
               HashPosition position;
               zval *pzval;
               int32_t index = 0;
               HashTable *table = (Z_TYPE_P(from) == IS_ARRAY) ? Z_ARRVAL_P(from) : Z_OBJPROP_P(from);
               
               for (zend_hash_internal_pointer_reset_ex(table, &position);
                    (pzval = zend_hash_get_current_data_ex(table, &position));
                    zend_hash_move_forward_ex(table, &position)) {
                    zval key;

					zend_hash_get_current_key_zval_ex(table, &key, &position);
					if (Z_TYPE(key) == IS_STRING)
						zend_string_delref(Z_STR(key));
                    
					switch (Z_TYPE(key)) {
						case IS_LONG:
		                    if (!overwrite && zend_hash_index_exists(threaded->store.props, Z_LVAL(key))) {
		                        goto next;
		                    }
		                    pthreads_store_write(destination, &key, pzval);
						break;

                        case IS_STRING:
							if (!overwrite && zend_hash_exists(threaded->store.props, Z_STR(key))) {
                                goto next;
                            }
                            pthreads_store_write(destination, &key, pzval);
						break;
                    }

next:
                    index++;
               }
               
               pthreads_monitor_unlock(threaded->monitor);
           }
        } break;
    }
    
    return SUCCESS;
} /* }}} */


/* {{{ Will free store element */
static void pthreads_store_storage_dtor (pthreads_storage *storage){
	if (storage) {
		switch (storage->type) {
			case IS_CLOSURE:
			case IS_OBJECT:
			case IS_STRING:
			case IS_ARRAY:
			case IS_RESOURCE:
				if (storage->data) {
					free(storage->data);
				}
			break;
		}
		free(storage);
	}
} /* }}} */

/* {{{ iteration helpers */
void pthreads_store_reset(zval *object, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_hash_internal_pointer_reset_ex(threaded->store.props, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
}

void pthreads_store_key(zval *object, zval *key, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_hash_get_current_key_zval_ex(threaded->store.props, key, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
}

void pthreads_store_data(zval *object, zval *value, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage = (pthreads_storage*) 
			zend_hash_get_current_data_ptr_ex(threaded->store.props, position);

		if (storage) {
			pthreads_store_convert(storage, value);
		} else ZVAL_UNDEF(value);
		
		pthreads_monitor_unlock(threaded->monitor);
	}
}

void pthreads_store_forward(zval *object, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_hash_move_forward_ex(
			threaded->store.props, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
} /* }}} */

#endif
