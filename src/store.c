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

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

/* {{{ element structure */
typedef struct _pthreads_storage {
	void		*data;		/* data */
	size_t 		length;		/* length */
	zend_uchar 	type;		/* type of data */
	zend_bool 	exists;		/* quicker isset() */
	long		lval;		/* long value */
	double		dval;		/* double value */
} *pthreads_storage; /* }}} */

/* {{{ statics */
static pthreads_storage pthreads_store_create(zval *pzval TSRMLS_DC);
static int pthreads_store_convert(pthreads_storage storage, zval *pzval TSRMLS_DC);
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength TSRMLS_DC);
static int pthreads_store_tozval(zval *pzval, char *pstring, size_t slength TSRMLS_DC);
static void pthreads_store_storage_dtor (pthreads_storage *element);
/* }}} */

/* {{{ allocate storage for an object */
pthreads_store pthreads_store_alloc(TSRMLS_D) {
	pthreads_store store = calloc(1, sizeof(*store));
	
	if (store) {
		store->line = NULL;
		if (zend_ts_hash_init(&store->table, 32, NULL, (dtor_func_t) pthreads_store_storage_dtor, 1)==SUCCESS){
			if (pthread_mutex_init(&store->lock, &defmutex)==SUCCESS) {
				return store;
			} else zend_ts_hash_destroy(&store->table);
		} else free(store);
	}
	return NULL;
} /* }}} */

/* {{{ lock buffer */
int pthreads_store_lock(pthreads_store store, int *acquired TSRMLS_DC) {
	if (store) {
		switch (pthread_mutex_lock(&store->lock)) {
			case SUCCESS:
				return (((*acquired)=1)==1);
				
			case EDEADLK:
				return (((*acquired)=0)==0);
				
			default:
				return (((*acquired)=0)==1);
		}
	} else return 0;
} /* }}} */

/* {{{ extend store line */
int pthreads_store_extend(pthreads_store store, pthreads_store line TSRMLS_DC) {
	int result = SUCCESS;
	if (store && line) {
		if (store != line) {
			int locked[2] = {0, 0};
			if (pthreads_store_lock(store, &locked[0] TSRMLS_CC)) {
				if (pthreads_store_lock(line, &locked[1] TSRMLS_CC)) {
					store->line = &line;
					pthreads_store_unlock(line, &locked[1] TSRMLS_CC);
				} else result = FAILURE;
				pthreads_store_unlock(store, &locked[0] TSRMLS_CC);
			} else result = FAILURE;
		} else result = FAILURE;
	} else result = FAILURE;
	return result;
} /* }}} */

/* {{{ tell if the store contains a specific value */
int pthreads_store_contains(pthreads_store store, char *key, int keyl, pthreads_store *line TSRMLS_DC) {
	int contains = 0, locked = 0;
	
	if (store) {
		pthreads_store *pointer = &store;
		
		do {
			if (pthreads_store_lock((*pointer), &locked TSRMLS_CC)) {
				if (zend_ts_hash_exists(&((*pointer)->table), key, keyl)) {
					contains = 1;
				}
				pthreads_store_unlock((*pointer), &locked TSRMLS_CC);
			} else break;
			
		} while(!contains && (pointer=((*pointer)->line))!=NULL);

		if (contains) {
			*line = *pointer;
		}
	}
	
	return contains;
} /* }}} */

/* {{{ delete a value from the store */
int pthreads_store_delete(pthreads_store store, char *key, int keyl TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (store) {
		pthreads_store *pointer = &store;
		do {
			if (pthreads_store_lock((*pointer), &locked TSRMLS_CC)) {
				if (zend_ts_hash_exists(&((*pointer)->table), key, keyl)) {
					if (zend_ts_hash_del(&((*pointer)->table), key, keyl)!=SUCCESS) {
						result = FAILURE;
					} else result = SUCCESS;
				} else result = SUCCESS;
				pthreads_store_unlock((*pointer), &locked TSRMLS_CC);
			} else break;
		} while(result != FAILURE && (pointer=((*pointer)->line))!=NULL);
	}
	return result;
}
/* }}} */

/* {{{ isset helper for handlers */
int pthreads_store_isset(pthreads_store store, char *key, int keyl, int has_set_exists TSRMLS_DC) {
	int locked = 0, result = 0;
	
	if (store) {
		pthreads_store line;
		if (pthreads_store_contains(store, key, keyl, &line TSRMLS_CC)) {
			if (pthreads_store_lock(line, &locked TSRMLS_CC)) {
				pthreads_storage *store;
				if (zend_ts_hash_find(&line->table, key, keyl, (void**)&store)==SUCCESS) {
					result=((*store)->exists);
				}
				pthreads_store_unlock(line, &locked TSRMLS_CC);
			}
		}
	}
	
	return result;
} /* }}} */

/* {{{ read a value from store */
int pthreads_store_read(pthreads_store store, char *key, int keyl, zval **read TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (store) {
		pthreads_store line;
		if (pthreads_store_contains(store, key, keyl, &line TSRMLS_CC)) {
			if (pthreads_store_lock(line, &locked TSRMLS_CC)) {
				pthreads_storage *store;

				if (zend_ts_hash_find(&line->table, key, keyl, (void**)&store)==SUCCESS) {
					ALLOC_ZVAL(*read);
					if ((result = pthreads_store_convert(*store, *read TSRMLS_CC))!=SUCCESS) {
						zend_error_noreturn(E_WARNING, "pthreads failed to read %s from storage (%s)", key, (*store)->data);
						FREE_ZVAL(*read);
					} else Z_SET_REFCOUNT_PP(read, 0);
				} else zend_error_noreturn(E_WARNING, "pthreads failed to find %s in storage", key);
				
				pthreads_store_unlock(line, &locked TSRMLS_CC);
			}
		} else { /* not found */ zend_error_noreturn(E_WARNING, "pthreads failed to find %s in storage", key); }
	} else { /* report error */ }

	return result;
} /* }}} */

