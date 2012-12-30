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
	* Global/Default Resource Destructor
	*/
	dtor_func_t (default_resource_dtor);
}; /* }}} */

extern struct _pthreads_globals pthreads_globals;

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ initialize (true) globals */
zend_bool pthreads_globals_init(TSRMLS_D); /* }}} */

/* {{{ acquire global lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC); /* }}} */

/* {{{ release global lock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC); /* }}} */

/* {{{ shutdown global structures */
void pthreads_globals_shutdown(TSRMLS_D); /* }}} */

ZEND_DECLARE_MODULE_GLOBALS(pthreads)

#endif /* HAVE_PTHREADS_GLOBAL_H */
