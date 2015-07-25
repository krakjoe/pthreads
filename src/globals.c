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
#ifndef HAVE_PTHREADS_GLOBALS
#define HAVE_PTHREADS_GLOBALS

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

struct _pthreads_globals pthreads_globals;

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef PTHREADS_G
#	define PTHREADS_G () ?  : (void***) &pthreads_globals
#endif

/* {{{ */
zend_bool pthreads_globals_init(){
	if (!PTHREADS_G(init)&&!PTHREADS_G(failed)) {
		PTHREADS_G(init)=1;
		if (!(PTHREADS_G(lock)=pthreads_lock_alloc()))
			PTHREADS_G(failed)=1;
		if (PTHREADS_G(failed)) {
		    PTHREADS_G(init)=0;
		} else {
		    zend_hash_init(
		    	&PTHREADS_G(objects), 64, NULL, (dtor_func_t) NULL, 1);
		}

		return PTHREADS_G(init);
	} else return 0;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_lock(zend_bool *locked){
	return pthreads_lock_acquire(PTHREADS_G(lock), locked);
} /* }}} */

/* {{{ */
void pthreads_globals_unlock(zend_bool locked) {
	pthreads_lock_release(PTHREADS_G(lock), locked);
} /* }}} */

/* {{{ */
void* pthreads_globals_object_alloc(size_t length) {
	zend_bool locked = 0;
	void *bucket     = (void*) ecalloc(1, length);
	
	if (!bucket)
		return NULL;
	
	if (pthreads_globals_lock(&locked)) {
		zend_hash_index_update_ptr(
			&PTHREADS_G(objects),
			(zend_ulong) bucket, bucket);
		
		pthreads_globals_unlock(locked);
	}
	
	return bucket;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_validate(zend_ulong address) {
	zend_bool valid = 0;
	zend_bool locked = 0;
	if (!address)
		return valid;
	
	if (pthreads_globals_lock(&locked)) {
		valid = zend_hash_index_exists(
			&PTHREADS_G(objects), address);
		pthreads_globals_unlock(locked);
	}
	
	return valid;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(void *address) {
	zend_bool deleted = 0;
	zend_bool locked = 0;
	
	if (!address)
		return deleted;
	
	if (pthreads_globals_lock(&locked)) {
		deleted = zend_hash_index_del(
			&PTHREADS_G(objects), (zend_ulong) address);
		pthreads_globals_unlock(locked);
	}
	
	return deleted;
} /* }}} */

/* {{{ */
void pthreads_globals_shutdown() {
	if (PTHREADS_G(init)) {
		PTHREADS_G(init)=0;
		PTHREADS_G(failed)=0;
		zend_hash_destroy(&PTHREADS_G(objects));
		pthreads_lock_free(PTHREADS_G(lock));
	}
} /* }}} */
#endif
