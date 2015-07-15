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
#ifndef HAVE_PTHREADS_STORE
#define HAVE_PTHREADS_STORE

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
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
	size_t 		length;
	zend_bool 	exists;
	union {
	    long    lval;
	    double  dval;
	} simple;
	void    *data;
} pthreads_storage;

#ifndef Z_OBJ_P
#	define Z_OBJ_P(zval_p) ((zend_object*)(EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(zval_p)].bucket.obj.object))
#endif

/* {{{ statics */
static void pthreads_store_create(pthreads_storage *storage, zval *pzval, zend_bool complex);
static int pthreads_store_convert(pthreads_storage *storage, zval *pzval);
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex);
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength);
static int pthreads_store_remove_complex_recursive(zval *pzval);
static int pthreads_store_validate_object(zval *pzval);
static int pthreads_store_remove_complex(zval *pzval);
static void pthreads_store_storage_dtor (pthreads_storage *element);
/* }}} */

static void pthreads_store_storage_table_dtor (zval *element) {
	pthreads_store_storage_dtor(Z_PTR_P(element));
}

/* {{{ allocate storage for an object */
pthreads_store pthreads_store_alloc() {
	pthreads_store store = malloc(sizeof(*store));
	
	if (store) {
		zend_hash_init(&store->table, 8, NULL, (dtor_func_t) pthreads_store_storage_table_dtor, 1);

		if ((store->lock = pthreads_lock_alloc())) {
		    	store->next = 0L;
			return store;
		}

		zend_hash_destroy(&store->table);
		free(store);
	}
	return NULL;
} /* }}} */

/* {{{ lock storage, userland only */
zend_bool pthreads_store_lock(zval *this_ptr) {	
	PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(this_ptr));
	if (pobject) {
		return pthreads_lock_acquire(
			pobject->store->lock,
			&pobject->hold
		);
	} else return 0;
} /* }}} */

/* {{{ unlock storage, userland only */
zend_bool pthreads_store_unlock(zval *this_ptr) {
	PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(this_ptr));
	if (pobject) {
		return pthreads_lock_release(
			pobject->store->lock,
			pobject->hold
		);
	} else return 0;
} /* }}} */

/* {{{ delete a value from the store */
int pthreads_store_delete(pthreads_store store, zend_string *key) {
	int result = FAILURE;
	zend_bool locked;
	
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked)) {
			if (zend_hash_exists(&store->table, key)) {
				result = zend_hash_del(&store->table, key);
			} else result = SUCCESS;
			pthreads_lock_release(store->lock, locked);
		} else result = FAILURE;
	}
	return result;
}
/* }}} */

/* {{{ isset helper for handlers */
zend_bool pthreads_store_isset(pthreads_store store, zend_string *key, int has_set_exists) {
	zend_bool locked = 0, isset = 0;
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked)) {
		    pthreads_storage *storage;
		    
		    isset = (storage = zend_hash_find_ptr(&store->table, key)) != NULL;
		    
		    if (has_set_exists) {
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
		pthreads_lock_release(store->lock, locked);
		}
	}
	return isset;
} /* }}} */

/* {{{ read a value from store */
int pthreads_store_read(pthreads_store store, zend_string *key, zval *read) {
	zend_bool locked = 0;
	int result = FAILURE;
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked)) {
			pthreads_storage *storage = NULL;

			if ((storage = zend_hash_find_ptr(&store->table, key))) {
				result = pthreads_store_convert(storage, read);
			}
			pthreads_lock_release(store->lock, locked);
		}

		if (result == SUCCESS) {
			if (Z_REFCOUNTED_P(read))
				Z_SET_REFCOUNT_P(read, 1);
		} else {
			ZVAL_NULL(read);
		}
	}
	return SUCCESS;
} /* }}} */

/* {{{ write a value to store */
int pthreads_store_write(pthreads_store store, zend_string *key, zval *write) {
	int result = FAILURE;
	zend_bool locked;
	
	if (store) {
		pthreads_storage storage;
		zend_string *keyed = zend_string_init(key->val, key->len, 1);

		pthreads_store_create(&storage, write, 1);
		
		if (pthreads_lock_acquire(store->lock, &locked)) {
			if (zend_hash_update_mem(&store->table, keyed, &storage, sizeof(pthreads_storage))) {
				pthreads_lock_release(store->lock, locked);
				zend_string_release(keyed);
				return SUCCESS;
			} else pthreads_store_storage_dtor(&storage);
			pthreads_lock_release(store->lock, locked);
		} else pthreads_store_storage_dtor(&storage);

		zend_string_release(keyed);
	}
	
	return FAILURE;
} /* }}} */

