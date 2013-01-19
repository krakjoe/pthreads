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

typedef struct _pthreads_storage {
	void		*data;
	size_t 		length;
	zend_uchar 	type;
	zend_bool 	exists;
	long		lval;
	double		dval;
} *pthreads_storage;

#ifndef Z_OBJ_P
#	define Z_OBJ_P(zval_p) ((zend_object*)(EG(objects_store).object_buckets[Z_OBJ_HANDLE_P(zval_p)].bucket.obj.object))
#endif

/* {{{ statics */
static pthreads_storage pthreads_store_create(zval *pzval, zend_bool complex TSRMLS_DC);
static int pthreads_store_convert(pthreads_storage storage, zval *pzval TSRMLS_DC);
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex TSRMLS_DC);
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength TSRMLS_DC);
static int pthreads_store_remove_resources_recursive(zval **pzval TSRMLS_DC);
static int pthreads_store_validate_resource(zval **pzval TSRMLS_DC);
static int pthreads_store_remove_resources(zval **pzval TSRMLS_DC);
static void pthreads_store_storage_dtor (pthreads_storage *element);
static void pthreads_store_event_dtor (pthreads_synchro *element);
/* }}} */

/* {{{ allocate storage for an object */
pthreads_store pthreads_store_alloc(TSRMLS_D) {
	pthreads_store store = calloc(1, sizeof(*store));
	
	if (store) {
		if (zend_ts_hash_init(&store->table, 8, NULL, (dtor_func_t) pthreads_store_storage_dtor, 1)==SUCCESS){
			if (zend_ts_hash_init(&store->event, 8, NULL, (dtor_func_t) pthreads_store_event_dtor, 1)==SUCCESS) {
				if ((store->lock = pthreads_lock_alloc(TSRMLS_C))) {
					return store;
				}
				zend_ts_hash_destroy(&store->event);
			}
			zend_ts_hash_destroy(&store->table);
		}
		free(store);
	}
	return NULL;
} /* }}} */

/* {{{ lock storage */
zend_bool pthreads_store_lock(zval *this_ptr TSRMLS_DC) {	
	PTHREAD pobject = PTHREADS_FETCH_FROM(getThis());
	if (pobject) {
		return pthreads_lock_acquire(
			pobject->store->lock,
			&pobject->hold TSRMLS_CC
		);
	} else return 0;
} /* }}} */

/* {{{ unlock storage */
zend_bool pthreads_store_unlock(zval *this_ptr TSRMLS_DC) {
	PTHREAD pobject = PTHREADS_FETCH_FROM(getThis());
	if (pobject) {
		return pthreads_lock_release(
			pobject->store->lock,
			pobject->hold TSRMLS_CC
		);
	} else return 0;
} /* }}} */

/* {{{ delete a value from the store */
int pthreads_store_delete(pthreads_store store, char *key, int keyl TSRMLS_DC) {
	int result = FAILURE;
	zend_bool locked;
	
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
			if (zend_ts_hash_exists(&store->table, key, keyl)) {
				if (zend_ts_hash_del(&store->table, key, keyl)!=SUCCESS) {
					result = FAILURE;
				} else result = SUCCESS;
			} else result = SUCCESS;
			pthreads_lock_release(store->lock, locked TSRMLS_CC);
		} else result = FAILURE;
	}
	return result;
}
/* }}} */

/* {{{ isset helper for handlers */
zend_bool pthreads_store_isset(pthreads_store store, char *key, int keyl, int has_set_exists TSRMLS_DC) {
	zend_bool locked = 0, isset = 0;
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
			pthreads_storage *storage;
			if (zend_ts_hash_find(&store->table, key, keyl, (void**)&storage)==SUCCESS) {
				isset=((*storage)->exists);
			}
			pthreads_lock_release(store->lock, locked TSRMLS_CC);
		}
	}
	return isset;
} /* }}} */

