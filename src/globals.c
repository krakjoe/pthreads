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
#ifndef HAVE_PTHREADS_GLOBALS
#define HAVE_PTHREADS_GLOBALS

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

struct _pthreads_globals pthreads_globals;

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef PTHREADS_GTSRMLS_C
#	define PTHREADS_GTSRMLS_C (TSRMLS_C) ? TSRMLS_C : (void***) &pthreads_globals
#endif

#ifndef PTHREADS_GLOBAL_LOCK_ARGS
#	define PTHREADS_GLOBAL_LOCK_ARGS \
		locked, PTHREADS_GTSRMLS_C
#endif

/* {{{ */
void pthreads_global_string_free(void *strkey) {
   free((char*)*((char**)strkey)); 
} /* }}} */

/* {{{ pthreads_globals_init */
zend_bool pthreads_globals_init(TSRMLS_D){
	if (!PTHREADS_G(init)&&!PTHREADS_G(failed)) {
		PTHREADS_G(init)=1;
		if (!(PTHREADS_G(lock)=pthreads_lock_alloc(PTHREADS_GTSRMLS_C)))
			PTHREADS_G(failed)=1;
		if (PTHREADS_G(failed)) {
		    PTHREADS_G(init)=0;
		} else {
		    zend_hash_init(
		        &PTHREADS_G(strings), 64, NULL, (dtor_func_t) pthreads_global_string_free, 1);
		    zend_hash_init(
		    	&PTHREADS_G(objects), 64, NULL, NULL, 1);
		}

		return PTHREADS_G(init);
	} else return 0;
} /* }}} */

/* {{{ pthreads_globals_lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC){
	return pthreads_lock_acquire(PTHREADS_G(lock), PTHREADS_GLOBAL_LOCK_ARGS);
} /* }}} */

/* {{{ pthreads_globals_unlock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC) {
	pthreads_lock_release(PTHREADS_G(lock), PTHREADS_GLOBAL_LOCK_ARGS);
} /* }}} */

/* {{{ pthreads_global_string */
char *pthreads_global_string(char *strkey, zend_uint keylen, zend_bool lower TSRMLS_DC) {   
    char **created = NULL;
    
    if (zend_hash_find(&PTHREADS_G(strings), strkey, keylen, (void**)&created) != SUCCESS) {
        char *creating = malloc(keylen+1);
        
        if (creating) {
            if (lower) {
                zend_str_tolower_copy(
                    creating, strkey, keylen);
            } else strcpy(creating, strkey);
            
            zend_hash_update(
                &PTHREADS_G(strings), strkey, keylen, (void**)&creating, sizeof(void*), NULL);
                
            return creating;
        }
    }
    
    return *created;
} /* }}} */

/* {{{ pthreads_globals_object */
void* pthreads_globals_object_alloc(size_t length TSRMLS_DC) {
	zend_bool locked = 0;
	void *bucket     = calloc(1, length);
	
	if (!bucket)
		return NULL;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		zend_hash_index_update(
			&PTHREADS_G(objects),
			(ulong) bucket, (void**)&bucket, length, NULL);
		
		pthreads_globals_unlock(locked TSRMLS_CC);
	}
	
	return bucket;
} /* }}} */

/* {{{ pthreads_globals_validate */
zend_bool pthreads_globals_object_validate(zend_ulong address TSRMLS_DC) {
	zend_bool valid = 0;
	zend_bool locked = 0;
	if (!address)
		return valid;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		valid = zend_hash_index_exists(
			&PTHREADS_G(objects), address);
		pthreads_globals_unlock(locked TSRMLS_CC);
	}
	
	return valid;
} /* }}} */

/* {{{ pthreads_globals_delete */
zend_bool pthreads_globals_object_delete(void *address TSRMLS_DC) {
	zend_bool deleted = 0;
	zend_bool locked = 0;
	
	if (!address)
		return deleted;
	
	if (pthreads_globals_lock(&locked TSRMLS_CC)) {
		deleted = zend_hash_index_del(
			&PTHREADS_G(objects), (zend_ulong) address);
		if (deleted) {
			free (address);
		}
		pthreads_globals_unlock(locked TSRMLS_CC);
	}
	
	return deleted;
} /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(TSRMLS_D) {
	if (PTHREADS_G(init)) {
	
	    PTHREADS_G(init)=0;
	    PTHREADS_G(failed)=0;

	    zend_hash_destroy(
	        &PTHREADS_G(strings));
	    
	    zend_hash_destroy(
	    	&PTHREADS_G(objects));
	    
		pthreads_lock_free(PTHREADS_G(lock) TSRMLS_CC);
	}
} /* }}} */
#endif
