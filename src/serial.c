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
#ifndef HAVE_PTHREADS_SERIAL
#define HAVE_PTHREADS_SERIAL

/*
* @TODO
*	1. write in support for other serialization methods like msgpack
*/

#ifndef HAVE_PTHREADS_SERIAL_H
#	include <src/serial.h>
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

/* {{{ serial buffer element structure */
typedef struct _pthreads_storage {
	void		*serial;	/* serial data */
	size_t 		length;		/* serial length */
	size_t 		strlen;		/* length of original string */
	zend_uchar 	type;		/* type of data */
	zend_bool 	exists;		/* preset exists to aid in quicker isset() */
} *pthreads_storage; /* }}} */

/* {{{ statics */
static pthreads_storage pthreads_serialize(zval *pzval TSRMLS_DC);
static int pthreads_unserialize(pthreads_storage storage, zval *pzval TSRMLS_DC);
static void pthreads_serial_store_dtor (void **element);
/* }}} */

/* {{{ allocate serial storage for a thread */
pthreads_serial pthreads_serial_alloc(TSRMLS_D) {
	pthreads_serial serial = calloc(1, sizeof(*serial));
	
	if (serial) {
		serial->line = NULL;
		if (zend_ts_hash_init(&serial->store, 32, NULL, (dtor_func_t) pthreads_serial_store_dtor, 1)==SUCCESS){
			if (pthread_mutex_init(&serial->lock, &defmutex)==SUCCESS) {
				return serial;
			} else zend_ts_hash_destroy(&serial->store);
		} else free(serial);
	}
	return NULL;
} /* }}} */

/* {{{ lock serial buffer */
int pthreads_serial_lock(pthreads_serial serial, int *acquired TSRMLS_DC) {
	if (serial) {
		switch (pthread_mutex_lock(&serial->lock)) {
			case SUCCESS:
				return (((*acquired)=1)==1);
				
			case EDEADLK:
				return (((*acquired)=0)==0);
				
			default:
				return (((*acquired)=0)==1);
		}
	} else return 0;
} /* }}} */

/* {{{ extend serial line */
int pthreads_serial_extend(pthreads_serial serial, pthreads_serial line TSRMLS_DC) {
	int result = SUCCESS;
	if (serial && line) {
		if (serial != line) {
			int locked[2] = {0, 0};
			if (pthreads_serial_lock(serial, &locked[0] TSRMLS_CC)) {
				if (pthreads_serial_lock(line, &locked[1] TSRMLS_CC)) {
					serial->line = &line;
					pthreads_serial_unlock(line, &locked[1] TSRMLS_CC);
				} else result = FAILURE;
				pthreads_serial_unlock(serial, &locked[0] TSRMLS_CC);
			} else result = FAILURE;
		} else result = FAILURE;
	} else result = FAILURE;
	return result;
} /* }}} */

/* {{{ tell if the serial buffer contains a specific value */
int pthreads_serial_contains(pthreads_serial serial, char *key, int keyl, pthreads_serial *line TSRMLS_DC) {
	int contains = 0, locked = 0;
	
	if (serial) {
		pthreads_serial *pointer = &serial;
		
		do {
			if (pthreads_serial_lock((*pointer), &locked TSRMLS_CC)) {
				if (zend_ts_hash_exists(&((*pointer)->store), key, keyl)) {
					contains = 1;
				}
				pthreads_serial_unlock((*pointer), &locked TSRMLS_CC);
			} else break;
			
		} while(!contains && (pointer=((*pointer)->line))!=NULL);

		if (contains) {
			*line = *pointer;
		}
	}
	
	return contains;
} /* }}} */

/* {{{ delete a value from the serial buffer */
int pthreads_serial_delete(pthreads_serial serial, char *key, int keyl TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (serial) {
		pthreads_serial *pointer = &serial;
		do {
			if (pthreads_serial_lock((*pointer), &locked TSRMLS_CC)) {
				if (zend_ts_hash_exists(&((*pointer)->store), key, keyl)) {
					if (zend_ts_hash_del(&((*pointer)->store), key, keyl)!=SUCCESS) {
						result = FAILURE;
					} else result = SUCCESS;
				} else result = SUCCESS;
				pthreads_serial_unlock((*pointer), &locked TSRMLS_CC);
			} else break;
		} while(result != FAILURE && (pointer=((*pointer)->line))!=NULL);
	}
	return result;
}
/* }}} */

/* {{{ isset proxy for handlers to avoid unserializing members when isset() is used */
int pthreads_serial_isset(pthreads_serial serial, char *key, int keyl, int has_set_exists TSRMLS_DC) {
	int locked = 0, result = 0;
	
	if (serial) {
		pthreads_serial line;
		if (pthreads_serial_contains(serial, key, keyl, &line TSRMLS_CC)) {
			if (pthreads_serial_lock(line, &locked TSRMLS_CC)) {
				pthreads_storage *store;
				if (zend_ts_hash_find(&line->store, key, keyl, (void**)&store)==SUCCESS) {
					result=((*store)->exists);
				}
				pthreads_serial_unlock(line, &locked TSRMLS_CC);
			}
		}
	}
	
	return result;
} /* }}} */