/* {{{ read a value from store */
int pthreads_store_read(pthreads_store store, char *key, int keyl, zval **read TSRMLS_DC) {
	zend_bool locked = 0;
	int result = FAILURE;
	if (store) {
		if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
			pthreads_storage *storage = NULL;
			if (zend_ts_hash_find(&store->table, key, keyl, (void**)&storage)==SUCCESS && storage) {
				MAKE_STD_ZVAL(*read);
				if ((result = pthreads_store_convert(*storage, *read TSRMLS_CC))!=SUCCESS) {
					FREE_ZVAL(*read);
				} else Z_SET_REFCOUNT_PP(read, 0);
			}
			pthreads_lock_release(store->lock, locked TSRMLS_CC);
		}
	}
	return result;
} /* }}} */

/* {{{ write a value to store */
int pthreads_store_write(pthreads_store store, char *key, int keyl, zval **write TSRMLS_DC) {
	int result = FAILURE;
	zend_bool locked;
	uint refcount = Z_REFCOUNT_PP(write);
	if (store) {
		pthreads_storage storage = pthreads_store_create(*write, 1 TSRMLS_CC);
		if (storage) {
			if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
				if (zend_ts_hash_update(&store->table, key, keyl, (void**) &storage, sizeof(pthreads_storage), NULL)==SUCCESS) {
					if (zend_ts_hash_exists(&store->event, key, keyl)) {
						pthreads_synchro *psync;
						if (zend_ts_hash_find(&store->event, key, keyl, (void**)&psync)==SUCCESS) {
							pthreads_synchro_notify(*psync TSRMLS_CC);
						}
					}
					result = SUCCESS;
				} else free(store);
				pthreads_lock_release(store->lock, locked TSRMLS_CC);
			}
		}
	}
	return result;
} /* }}} */

/* {{{ wait for a value to be set on buffer */
zend_bool pthreads_store_wait(pthreads_store store, char *key, int keyl, ulong timeout TSRMLS_DC) {
	zend_bool isset = 0, locked;
	
	if (store) {
		pthreads_synchro event = NULL, *pevent = NULL;
		
		if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
			if (!zend_ts_hash_exists(&store->event, key, keyl)) {
				event = pthreads_synchro_alloc(TSRMLS_C);
				if (event) {
					if (zend_ts_hash_update(
						&store->event, 
						key, keyl, 
						(void**)&event, sizeof(pthreads_synchro*), NULL
					)!=SUCCESS) {
						pthreads_synchro_free(
							event TSRMLS_CC
						);
						event = NULL;
						
						goto unlock;
					}
				}
			} else {
				if (zend_ts_hash_find(&store->event, key, keyl, (void**)&pevent)==SUCCESS) {
					event = *pevent;
				}
			}
unlock:
			pthreads_lock_release(store->lock, locked TSRMLS_CC);
		}
		
		if (event) {
			if (!(isset=pthreads_store_isset(store, key, keyl, 2 TSRMLS_CC))) {
				pthreads_synchro_wait_ex(event, timeout TSRMLS_CC);
			} else isset = 1;
			if (isset)
				pthreads_synchro_notify(event TSRMLS_CC);
			isset = pthreads_store_isset(store, key, keyl, 2 TSRMLS_CC);
		} else zend_error(E_ERROR, "event not created, pthreads bailing ...");
	}
	return isset;
} /* }}} */

/* {{{ seperate a zval using internals */
int pthreads_store_separate(zval * pzval, zval **separated, zend_bool allocate, zend_bool complex TSRMLS_DC) {
	int result = FAILURE;
	pthreads_storage storage;
	
	if (pzval) {	
		storage = pthreads_store_create(pzval, complex TSRMLS_CC);
		if (storage) {
			if (allocate) {
				MAKE_STD_ZVAL(*separated);
			}

			result = pthreads_store_convert(storage, *separated TSRMLS_CC);

			if (result == SUCCESS)
				pthreads_store_storage_dtor(&storage);
		}
	}
	return result;
} /* }}} */

