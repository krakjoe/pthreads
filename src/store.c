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
			&store->table, 8, NULL, 
			(dtor_func_t) pthreads_store_storage_table_dtor, 1);
	}

	return store;
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_store_is_immutable(zval *object, zend_string *key) {	
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_storage *storage;

	if (instanceof_function(Z_OBJCE_P(object), pthreads_volatile_entry)) {
		return 0;
	}

	if ((storage = zend_hash_find_ptr(&threaded->store->table, key)) && (storage->type == IS_PTHREADS)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"Threaded members previously set to Threaded objects are immutable, cannot overwrite %s",
			ZSTR_VAL(key));		
		return 1;
	}

	return 0;
} /* }}} */

/* {{{ */
int pthreads_store_delete(zval *object, zend_string *key) {
	int result = FAILURE;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	
	if (pthreads_monitor_lock(threaded->monitor)) {
		if (!pthreads_store_is_immutable(object, key)) {
			result = zend_hash_del(&threaded->store->table, key);
		}
		pthreads_monitor_unlock(threaded->monitor);
	} else result = FAILURE;
	
	if (result == SUCCESS) {
		zend_hash_del(threaded->std.properties, key);
	}

	return result;
}
/* }}} */

/* {{{ */
zend_bool pthreads_store_isset(zval *object, zend_string *key, int has_set_exists) {
	zend_bool isset = 0;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage;

		isset = (storage = zend_hash_find_ptr(&threaded->store->table, key)) != NULL;

		if (has_set_exists && storage) {
		    switch (storage->type) {
				case IS_LONG:
				case IS_TRUE:
				case IS_FALSE:
					if (storage->simple.lval == 0)
					isset = 0;
				break;
		
				case IS_ARRAY:
					if (storage->length == 0)
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

	return isset;
} /* }}} */

/* {{{ */
int pthreads_store_read(zval *object, zend_string *key, zval *read) {
	int result = FAILURE;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zval *property = !IS_PTHREADS_VOLATILE(object) ?
		zend_hash_find(threaded->std.properties, key) : NULL;

	if (property && IS_PTHREADS_OBJECT(property)) {
		ZVAL_COPY(read, property);
		return SUCCESS;
	}

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage;
		if ((storage = zend_hash_find_ptr(&threaded->store->table, key))) {
			result = pthreads_store_convert(storage, read);
		}
		pthreads_monitor_unlock(threaded->monitor);
	}

	if (result != SUCCESS) {
		ZVAL_NULL(read);
	} else {
		if (!IS_PTHREADS_VOLATILE(object) && IS_PTHREADS_OBJECT(read)) {
			rebuild_object_properties(&threaded->std);
			zend_hash_update(
				threaded->std.properties, key, read);
			Z_ADDREF_P(read);
		}
	}

	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_write(zval *object, zend_string *key, zval *write) {
	int result = FAILURE;
	zend_string *keyed;
	pthreads_storage *storage;
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	keyed = zend_string_init(key->val, key->len, 1);
	storage = pthreads_store_create(write, 1);

	if (pthreads_monitor_lock(threaded->monitor)) {
		if (!pthreads_store_is_immutable(object, key)) {
			if (zend_hash_update_ptr(&threaded->store->table, keyed, storage)) {
				result = SUCCESS;
			}
		}
		pthreads_monitor_unlock(threaded->monitor);
	}

	if (result != SUCCESS) {
		pthreads_store_storage_dtor(storage);
	} else {
		if (IS_PTHREADS_OBJECT(write)) {
			/*
				This could be a volatile object, but, we don't want to break
				normal refcounting, we'll just never read the reference
			*/
			rebuild_object_properties(&threaded->std);
			zend_hash_update(
				threaded->std.properties, key, write);
			Z_ADDREF_P(write);
		}
	}
	zend_string_release(keyed);
	
	return result;
} /* }}} */

/* {{{ */
int pthreads_store_separate(zval * pzval, zval *separated, zend_bool complex) {
	int result = FAILURE;
	pthreads_storage *storage;
	
	if (Z_TYPE_P(pzval) != IS_NULL) {
		storage = pthreads_store_create(pzval, complex);
		if (pthreads_store_convert(storage, separated) != SUCCESS) {
			ZVAL_NULL(separated);
		}			
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
		if (pthreads_store_convert(storage, zv) != SUCCESS) {
			ZVAL_NULL(zv);
		}
	    pthreads_store_storage_dtor(storage);
	} else ZVAL_NULL(zv);
} /* }}} */

/* {{{ */
int pthreads_store_count(zval *object, zend_long *count) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		(*count) = zend_hash_num_elements(
		    &threaded->store->table);
		pthreads_monitor_unlock(threaded->monitor);
	} else (*count) = 0L;
   
	return SUCCESS;
} /* }}} */