/* {{{ read a value from serial input */
int pthreads_serial_read(pthreads_serial serial, char *key, int keyl, zval **read TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (serial) {
		pthreads_serial line;
		if (pthreads_serial_contains(serial, key, keyl, &line TSRMLS_CC)) {
			if (pthreads_serial_lock(line, &locked TSRMLS_CC)) {
				pthreads_storage *store;

				if (zend_ts_hash_find(&line->store, key, keyl, (void**)&store)==SUCCESS) {
					ALLOC_ZVAL(*read);
					if ((result = pthreads_unserialize(*store, *read TSRMLS_CC))!=SUCCESS) {
						zend_error_noreturn(E_WARNING, "pthreads failed to read %s from storage (%s)", key, (*store)->serial);
						FREE_ZVAL(*read);
					} else Z_SET_REFCOUNT_PP(read, 0);
				} else zend_error_noreturn(E_WARNING, "pthreads failed to find %s in storage", key);
				
				pthreads_serial_unlock(line, &locked TSRMLS_CC);
			}
		} else { /* not found */ zend_error_noreturn(E_WARNING, "pthreads failed to find %s in storage", key); }
	} else { /* report error */ }

	return result;
} /* }}} */

/* {{{ write a value to serial output */
int pthreads_serial_write(pthreads_serial serial, char *key, int keyl, zval **write TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (serial) {
		pthreads_storage store = pthreads_serialize(*write TSRMLS_CC);
		if (store) {
			if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
				if (zend_ts_hash_update(&serial->store, key, keyl, (void**) &store, sizeof(pthreads_storage), NULL)==SUCCESS) {
					result = SUCCESS;
				} else free(store);
				pthreads_serial_unlock(serial, &locked TSRMLS_CC);
			} else { /* report error */ }
		} else { /* serialization error */ }
	} else { /* report error */ }
	return result;
} /* }}} */

/* {{{ unlock serial buffer */
int pthreads_serial_unlock(pthreads_serial serial, int *acquired TSRMLS_DC) {
	if (serial) {
		if ((*acquired)) {
			return pthread_mutex_unlock(&serial->lock);
		} else return 1;
	} else return 0;
} /* }}} */

/* {{{ free serial storage for a thread */
void pthreads_serial_free(pthreads_serial serial TSRMLS_DC){
	if (serial) {
		int locked;
		
		if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
			zend_ts_hash_destroy(&serial->store);
			pthreads_serial_unlock(serial, &locked TSRMLS_CC);
		}
		pthread_mutex_destroy(&serial->lock);
		free(serial);
	}
} /* }}} */

/* {{{ Will serialize the zval into a newly allocated buffer which must be free'd by the caller */
static pthreads_storage pthreads_serialize(zval *unserial TSRMLS_DC){					
	pthreads_storage storage;
	
	if (unserial) {
		smart_str *output = (smart_str*) calloc(1, sizeof(smart_str));
		
		if (output) {
			/*
			* Populate PHP storage
			*/
			{
				/*
				* This might seem like the long way round
				* pthreads_storage can be cast to a smart_str pointer but zend doesn't like it
				*/
				{
					php_serialize_data_t vars;
					PHP_VAR_SERIALIZE_INIT(vars);	
					php_var_serialize(							
						output, 
						&unserial, 
						&vars TSRMLS_CC
					);
					PHP_VAR_SERIALIZE_DESTROY(vars);
				}
			}
			
			/*
			* Populate pthreads storage
			*/
			if (output) {
			
				/*
				* Make an exact copy of the serial data for internal storage
				*/
				storage = (pthreads_storage) calloc(1, sizeof(*storage));
				if (storage) {
					storage->strlen = 0;
					storage->serial = (char*) calloc(1, output->len);
					if (storage->serial) {
						storage->length = output->len;
							
						/*
						* Set some storage flags
						*/
						switch((storage->type = Z_TYPE_P(unserial))){
							case IS_STRING:
								storage->strlen = Z_STRLEN_P(unserial);
								storage->exists = (storage->strlen > 0);
							break;
							
							case IS_LONG: storage->exists = (Z_LVAL_P(unserial) > 0L); break;
							case IS_DOUBLE: storage->exists = (Z_DVAL_P(unserial) > 0.0); break;
							case IS_ARRAY: storage->exists = zend_hash_num_elements(Z_ARRVAL_P(unserial)); break;
							
							default: storage->exists = 0;
						}
						
						/*
						* not memcpy or strndup or anything that cares about null termination
						*/
						memmove(
							(char*) storage->serial, (const void*) output->c, output->len
						);
					} else free(storage);
				}
				
				/*
				* Free PHP storage
				*/
				{
					smart_str_free(output);	
					free(output);
				}
			}
		}
	}
	
	return storage;														
}
/* }}} */

/* {{{ serialize instantce of a Thread, Worker or Stackable */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC) {
	return FAILURE;
} /* }}} */

/* {{{ unserialize an instance of a Thread, Worker or Stackable */
int pthreads_internal_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC) {
	return FAILURE;
} /* }}} */

/* {{{ Will unserialize data into the allocated zval passed */
static int pthreads_unserialize(pthreads_storage storage, zval *pzval TSRMLS_DC){
	int result = SUCCESS;
	
	if (storage) {
		const unsigned char* pointer = (const unsigned char*) storage->serial;
		
		if (pointer) {
			/*
			* Populate PHP storage from pthreads_storage
			*/
			{
				php_unserialize_data_t vars;
				PHP_VAR_UNSERIALIZE_INIT(vars);
				if (!php_var_unserialize(&pzval, &pointer, pointer+storage->length, &vars TSRMLS_CC)) {
					result = FAILURE;
					zval_dtor(pzval);
				}							
				PHP_VAR_UNSERIALIZE_DESTROY(vars);
			}
		}
	}
	return result;
}
/* }}} */

/* {{{ Will free serial element */
static void pthreads_serial_store_dtor (void **element){
	if (*element) {
		pthreads_storage storage = (pthreads_storage) *element;
		
		if (storage) {
			if (storage->serial) {
				free(storage->serial);
			}
			
			free(storage);
		}
	}
} /* }}} */

#endif