/* {{{ copy store to hashtable */
void pthreads_store_tohash(pthreads_store store, HashTable *hash TSRMLS_DC) {

	zend_bool locked;
	TsHashTable *safe = &store->table;

	if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {

		HashTable *stored = TS_HASH(safe);
		HashPosition position;
		pthreads_storage *storage;

		for (zend_hash_internal_pointer_reset_ex(stored, &position);
			zend_hash_get_current_data_ex(stored, (void**) &storage, &position)==SUCCESS;
			zend_hash_move_forward_ex(stored, &position)) {	
			
			char *name;
			uint nlength;
			ulong idx;

			if (zend_hash_get_current_key_ex(stored, &name, &nlength, &idx, 0, &position)==HASH_KEY_IS_STRING) {
				zval *pzval;

				MAKE_STD_ZVAL(pzval);

				if (pthreads_store_convert((*storage), pzval TSRMLS_CC)!=SUCCESS) {
					ZVAL_NULL(pzval);
				} 

				zend_hash_update(hash, name, nlength+1, &pzval, sizeof(zval), NULL);
			}
		}
		pthreads_lock_release(store->lock, locked TSRMLS_CC);
	}
	
} /* }}} */

/* {{{ free store storage for a thread */
void pthreads_store_free(pthreads_store store TSRMLS_DC){
	if (store) {
		zend_bool locked;
		if (pthreads_lock_acquire(store->lock, &locked TSRMLS_CC)) {
			zend_ts_hash_destroy(&store->table);
			zend_ts_hash_destroy(&store->event);
			pthreads_lock_release(store->lock, locked TSRMLS_CC);
		}
		pthreads_lock_free(store->lock TSRMLS_CC);
		free(store);
	}
} /* }}} */

/* {{{ Will storeize the zval into a newly allocated buffer which must be free'd by the caller */
static pthreads_storage pthreads_store_create(zval *unstore, zend_bool complex TSRMLS_DC){					
	pthreads_storage storage;
	
	if (unstore) {
		/*
		* Make an exact copy of the store data for internal storage
		*/
		storage = (pthreads_storage) calloc(1, sizeof(*storage));
		if (storage) {
			storage->length = 0;
			
			switch((storage->type = Z_TYPE_P(unstore))){
				case IS_NULL:
					storage->data = NULL;
					storage->exists = 0;
				break;
				
				case IS_STRING: {
					if ((storage->length = Z_STRLEN_P(unstore))) {
						storage->data = (char*) calloc(1, storage->length+1);
						memmove(
							storage->data, (const void*) Z_STRVAL_P(unstore), storage->length
						);
						storage->exists = (storage->length > 0);
					} else {
						storage->data = NULL;
					}
				} break;
				
				case IS_BOOL:
				case IS_LONG: {
					storage->exists = ((storage->lval=Z_LVAL_P(unstore)) > 0L);
				} break;

				case IS_RESOURCE: {
					if (complex) {
						pthreads_resource resource = calloc(1, sizeof(*resource));
						if (resource) {
							resource->scope = EG(scope);
							resource->ls = TSRMLS_C;
						
							storage->lval = Z_RESVAL_P(unstore);
							storage->exists = 1;
							storage->data = resource;

							zend_list_addref(Z_RESVAL_P(unstore));
						}
					} else {
						storage->exists = 0;
						storage->type = IS_NULL;
					}
				} break;
				
				case IS_DOUBLE: storage->exists = ((storage->dval=Z_DVAL_P(unstore)) > 0.0); break;
				
				case IS_OBJECT:
				case IS_ARRAY: {
					if (pthreads_store_tostring(unstore, (char**) &storage->data, &storage->length, complex TSRMLS_CC)==SUCCESS) {
						if (storage->type==IS_ARRAY) {
							storage->exists = zend_hash_num_elements(Z_ARRVAL_P(unstore));
						} else storage->exists = 1;
					}
				} break;
				
				default: storage->exists = 0;
			}
		}
	}
	
	return storage;														
}
/* }}} */