/* {{{ seperate a zval using internals */
int pthreads_store_separate(zval * pzval, zval *separated, zend_bool complex) {
	int result = FAILURE;
	pthreads_storage storage;
	
	if (pzval) {
        	pthreads_store_create(&storage, pzval, complex);
		result = pthreads_store_convert(&storage, separated);
		if (result == SUCCESS)
			pthreads_store_storage_dtor(&storage);
	    else ZVAL_UNDEF(separated);
	} else ZVAL_UNDEF(separated);
	
	return result;
} /* }}} */

/* {{{ count properties */
int pthreads_store_count(zval *object, long *count) {
   PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
   
   if (pthreads) {
    if (pthreads_store_lock(object)) {
        (*count) = zend_hash_num_elements(
            &pthreads->store->table);
        pthreads_store_unlock(object);
    } else (*count) = 0L;
   } else (*count) = 0L;
   
   return SUCCESS;
} /* }}} */

/* {{{ shift member */
int pthreads_store_shift(zval *object, zval *member) {
   PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
   
   if (pthreads_store_lock(object)) {
	zend_string *key;
	zend_ulong index;
	HashPosition position;
	pthreads_storage *storage;
	
	zend_hash_internal_pointer_reset_ex(&pthreads->store->table, &position);
	if ((storage = zend_hash_get_current_data_ptr_ex(&pthreads->store->table, &position))) {
		int type = zend_hash_get_current_key_ex(
			&pthreads->store->table, &key, &index, &position);
		pthreads_store_convert(storage, member);
		if (type == HASH_KEY_IS_STRING)
			zend_hash_del(&pthreads->store->table, key);
		else zend_hash_index_del(&pthreads->store->table, index);
	} else ZVAL_NULL(member);
        pthreads_store_unlock(object);
        
        return SUCCESS;
    }
	
    return FAILURE;
} /* }}} */

/* {{{ store chunk */
int pthreads_store_chunk(zval *object, long size, zend_bool preserve, zval *chunk) {
   PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
   
   if (pthreads_store_lock(object)) {
	/* do chunk */
	HashPosition position;
	pthreads_storage *storage;
	zend_string *key;
	zend_ulong index;
	
	array_init(chunk);
	zend_hash_internal_pointer_reset_ex(&pthreads->store->table, &position);
	while((zend_hash_num_elements(Z_ARRVAL_P(chunk)) < size) &&
	      (storage = zend_hash_get_current_data_ptr_ex(&pthreads->store->table, &position))) {
		zval zv;
		int type = zend_hash_get_current_key_ex(
			&pthreads->store->table, &key, &index, &position);
		pthreads_store_convert(storage, &zv);
		switch (type) {
			case HASH_KEY_IS_STRING:
				zend_hash_update(Z_ARRVAL_P(chunk), key, &zv);
			break;
			
			case HASH_KEY_IS_LONG:
				zend_hash_index_update(Z_ARRVAL_P(chunk), index, &zv);
			break;
		}
		zend_hash_move_forward_ex(&pthreads->store->table, &position);
		if (type == HASH_KEY_IS_STRING)
			zend_hash_del(&pthreads->store->table, key);
		else zend_hash_index_del(&pthreads->store->table, index);
	}	
        pthreads_store_unlock(object);
     
	return SUCCESS;
    }

    return FAILURE;
} /* }}} */

/* {{{ pop member */
int pthreads_store_pop(zval *object, zval *member) {
   PTHREAD pthreads = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
   
   if (pthreads_store_lock(object)) {
	zend_string *key;
	zend_ulong index;
	HashPosition position;
	pthreads_storage *storage;
	
	zend_hash_internal_pointer_end_ex(&pthreads->store->table, &position);
	if ((storage = zend_hash_get_current_data_ptr_ex(&pthreads->store->table, &position))) {
		int type = zend_hash_get_current_key_ex(
			&pthreads->store->table, &key, &index, &position);
		pthreads_store_convert(storage, member);
		if (type == HASH_KEY_IS_STRING)
			zend_hash_del(&pthreads->store->table, key);
		else zend_hash_index_del(&pthreads->store->table, index);
	} else ZVAL_NULL(member);
	
        pthreads_store_unlock(object);
        
        return SUCCESS;
    }
   
   return FAILURE;
} /* }}} */

