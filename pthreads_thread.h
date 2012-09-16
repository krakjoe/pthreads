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
#ifndef HAVE_PTHREADS_THREAD_H
#define HAVE_PTHREADS_THREAD_H

/* {{{ thread structure */
typedef struct _pthread_construct {
	/*
	* Standard Entry
	*/
	zend_object std;
	
	/*
	* The Thread
	*/
	pthread_t thread;
	
	/*
	* The Thread Identifier
	*/
	unsigned long tid;
	
	/*
	* The Thread Identifier of Creator
	*/
	unsigned long cid;
	
	/*
	* Thread Safe Local Storage
	*/
	void ***ls;
	
	/*
	* State Management
	*/
	pthread_cond_t	*sync;
	pthread_mutex_t *lock;
	pthread_mutex_t *wait;
	int state;
	
	/*
	* Flags
	*/
	zend_bool self;
	zend_bool synchronized;
	zend_bool import;
	
	/*
	* Serial Buffer
	*/
	char *serial;
	
	/* 
	* Significant Other
	*/
	struct _pthread_construct *sig;
} THREAD, *PTHREAD;

/* {{{ comparison function */
int pthreads_equal(PTHREAD first, PTHREAD second) {
	if (first && second)
		return pthread_equal(first->thread, second->thread);
	return 0;
} /* }}} */

/* {{{ pthread_self wrapper */
ulong pthreads_self() {
#ifdef _WIN32
	return (ulong) GetCurrentThreadId();
#else
	return (ulong) pthread_self();
#endif
} /* }}} */

/* {{{ tell if the current thread created the referenced thread */
#define PTHREADS_IS_CREATOR(t)	(t->cid == pthreads_self()) /* }}} */

/* {{{ tell if the referenced thread is an imported reference */
#define PTHREADS_IS_IMPORT(t) t->import /* }}} */



#endif /* }}} */ /* HAVE_PTHREADS_THREAD_H */
