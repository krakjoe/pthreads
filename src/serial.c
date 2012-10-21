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

extern pthread_mutexattr_t defmutex;


/* {{{ element dtor */
static void pthreads_serial_store_dtor (void **element){
	if (*element) {
		char * buffer = (char*) *element;
		
		if (buffer) {
			free(buffer);
		}
	}
} /* }}} */

/* {{{ allocate serial storage for a thread */
pthreads_serial pthreads_serial_alloc(TSRMLS_D) {
	pthreads_serial serial = calloc(1, sizeof(*serial));
	
	if (serial) {
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
				(*acquired) = 1;
				return 1;
			case EDEADLK:
				(*acquired) = 0;
				return 1;
			default:
				/* report possible fatality */
				(*acquired) = 0;
				return 0;
		}
	} else return 0;
} /* }}} */

/* {{{ tell if the serial buffer contains a specific value */
int pthreads_serial_contains(pthreads_serial serial, char *key, int keyl TSRMLS_DC) {
	int contains = 0, locked = 0;
	
	if (serial) {
		if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
			contains = zend_ts_hash_exists(
				&serial->store, key, keyl
			);
			pthreads_serial_unlock(serial, &locked TSRMLS_CC);
		}
	}
	return contains;
} /* }}} */

/* {{{ delete a value from the serial buffer */
int pthreads_serial_delete(pthreads_serial serial, char *key, int keyl TSRMLS_DC) {
	int result = FAILURE, locked = 0;
	
	if (serial) {
		if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
			result = zend_ts_hash_del(
				&serial->store, key, keyl
			);
			pthreads_serial_unlock(serial, &locked TSRMLS_CC);
		}
	}
	return result;
}
/* }}} */

/* {{{ read a value from serial input */
int pthreads_serial_read(pthreads_serial serial, char *key, int keyl, zval **read TSRMLS_DC) {
	char **store = NULL;
	int result = FAILURE, locked = 0;
	
	if (serial) {
		if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
			if (zend_ts_hash_exists(&serial->store, key, keyl)) {
				if (zend_ts_hash_find(&serial->store, key, keyl, (void**)&store)==SUCCESS) {
					MAKE_STD_ZVAL(*read);
					if (pthreads_unserialize_into(*store, *read TSRMLS_CC)==SUCCESS) {
						result = SUCCESS;
					} else FREE_ZVAL(*read);
				}
			}
			pthreads_serial_unlock(serial, &locked TSRMLS_CC);
		} else { /* report error */ }
	} else { /* report error */ }
	return result;
} /* }}} */

/* {{{ write a value to serial output */
int pthreads_serial_write(pthreads_serial serial, char *key, int keyl, zval **write TSRMLS_DC) {
	char *store = NULL;
	int result = FAILURE, locked = 0;
	
	if (serial) {
		store = pthreads_serialize(*write TSRMLS_CC);
		if (store) {
			if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
				if (zend_ts_hash_update(&serial->store, key, keyl, (void**) &store, sizeof(char *), NULL)==SUCCESS) {
					result = SUCCESS;
				} else free(store);
				pthreads_serial_unlock(serial, &locked TSRMLS_CC);
			} else { /* report error */ }
		} else { /* report error */ }
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
		int locked = 0;
		if (pthreads_serial_lock(serial, &locked TSRMLS_CC)) {
			zend_ts_hash_destroy(&serial->store);
			pthreads_serial_unlock(serial, &locked TSRMLS_CC);
		}
		pthread_mutex_destroy(&serial->lock);
		free(serial);
	}
} /* }}} */



/* {{{ Will serialize the zval into a newly allocated buffer which must be free'd by the caller */
char * pthreads_serialize(zval *unserial TSRMLS_DC){					
	char *result = NULL;
	
	if (unserial) {
		smart_str *output;
		php_serialize_data_t vars;
		
		PHP_VAR_SERIALIZE_INIT(vars);				
		output = (smart_str*) calloc(1, sizeof(smart_str));
		php_var_serialize(							
			output, 
			&unserial, 
			&vars TSRMLS_CC
		);
		PHP_VAR_SERIALIZE_DESTROY(vars);			
		result = (char*) calloc(1, output->len+1);	
		memcpy(result, output->c, output->len);
		smart_str_free(output);						
		free(output);	
	}							
	return result;														
}
/* }}} */

/* {{{ Will unserialize data into the allocated zval passed */
int pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC){
	if (serial) {
		const unsigned char *pointer = (const unsigned char *)serial;
		php_unserialize_data_t vars;
		
		PHP_VAR_UNSERIALIZE_INIT(vars);
		if (!php_var_unserialize(&result, &pointer, pointer+strlen(serial), &vars TSRMLS_CC)) {
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
			zval_dtor(result);
			zend_error(E_WARNING, "The thread attempted to declare properties (%ld bytes of %s) that do not support serialization", strlen(serial), serial);
			return FAILURE;
		} else { 
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		}
		
		return SUCCESS;														
	} else return SUCCESS;
}
/* }}} */

/* {{{ Will unserialze data into a newly allocated buffer which must be free'd by the caller */
zval *	pthreads_unserialize(char *serial TSRMLS_DC){					
	zval *result;
	ALLOC_INIT_ZVAL(result);
	
	if (pthreads_unserialize_into(serial, result TSRMLS_CC)==SUCCESS) {
			return result;												
	} else return NULL;
}
/* }}} */
#endif
