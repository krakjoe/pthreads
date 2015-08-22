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

#ifndef PTHREADS_G
#	define PTHREADS_G () ?  : (void***) &pthreads_globals
#endif

/* {{{ */
zend_bool pthreads_globals_init(){
	if (!PTHREADS_G(init)&&!PTHREADS_G(failed)) {
		PTHREADS_G(init)=1;
		if (!(PTHREADS_G(monitor)=pthreads_monitor_alloc()))
			PTHREADS_G(failed)=1;
		if (PTHREADS_G(failed)) {
		    PTHREADS_G(init)=0;
		} else {
		    zend_hash_init(
		    	&PTHREADS_G(objects), 64, NULL, (dtor_func_t) NULL, 1);
		}

#define INIT_STRING(n, v) PTHREADS_G(strings).n = zend_string_init(v, 1)
		INIT_STRING(run, ZEND_STRL("run"));
		INIT_STRING(worker, ZEND_STRL("worker"));
		INIT_STRING(session.cache_limiter, ZEND_STRL("cache_limiter"));
		INIT_STRING(session.use_cookies, ZEND_STRL("use_cookies"));
#undef INIT_STRING

		return PTHREADS_G(init);
	} else return 0;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_lock(){
	return pthreads_monitor_lock(PTHREADS_G(monitor));
} /* }}} */

/* {{{ */
void pthreads_globals_unlock() {
	pthreads_monitor_unlock(PTHREADS_G(monitor));
} /* }}} */

/* {{{ */
void* pthreads_globals_object_alloc(size_t length) {
	zend_bool locked = 0;
	void *bucket     = (void*) ecalloc(1, length);
	
	if (!bucket)
		return NULL;
	
	if (pthreads_globals_lock()) {
		zend_hash_index_update_ptr(
			&PTHREADS_G(objects),
			(zend_ulong) bucket, bucket);
		pthreads_globals_unlock();
	}
	
	return bucket;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_validate(zend_ulong address) {
	zend_bool valid = 0;
	if (!address)
		return valid;
	
	if (pthreads_globals_lock()) {
		valid = zend_hash_index_exists(
			&PTHREADS_G(objects), address);
		pthreads_globals_unlock();
	}
	
	return valid;
} /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(void *address) {
	zend_bool deleted = 0;
	
	if (!address)
		return deleted;
	
	if (pthreads_globals_lock()) {
		deleted = zend_hash_index_del(
			&PTHREADS_G(objects), (zend_ulong) address);
		pthreads_globals_unlock();
	}
	
	return deleted;
} /* }}} */

/* {{{ */
void pthreads_globals_shutdown() {
	if (PTHREADS_G(init)) {
		PTHREADS_G(init)=0;
		PTHREADS_G(failed)=0;
		zend_hash_destroy(&PTHREADS_G(objects));
		zend_string_free(PTHREADS_G(strings).run);
		zend_string_free(PTHREADS_G(strings).worker);
		zend_string_free(PTHREADS_G(strings).session.cache_limiter);
		zend_string_free(PTHREADS_G(strings).session.use_cookies);
		pthreads_monitor_free(PTHREADS_G(monitor));
	}
} /* }}} */
#endif