/* {{{ Will unstoreize data into the allocated zval passed */
static int pthreads_store_convert(pthreads_storage storage, zval *pzval TSRMLS_DC){
	int result = SUCCESS;
	ulong refcount = Z_REFCOUNT_P(pzval);
	
	if (storage) {
		switch(storage->type) {
			case IS_STRING:
				if (storage->data && storage->length) {
					ZVAL_STRINGL(pzval, (char*)storage->data, storage->length, 1); 
				} else ZVAL_EMPTY_STRING(pzval);
			break;
			
			case IS_NULL: ZVAL_NULL(pzval); break;
			case IS_BOOL: ZVAL_BOOL(pzval, storage->lval); break;
			case IS_LONG: ZVAL_LONG(pzval, storage->lval); break;
			case IS_DOUBLE: ZVAL_DOUBLE(pzval, storage->dval); break;
			case IS_RESOURCE: {	
				pthreads_resource resource = (pthreads_resource) storage->data;
				if (EG(scope) != resource->scope) {
					zend_rsrc_list_entry *original;
					if (resource->ls != TSRMLS_C && zend_hash_index_find(&PTHREADS_EG(resource->ls, regular_list), storage->lval, (void**)&original)==SUCCESS) {
							
						zend_bool found = 0;
						int existed = 0;
						{
							zend_rsrc_list_entry *search;
							HashPosition position;
							for(zend_hash_internal_pointer_reset_ex(&EG(regular_list), &position);
								zend_hash_get_current_data_ex(&EG(regular_list), (void**) &search, &position)==SUCCESS;
								zend_hash_move_forward_ex(&EG(regular_list), &position)) {			
								if (search->ptr == original->ptr) {
									found=1;
									existed++;
									break;
								} else ++existed;
							}
						}
						
						if (!found) {
							PTHREAD object = PTHREADS_FETCH_FROM(EG(This));
							
							zend_rsrc_list_entry create;
							{
								int created;
								
								create.type = original->type;
								create.ptr = original->ptr;
								create.refcount = 1;
								created=zend_hash_next_free_element(&EG(regular_list));

								if (zend_hash_index_update(
									&EG(regular_list), created, (void*) &create, sizeof(zend_rsrc_list_entry), NULL
								)==SUCCESS) {
									ZVAL_RESOURCE(pzval, created);
									pthreads_resources_keep(
										object->resources, &create, resource TSRMLS_CC
									);
								} else ZVAL_NULL(pzval);
							}
						} else {
							ZVAL_RESOURCE(pzval, existed);
							zend_list_addref(Z_RESVAL_P(pzval));
						}
					} else {
						ZVAL_RESOURCE(pzval, storage->lval);
						zend_list_addref(Z_RESVAL_P(pzval));
					}
				} else {
					ZVAL_RESOURCE(pzval, storage->lval);
					zend_list_addref(Z_RESVAL_P(pzval));
				}
			} break;
			case IS_ARRAY:
			case IS_OBJECT: {
				result = pthreads_store_tozval(
					pzval, 
					(char*) storage->data, 
					storage->length TSRMLS_CC
				);
			} break;
			
			default: ZVAL_NULL(pzval);
		}	
	}
	return result;
}
/* }}} */

/* {{{ zval to string */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength, zend_bool complex TSRMLS_DC) {
	int result = FAILURE;
	if (pzval && (Z_TYPE_P(pzval) != IS_OBJECT || Z_OBJ_P(pzval))) {
		smart_str *psmart = (smart_str*) calloc(1, sizeof(smart_str));
		if (psmart) {
			if (!complex && (Z_TYPE_P(pzval) == IS_OBJECT || Z_TYPE_P(pzval) == IS_ARRAY)) {
				pthreads_store_remove_resources_recursive(&pzval TSRMLS_CC);
			}
			
			{
				if ((Z_TYPE_P(pzval) != IS_OBJECT) ||
					(Z_OBJCE_P(pzval)->serialize != zend_class_serialize_deny)) {
					php_serialize_data_t vars;
					PHP_VAR_SERIALIZE_INIT(vars);
					php_var_serialize(						
						psmart,
						&pzval, 
						&vars TSRMLS_CC
					);
					PHP_VAR_SERIALIZE_DESTROY(vars);
				}
			}
		
			*slength = psmart->len;
			if (psmart->len) {
				*pstring = calloc(1, psmart->len+1);
				if (*pstring) {
					memmove(
						(char*) *pstring, (const void*) psmart->c, psmart->len
					);
					result = SUCCESS;
				}
			} else *pstring = NULL;
			smart_str_free(psmart);	
			free(psmart);
		}
	}
	return result;
} /* }}} */

