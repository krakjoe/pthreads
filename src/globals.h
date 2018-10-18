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
#ifndef HAVE_PTHREADS_GLOBALS_H
#define HAVE_PTHREADS_GLOBALS_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

/* {{{ pthreads_globals */
struct _pthreads_globals {
	/*
	* Initialized flag
	*/
	volatile zend_bool init;
	
	/*
	* Failed flag
	*/
	zend_bool failed;
	
	/*
	* Global Monitor
	*/
	pthreads_monitor_t *monitor;

	/*
	* Global/Default Resource Destructor
	*/
	dtor_func_t (default_resource_dtor);
	
	/*
	* Objects Cache
	*/
	HashTable objects;

	/**
	* Global class table
	 */
	HashTable postcompile;
	
	/*
	* Global compile hook Monitor
	*/
	pthreads_monitor_t *compile_hook_monitor;

	/*
	* Default static props cache
	*/
	HashTable default_static_props;

	/*
	* High Frequency Strings
	*/
	struct _strings {
		zend_string *run;
		zval         worker;
		struct _session {
			zend_string *cache_limiter;
			zend_string *use_cookies;
		} session;
	} strings;
}; /* }}} */

extern struct _pthreads_globals pthreads_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads)

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(void *address); /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_connect(zend_ulong address, zend_class_entry *ce, zval *object); /* }}} */

/* {{{ */
void* pthreads_globals_object_alloc(size_t length); /* }}} */

/* {{{ initialize (true) globals */
zend_bool pthreads_globals_init(); /* }}} */

/* {{{ acquire global lock */
zend_bool pthreads_globals_lock(); /* }}} */

/* {{{ release global lock */
void pthreads_globals_unlock(); /* }}} */

/* {{{ acquire compile hook lock */
zend_bool pthreads_compile_hook_lock(); /* }}} */

/* {{{ release compile hook lock */
void pthreads_compile_hook_unlock(); /* }}} */

/* {{{ copy string to globals */
char *pthreads_global_string(char *strkey, int32_t keylen, zend_bool lower); /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(); /* }}} */

#endif /* HAVE_PTHREADS_GLOBAL_H */