/* {{{ */
int pthreads_store_shift(zval *object, zval *member) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_string *key;
		zend_ulong index;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_reset_ex(&threaded->store->table, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(&threaded->store->table, &position))) {
			switch (zend_hash_get_current_key_ex(&threaded->store->table, &key, &index, &position)) {
				case HASH_KEY_IS_STRING:
					if (!pthreads_store_is_immutable(object, key)) {
						pthreads_store_convert(storage, member);
						zend_hash_del(
							&threaded->store->table, key);
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
   
	if (pthreads_monitor_lock(threaded->monitor)) {
		HashPosition position;
		pthreads_storage *storage;
		zend_string *key;
		zend_ulong index;

		array_init(chunk);
		zend_hash_internal_pointer_reset_ex(&threaded->store->table, &position);
		while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
			(storage = zend_hash_get_current_data_ptr_ex(&threaded->store->table, &position))) {
			zval zv;

			switch (zend_hash_get_current_key_ex(&threaded->store->table, &key, &index, &position)) {
				case HASH_KEY_IS_STRING:
					if (!pthreads_store_is_immutable(object, key)) {
						pthreads_store_convert(storage, &zv);
						zend_hash_update(
							Z_ARRVAL_P(chunk), key, &zv);
						zend_hash_del(&threaded->store->table, key);
					} else break;
				break;			
			}

			zend_hash_move_forward_ex(&threaded->store->table, &position);
		}
		pthreads_monitor_unlock(threaded->monitor);

		return SUCCESS;
	}

    return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_store_pop(zval *object, zval *member) {
	pthreads_object_t* threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
   
	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_string *key;
		zend_ulong index;
		HashPosition position;
		pthreads_storage *storage;

		zend_hash_internal_pointer_end_ex(&threaded->store->table, &position);
		if ((storage = zend_hash_get_current_data_ptr_ex(&threaded->store->table, &position))) {
			switch (zend_hash_get_current_key_ex(&threaded->store->table, &key, &index, &position)) {
				case HASH_KEY_IS_STRING:
					if (!pthreads_store_is_immutable(object, key)) {
						pthreads_store_convert(storage, member);
						zend_hash_del(
							&threaded->store->table, key);			
					}
				break;
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

	/* php is reusing this hash and making things misbehave */
	zend_hash_clean(hash);
    
	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_string *name = NULL;
		zend_ulong idx;
		pthreads_storage *storage;

		ZEND_HASH_FOREACH_KEY_PTR(&threaded->store->table, idx, name, storage) {
			zval pzval;
			zend_string *rename;
			ZVAL_NULL(&pzval);

			if (pthreads_store_convert(storage, &pzval)!=SUCCESS) {
				continue;
			}

			rename = zend_string_init(name->val, name->len, 0);
			if (!zend_hash_update(hash, rename, &pzval))
				zval_dtor(&pzval);
			zend_string_release(rename);
		} ZEND_HASH_FOREACH_END();

		pthreads_monitor_unlock(threaded->monitor);
	}
} /* }}} */

/* {{{ */
void pthreads_store_free(pthreads_store_t *store){
	zend_hash_destroy(&store->table);
	free(store);	
} /* }}} */

/* {{{ */
static pthreads_storage* pthreads_store_create(zval *unstore, zend_bool complex){					
	pthreads_storage *storage = NULL;

	if (Z_TYPE_P(unstore) == IS_INDIRECT)
		return pthreads_store_create(Z_INDIRECT_P(unstore), 1);
	if (Z_TYPE_P(unstore) == IS_REFERENCE)
		return pthreads_store_create(&Z_REF_P(unstore)->val, 1);

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
				zval *search;
				zend_ulong index;
				zend_string *name;
				HashPosition position;
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

			zend_create_closure(pzval, closure, EG(scope), closure->common.scope, NULL);
			name_len = spprintf(&name, 0, "Closure@%p", zend_get_closure_method_def(pzval));
			if (!zend_hash_str_update_ptr(EG(function_table), name, name_len, closure)) {
				result = FAILURE;
				zval_dtor(pzval);
			} else result = SUCCESS;
			efree(name);
		} break;

		case IS_PTHREADS: {
			pthreads_object_t* threaded = PTHREADS_FETCH_FROM(storage->data);

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
			if (result == FAILURE) {
			    ZVAL_NULL(pzval);
			}
		break;

		default: ZVAL_NULL(pzval);
	}

	return result;
}
/* }}} */

/* {{{ */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex) {
	int result = FAILURE;
	if (pzval && (Z_TYPE_P(pzval) != IS_NULL)) {
		smart_str smart;

		memset(&smart, 0, sizeof(smart_str));

		if ((Z_TYPE_P(pzval) != IS_OBJECT) ||
			(Z_OBJCE_P(pzval)->serialize != zend_class_serialize_deny)) {
			php_serialize_data_t vars;

			PHP_VAR_SERIALIZE_INIT(vars);
			php_var_serialize(&smart, pzval, &vars);
			PHP_VAR_SERIALIZE_DESTROY(vars);
		}

		if (smart.s) {
			*slength = smart.s->len;
			if (*slength) {
				*pstring = malloc(*slength+1);
				if (*pstring) {
					memcpy(
						(char*) *pstring, (const void*) smart.s->val, smart.s->len
					);
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
			
			if (result != FAILURE) {
				if (Z_REFCOUNTED_P(pzval) && !IS_PTHREADS_OBJECT(pzval)) {
					Z_SET_REFCOUNT_P(pzval, 1);
				}
			} else ZVAL_NULL(pzval);
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
                        HashTable *tables[2] = {&threaded[0]->store->table, &threaded[1]->store->table};
						zend_string *key = NULL;
                        zend_ulong idx = 0L;                        

                        for (zend_hash_internal_pointer_reset_ex(tables[1], &position);
                             (storage = zend_hash_get_current_data_ptr_ex(tables[1], &position));
                             zend_hash_move_forward_ex(tables[1], &position)) {

                            if (zend_hash_get_current_key_ex(tables[1], &key, &idx, &position) == HASH_KEY_IS_STRING) {
                                if (!overwrite && zend_hash_exists(tables[0], key)) {
                                    continue;
                                }

								if (pthreads_store_is_immutable(destination, key)) {
									break;
								}
								
                                if (storage->type != IS_RESOURCE) {
                                    pthreads_storage copy;

									memcpy(&copy, storage, sizeof(pthreads_storage));

                                    switch (copy.type) {
                                        case IS_STRING:
                                        case IS_OBJECT:
				        				case IS_ARRAY: if (storage->length) {
	                                        copy.data = (char*) malloc(copy.length+1);
											if (!copy.data) {
												break;
											}
											memcpy(copy.data, (const void*) storage->data, copy.length);
                                    	} break;
                                    }

                                    zend_hash_update_mem(tables[0], key, &copy, sizeof(pthreads_storage));
                                }
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
                    zend_string *key;
                    zend_ulong idx;
                    zval tmp;

                    switch (zend_hash_get_current_key_ex(table, &key, &idx, &position)) {
                        case HASH_KEY_IS_STRING: 
							if (!overwrite && zend_hash_exists(&threaded->store->table, key)) {
                                goto next;
                            }
                            pthreads_store_write(destination, key, pzval);
						break;
                        
                        case HASH_KEY_IS_LONG:
							ZVAL_LONG(&tmp, idx);
		                    convert_to_string(&tmp);
		                    if (!overwrite && zend_hash_exists(&threaded->store->table, Z_STR(tmp))) {
		                        zval_dtor(&tmp);
		                        goto next;
		                    }
		                    pthreads_store_write(destination, Z_STR(tmp), pzval);
		                    zval_dtor(&tmp);
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
} /* }}} */

/* {{{ iteration helpers */
void pthreads_store_reset(zval *object, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_hash_internal_pointer_reset_ex(
			&threaded->store->table, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
}

void pthreads_store_key(zval *object, zval *key, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		zend_hash_get_current_key_zval_ex(
			&threaded->store->table, key, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
}

void pthreads_store_data(zval *object, zval *value, HashPosition *position) {
	pthreads_object_t *threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(threaded->monitor)) {
		pthreads_storage *storage = (pthreads_storage*) 
			zend_hash_get_current_data_ptr_ex(&threaded->store->table, position);

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
			&threaded->store->table, position);
		pthreads_monitor_unlock(threaded->monitor);
	}
} /* }}} */

#endif
