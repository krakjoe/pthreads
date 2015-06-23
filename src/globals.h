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
#ifndef HAVE_PTHREADS_GLOBALS_H
#define HAVE_PTHREADS_GLOBALS_H

/*
* NOTES
* 1. pthreads cannot use the Zend implementation of globals, it makes for instability - we sometimes require a true global lock
*/
#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

/* {{{ pthreads_globals */
struct _pthreads_globals {
	/*
	* Global LS
	*/
	void*** global_tls;

	/*
	* Initialized flag
	*/
	zend_bool init;

	/*
	* Failed flag
	*/
	zend_bool failed;

	/*
	* Globals Mutex
	*/
	pthreads_lock lock;

	/*
	* Global Strings
	*/
	HashTable strings;

	/*
	* Global/Default Resource Destructor
	*/
	dtor_func_t (default_resource_dtor);

	/*
	* Objects Cache
	*/
	HashTable objects;
}; /* }}} */

extern struct _pthreads_globals pthreads_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads)

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ */
zend_bool pthreads_globals_object_delete(void *address TSRMLS_DC); /* }}} */

/* {{{ */
zend_bool pthreads_globals_object_validate(zend_ulong address TSRMLS_DC); /* }}} */

/* {{{ */
void* pthreads_globals_object_alloc(size_t length TSRMLS_DC); /* }}} */

/* {{{ initialize (true) globals */
zend_bool pthreads_globals_init(TSRMLS_D); /* }}} */

/* {{{ acquire global lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC); /* }}} */

/* {{{ release global lock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC); /* }}} */

/* {{{ copy string to globals */
char *pthreads_global_string(char *strkey, zend_uint keylen, zend_bool lower TSRMLS_DC); /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(TSRMLS_D); /* }}} */

#endif /* HAVE_PTHREADS_GLOBAL_H */