/* {{{ copy store to hashtable */
void pthreads_store_tohash(pthreads_store store, HashTable *hash) {

	zend_bool locked;
	if (store) {
		/* php is reusing this hash and making things misbehave */
		zend_hash_clean(hash);
	    
		if (pthreads_lock_acquire(store->lock, &locked)) {
			zend_string *name = NULL;
			zend_ulong idx;
			pthreads_storage *storage;

			ZEND_HASH_FOREACH_KEY_PTR(&store->table, idx, name, storage) {
				zval pzval;
				ZVAL_UNDEF(&pzval);

				if (pthreads_store_convert(storage, &pzval)!=SUCCESS) {
					ZVAL_UNDEF(&pzval);
					continue;
				}

				if (!zend_hash_update(hash, name, &pzval))
					zval_dtor(&pzval);
			} ZEND_HASH_FOREACH_END();
		
			pthreads_lock_release(store->lock, locked);
		}
	}
} /* }}} */

/* {{{ free store storage for a thread */
void pthreads_store_free(pthreads_store store){
	if (store) {
		zend_bool locked;

		if (pthreads_lock_acquire(store->lock, &locked)) {
			zend_hash_destroy(&store->table);
			pthreads_lock_release(store->lock, locked);
		}
		pthreads_lock_free(store->lock);
		free(store);
	}
} /* }}} */

/* {{{ Will storeize the zval into a newly allocated buffer which must be free'd by the caller */
static void pthreads_store_create(pthreads_storage *storage, zval *unstore, zend_bool complex){					
	
	if (unstore) {
		/*
		* Make an exact copy of the store data for internal storage
		*/
		storage->length = 0;
		storage->exists = 0;
		storage->data = NULL;

		switch((storage->type = Z_TYPE_P(unstore))){
			case IS_NULL: { /* nothing to do */ } break;

			case IS_INDIRECT: {
				pthreads_store_create(storage, Z_INDIRECT_P(unstore), 1);
			} break;

			case IS_STRING: {
				if ((storage->length = Z_STRLEN_P(unstore))) {
					storage->data = (char*) malloc(storage->length+1);
					memcpy(
						storage->data, (const void*) Z_STRVAL_P(unstore), storage->length
					);
				}
			} break;
			
			case IS_TRUE: storage->simple.lval = 1; break;
			case IS_FALSE: storage->simple.lval = 0; break;

			case IS_LONG: {
				storage->simple.lval = Z_LVAL_P(unstore);
			} break;

			case IS_RESOURCE: {
				if (complex) {
					pthreads_resource resource = malloc(sizeof(*resource));
					if (resource) {
						TSRMLS_CACHE_UPDATE();
						
						resource->original = Z_RES_P(unstore);
						resource->ls = TSRMLS_CACHE;
						
						storage->data = resource;
						Z_ADDREF_P(unstore);
					}
				} else {
					storage->type = IS_NULL;
				}
			} break;
			
			case IS_DOUBLE:
			    storage->simple.dval = Z_DVAL_P(unstore);
			break;
			
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
			    
			case IS_ARRAY: {
				if (pthreads_store_tostring(unstore, (char**) &storage->data, &storage->length, complex)==SUCCESS) {
					if (storage->type==IS_ARRAY) {
						storage->exists = zend_hash_num_elements(
						    Z_ARRVAL_P(unstore));
					}
				}
			} break;
			
			default: {
			    storage->exists = 0;
			}
		}
	}
}
/* }}} */

/* {{{ Will unstoreize data into the allocated zval passed */
static int pthreads_store_convert(pthreads_storage *storage, zval *pzval){
	int result = SUCCESS;
	
	if (storage) {
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

				TSRMLS_CACHE_UPDATE();

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
						if (zend_hash_next_index_insert(&EG(regular_list), pzval)==SUCCESS) {
						    Z_ADDREF_P(pzval);
						} else ZVAL_NULL(pzval);
					} else {
						ZVAL_COPY(pzval, search);
						Z_ADDREF_P(pzval);
					}
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
			    closure = (zend_function*) zend_get_closure_method_def(pzval);
			    name_len = spprintf(&name, 0, "Closure@%p", closure);
			    if (!zend_hash_str_update_ptr(EG(function_table), name, name_len, closure)) {
			        result = FAILURE;
				zval_dtor(pzval);
			    } else result = SUCCESS;
			    efree(name);
			} break;
			
			case IS_ARRAY:
			case IS_OBJECT: {
				result = pthreads_store_tozval(
					pzval, 
					(char*) storage->data, 
					storage->length
				);
				if (result == FAILURE) {
				    /* display error, do something ... ? */
				    ZVAL_UNDEF(pzval);
				}
			} break;
			
			default: {
			    ZVAL_UNDEF(pzval);
			}
		}	
	}
	return result;
}
/* }}} */