/* {{{ write a value to store */
int pthreads_store_write(pthreads_store store, char *key, int keyl, zval **write TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (store) {
		pthreads_storage storage = pthreads_store_create(*write TSRMLS_CC);
		if (storage) {
			if (pthreads_store_lock(store, &locked TSRMLS_CC)) {
				if (zend_ts_hash_update(&store->table, key, keyl, (void**) &storage, sizeof(pthreads_storage), NULL)==SUCCESS) {
					result = SUCCESS;
				} else free(store);
				pthreads_store_unlock(store, &locked TSRMLS_CC);
			} else { /* report error */ }
		} else { /* storeization error */ }
	} else { /* report error */ }
	return result;
} /* }}} */

/* {{{ unlock store */
int pthreads_store_unlock(pthreads_store store, int *acquired TSRMLS_DC) {
	if (store) {
		if ((*acquired)) {
			return pthread_mutex_unlock(&store->lock);
		} else return 1;
	} else return 0;
} /* }}} */

/* {{{ copy a zval */
int pthreads_store_copy(zval *source, zval *destination TSRMLS_DC) {
	int result = SUCCESS;
	
	if (source) {
		ALLOC_ZVAL(destination);
		
		switch (Z_TYPE_P(source)) {
			case IS_NULL:
				ZVAL_NULL(destination);
			break;
			
			case IS_STRING: {
				ZVAL_STRINGL(destination, Z_STRVAL_P(source), Z_STRLEN_P(source), 1);
			} break;
			
			case IS_BOOL: ZVAL_BOOL(destination, Z_BVAL_P(destination)); break;
			case IS_LONG: ZVAL_LONG(destination, Z_LVAL_P(destination)); break;
			case IS_DOUBLE: ZVAL_DOUBLE(destination, Z_DVAL_P(destination)); break;
			
			case IS_OBJECT:
			case IS_ARRAY: {
				char *store;
				size_t slength;
				if (pthreads_store_tostring(source, &store, &slength TSRMLS_CC)==SUCCESS) {
					if (pthreads_store_tozval(destination, store, slength TSRMLS_CC)!=SUCCESS) {
						result = FAILURE;
					}
					free(store);
				} else result = FAILURE;
			} break;
		}
		
		if (Z_REFCOUNT_P(destination)==0)
			Z_ADDREF_P(destination);
		if (Z_ISREF_P(destination))
			Z_UNSET_ISREF_P(destination);
			
	} else {
		destination = NULL;
		result = FAILURE;
	}
	
	return result;
} /* }}} */

/* {{{ free store storage for a thread */
void pthreads_store_free(pthreads_store store TSRMLS_DC){
	if (store) {
		int locked;
		
		if (pthreads_store_lock(store, &locked TSRMLS_CC)) {
			zend_ts_hash_destroy(&store->table);
			pthreads_store_unlock(store, &locked TSRMLS_CC);
		}
		pthread_mutex_destroy(&store->lock);
		free(store);
	}
} /* }}} */

/* {{{ storeize instantce of a Thread, Worker or Stackable */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC) {
	return FAILURE;
} /* }}} */

/* {{{ unstoreize an instance of a Thread, Worker or Stackable */
int pthreads_internal_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC) {
	return FAILURE;
} /* }}} */

/* {{{ zval to string */
static int pthreads_store_tostring(zval *pzval, char **pstring, size_t *slength TSRMLS_DC) {
	smart_str *psmart = (smart_str*) calloc(1, sizeof(smart_str));
	int result = FAILURE;
	
	if (psmart) {
		php_serialize_data_t vars;
		PHP_VAR_SERIALIZE_INIT(vars);	
		php_var_serialize(							
			psmart, 
			&pzval, 
			&vars TSRMLS_CC
		);
		PHP_VAR_SERIALIZE_DESTROY(vars);
		
		if (psmart) {
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
		}
	}
	return result;
} /* }}} */

/* {{{ Will storeize the zval into a newly allocated buffer which must be free'd by the caller */
static pthreads_storage pthreads_store_create(zval *unstore TSRMLS_DC){					
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
					} else storage->data = NULL;
				} break;
				
				case IS_BOOL:
				case IS_LONG: {
					storage->exists = ((storage->lval=Z_LVAL_P(unstore)) > 0L);
				} break;
				
				case IS_DOUBLE: storage->exists = ((storage->dval=Z_DVAL_P(unstore)) > 0.0); break;
				
				case IS_OBJECT:
				case IS_ARRAY: {
					if (pthreads_store_tostring(unstore, (char**) &storage->data, &storage->length TSRMLS_CC)==SUCCESS) {
						if (storage->type==IS_ARRAY) {
							storage->exists = zend_hash_num_elements(Z_ARRVAL_P(unstore));
						} else storage->exists = 1;
					} else free(storage);
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
		Z_ADDREF_P(pzval);
	}
	return result;
}
/* }}} */

/* {{{ Will free store element */
static void pthreads_store_storage_dtor (pthreads_storage *storage){
	if (storage && (*storage)) {
		switch ((*storage)->type) {
			case IS_OBJECT:
			case IS_STRING:
			case IS_ARRAY:
				if ((*storage)->data) {
					free((*storage)->data);
				}
			break;
		}
		
		free((*storage));
	}
} /* }}} */

#endif
