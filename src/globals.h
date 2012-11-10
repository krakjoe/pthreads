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
* 2. providing a mechanism for limiting the threads a user can create may be useful to server admin in shared hosting environments
* 3. providing peak usage and current realtime statistics can help you to design and execute more efficiently
* 4. these globals are completely safe; they are protected by mutex
* 5. in some SAPI environments it may not make sense to use pthreads.max ini setting, that is for the server administrator to decide
* 6. in some SAPI environments it may make sense to disable importing Threads for security reasons, pthreads.importing should be set to 0
* 7. pthreads.* ini settings are system only for security
*
* TODO
* 1. make errors more meaningful
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
	* Globals Mutex
	*/
	pthreads_lock lock;
	
	/*
	* Peak Number of Objects
	*/
	size_t peak;
	
	/*
	* Maximum number of Objects
	*/
	size_t max;
	
	/*
	* Flag for disable/enable importing
	*/
	zend_bool importing;
	
	/*
	* Objects
	*/
	zend_llist objects;
	
	/*
	* Next Object Identifier
	*/
	ulong nid;
}; /* }}} */

extern struct _pthreads_globals pthreads_globals;

/* {{{ PTHREADS_G */
#define PTHREADS_G(v) pthreads_globals.v
/* }}} */

/* {{{ initialize (true) globals */
void pthreads_globals_init(TSRMLS_D); /* }}} */

/* {{{ acquire global lock */
zend_bool pthreads_globals_lock(zend_bool *locked TSRMLS_DC); /* }}} */

/* {{{ release global lock */
void pthreads_globals_unlock(zend_bool locked TSRMLS_DC); /* }}} */

/* {{{ get current number of accessible objects */
size_t pthreads_globals_count(TSRMLS_D); /* }}} */

/* {{{ push an object into global list */
void pthreads_globals_add(PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ pop an object from global list */
void pthreads_globals_del(PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ get peak number of accessible objects */
long pthreads_globals_peak(TSRMLS_D); /* }}} */

/* {{{ find an object using internal ids */
PTHREAD pthreads_globals_fetch(ulong target TSRMLS_DC); /* }}} */

#endif /* HAVE_PTHREADS_GLOBAL_H */