/* {{{ string to zval */
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength TSRMLS_DC) {
	int result = SUCCESS;
	ulong refcount = Z_REFCOUNT_P(pzval);
	if (pstring) {
		const unsigned char* pointer = (const unsigned char*) pstring;
		if (pointer) {
			/*
			* Populate PHP storage from pthreads_storage
			*/
			{
				php_unserialize_data_t vars;
				PHP_VAR_UNSERIALIZE_INIT(vars);
				if (!php_var_unserialize(&pzval, &pointer, pointer+slength, &vars TSRMLS_CC)) {
					result = FAILURE;
					zval_dtor(pzval);
				}							
				PHP_VAR_UNSERIALIZE_DESTROY(vars);
			}
			if (pzval && refcount)
				Z_SET_REFCOUNT_P(pzval, refcount);
		} else result = FAILURE;
	} else result = FAILURE;
	
	return result;
} /* }}} */

/* {{{ set resources to NULL for non-complex types; helper-function for pthreads_store_remove_resources_recursive */
static int pthreads_store_remove_resources(zval **pzval TSRMLS_DC) {
	if (Z_TYPE_PP(pzval) == IS_RESOURCE) {
		Z_TYPE_PP(pzval) = IS_NULL;
	}
	return pthreads_store_remove_resources_recursive(pzval TSRMLS_CC);
} /* }}} */

/* {{{ check a handle is valid before reading it */
static int pthreads_store_validate_resource(zval **pzval TSRMLS_DC) {	
	return (EG(objects_store).top > Z_OBJ_HANDLE_PP(pzval));
} /* }}} */

/* {{{ set corrupt objects (like mysqli after thread duplication) to NULL and recurse */
static int pthreads_store_remove_resources_recursive(zval **pzval TSRMLS_DC) {
	int is_temp;

	HashTable *thash = NULL;
	switch (Z_TYPE_PP(pzval)) {
		case IS_ARRAY:
			thash = Z_ARRVAL_PP(pzval);

		case IS_OBJECT:
			if (thash == NULL) {
				if (!pthreads_store_validate_resource(pzval TSRMLS_CC) || !Z_OBJ_P(*pzval)) {
					GC_REMOVE_ZVAL_FROM_BUFFER(*pzval);
					Z_TYPE_PP(pzval) = IS_NULL;
					return ZEND_HASH_APPLY_KEEP;
				}
				thash = Z_OBJDEBUG_PP(pzval, is_temp);
			}

			if (thash) {
				zend_hash_apply(thash, (apply_func_t)pthreads_store_remove_resources TSRMLS_CC);
			}
			
		break;
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ Will free store element */
static void pthreads_store_storage_dtor (pthreads_storage *storage){
	if (storage && (*storage)) {
		switch ((*storage)->type) {
			case IS_OBJECT:
			case IS_STRING:
			case IS_ARRAY:
			case IS_RESOURCE:
				if ((*storage)->data) {
					free((*storage)->data);
				}
			break;
		}
		
		free((*storage));
	}
} /* }}} */

/* {{{ Will free store event */
static void pthreads_store_event_dtor (pthreads_synchro *psync){
	TSRMLS_FETCH();
	if (psync && (*psync)) {
		pthreads_synchro_free(*psync TSRMLS_CC);
	}
} /* }}} */

#endif