/* {{{ zval to string */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex) {
	int result = FAILURE;
	if (pzval && 
	    (Z_TYPE_P(pzval) != IS_NULL) && 
	    (Z_TYPE_P(pzval) != IS_OBJECT || pthreads_store_validate_object(pzval))) {
	    
		smart_str *psmart = (smart_str*) ecalloc(1, sizeof(smart_str));
		
		if (psmart) {
			if (!complex && 
			    (Z_TYPE_P(pzval) == IS_OBJECT || Z_TYPE_P(pzval) == IS_ARRAY)) {
				//pthreads_store_remove_complex_recursive(pzval);
			}

			{
				if ((Z_TYPE_P(pzval) != IS_OBJECT) ||
					(Z_OBJCE_P(pzval)->serialize != zend_class_serialize_deny)) {
					php_serialize_data_t vars;
					PHP_VAR_SERIALIZE_INIT(vars);
					php_var_serialize(						
						psmart,
						pzval, 
						&vars
					);
					PHP_VAR_SERIALIZE_DESTROY(vars);
				}
			}
			*slength = psmart->s->len;
			if (psmart->s->len) {
				*pstring = malloc(psmart->s->len+1);
				if (*pstring) {
					memcpy(
						(char*) *pstring, (const void*) psmart->s->val, psmart->s->len
					);
					result = SUCCESS;
				}
			} else *pstring = NULL;
			smart_str_free(psmart);	
			efree(psmart);
		}
	} else {
	    *slength = 0;
	    *pstring = NULL;
	}
	return result;
} /* }}} */

/* {{{ string to zval */
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength) {
	int result = SUCCESS;
	uint refcount = Z_REFCOUNTED_P(pzval) ? Z_REFCOUNT_P(pzval) : 0;
	
	if (pstring) {
		const unsigned char* pointer = (const unsigned char*) pstring;
		if (pointer) {
			/*
			* Populate PHP storage from pthreads_storage
			*/
			{
				php_unserialize_data_t vars;
				PHP_VAR_UNSERIALIZE_INIT(vars);
				if (!php_var_unserialize(pzval, &pointer, pointer+slength, &vars)) {
					result = FAILURE;
					zval_dtor(pzval);
				}							
				PHP_VAR_UNSERIALIZE_DESTROY(vars);
			}
			
			if (result != FAILURE) {
				if (Z_REFCOUNTED_P(pzval)) {
					Z_SET_REFCOUNT_P(pzval, refcount);
				}
			} else ZVAL_NULL(pzval);
		} else result = FAILURE;
	} else result = FAILURE;
	
	return result;
} /* }}} */

/* {{{ import members from one object (or array) into a pthreads object */
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite) {
    if (Z_TYPE_P(from) != IS_ARRAY && 
        Z_TYPE_P(from) != IS_OBJECT) {
        return FAILURE;
    }
    
    switch (Z_TYPE_P(from)) {
        case IS_OBJECT: {
            /* check for a suitable pthreads object */
            if (IS_PTHREADS_OBJECT(from)) {
                
                zend_bool locked[2] = {0, 0};
                PTHREAD pobjects[2] = {PTHREADS_FETCH_FROM(Z_OBJ_P(destination)), PTHREADS_FETCH_FROM(Z_OBJ_P(from))};
                
                /* acquire destination lock */
                if (pthreads_lock_acquire(pobjects[0]->store->lock, &locked[0])) {
                    
                    /* acquire other lock */
                    if (pthreads_lock_acquire(pobjects[1]->store->lock, &locked[1])) {
                        
                        /* free to do what we want, everything locked */
                        HashPosition position;
                        pthreads_storage *storage;
			zval *bucket;
                        HashTable *tables[2] = {&pobjects[0]->store->table, &pobjects[1]->store->table};
			zend_string *key = NULL;
                        zend_ulong idx = 0L;                        
			
                        for (zend_hash_internal_pointer_reset_ex(tables[1], &position);
                             (bucket = zend_hash_get_current_data_ex(tables[1], &position));
                             zend_hash_move_forward_ex(tables[1], &position)) {

			    storage = (pthreads_storage*) Z_PTR_P(bucket);

                            if (zend_hash_get_current_key_ex(tables[1], &key, &idx, &position) == HASH_KEY_IS_STRING) {
                                
                                /* skip if not overwriting where the 
                                        entry exists in destination table */
                                if (!overwrite && zend_hash_exists(tables[0], key)) {
                                    continue;
                                }
                                
                                /* skip resources */
                                if (storage->type != IS_RESOURCE) {

                                    /* copy storage */
                                    pthreads_storage copy;
                                    
				    memcpy(&copy, storage, sizeof(pthreads_storage));

                                    switch (copy.type) {
                                        case IS_STRING: {
                                        	copy.data = (char*) malloc(copy.length+1);
		                                memcpy(
		                                    copy.data, (const void*) storage->data, copy.length
		                                );
                                        } break;
                                        
                                        case IS_OBJECT:
				        case IS_ARRAY: {
				        	if (storage->length) {
		                                        copy.data = (char*) malloc(copy.length+1);
		                                        memcpy(
		                                            copy.data, (const void*) storage->data, copy.length
		                                        );
                                        	}
				        } break;
				                        
				        default: {  }
                                    }

                                    zend_hash_update_mem(tables[0], key, &copy, sizeof(pthreads_storage));
                                }
                            }
                        }
                        
                        pthreads_lock_release(pobjects[1]->store->lock, locked[1]);
                    }
                    
                    pthreads_lock_release(pobjects[0]->store->lock, locked[0]);
                    
                    return SUCCESS;
                    
                } else return FAILURE;
            }
        }
        
        /* fall through on purpose to handle normal objects and arrays */
        
        default: {
           zend_bool locked = 0;
           PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(destination));
           
           if (pthreads_lock_acquire(pobject->store->lock, &locked)) {
               HashPosition position;
               zval *pzval;
               int32_t index = 0;
               HashTable *table = (Z_TYPE_P(from) == IS_ARRAY) ? Z_ARRVAL_P(from) : Z_OBJPROP_P(from);
               
               for (zend_hash_internal_pointer_reset_ex(table, &position);
                    (pzval = zend_hash_get_current_data_ex(table, &position));
                    zend_hash_move_forward_ex(table, &position)) {
                    zend_string *key;
                    zend_ulong idx;
                    
                    switch (zend_hash_get_current_key_ex(table, &key, &idx, &position)) {
                        case HASH_KEY_IS_STRING: {
                            if (!overwrite && zend_hash_exists(table, key)) {
                                goto next;
                            }
                            
                            pthreads_store_write(pobject->store, key, pzval);
                        } break;
                        
                        case HASH_KEY_IS_LONG: {
                            zval zkey;
                            
                            ZVAL_LONG(&zkey, idx);
                            
                            convert_to_string(&zkey);
                            
                            if (!overwrite && zend_hash_exists(table, Z_STR(zkey))) {
                                zval_dtor(&zkey);
                                goto next;
                            }
                            
                            pthreads_store_write(pobject->store, Z_STR(zkey), pzval);
                            
                            zval_dtor(&zkey);
                        } break;
                    }
next:
                    index++;
               }
               
               pthreads_lock_release(pobject->store->lock, locked);
           }
        } break;
    }
    
    return SUCCESS;
} /* }}} */

/* {{{ set resources to NULL for non-complex types; helper-function for pthreads_store_remove_complex_recursive */
static int pthreads_store_remove_complex(zval *pzval) {
	if (Z_TYPE_P(pzval) == IS_RESOURCE) {
		ZVAL_UNDEF(pzval);
		return ZEND_HASH_APPLY_REMOVE;
	}
	return pthreads_store_remove_complex_recursive(pzval);
} /* }}} */

/* {{{ check a handle is valid before reading it */
static int pthreads_store_validate_object(zval *pzval) {
	return 1;
} /* }}} */

/* {{{ set corrupt objects (like mysqli after thread duplication) to NULL and recurse */
static int pthreads_store_remove_complex_recursive(zval *pzval) {
	int is_temp;

	HashTable *thash = NULL;
	
	switch (Z_TYPE_P(pzval)) {
		case IS_ARRAY:
			thash = Z_ARRVAL_P(pzval);

		case IS_OBJECT:
			if (thash == NULL) {
				if (!pthreads_store_validate_object(pzval)) {
					GC_REMOVE_ZVAL_FROM_BUFFER(pzval);
					ZVAL_UNDEF(pzval);
					return ZEND_HASH_APPLY_KEEP;
				}
				thash = Z_OBJDEBUG_P(pzval, is_temp);
			}

			if (thash) {
				zend_hash_apply(thash, (apply_func_t)pthreads_store_remove_complex);
			}
			
		break;
	}
	return ZEND_HASH_APPLY_KEEP;
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
	}
} /* }}} */

#endif
