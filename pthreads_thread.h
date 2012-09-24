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
	void ***pls;
	
	/*
	* State Management
	*/
	pthread_cond_t	*sync;
	pthread_mutex_t *lock;
	pthread_mutex_t *wait;
	int state;
	
	/*
	* Flags, safe but no locking
	*/
	zend_bool self;
	zend_bool synchronized;
	zend_bool import;
	
	/*
	* Requires a thread lock for access
	*/
	zend_bool worker;
	
	/*
	* Serial Buffer
	*/
	char *serial;
	
	/*
	* Work Buffer
	*/
	zend_llist stack;
	
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

/* {{{ comparison callback for llists */
static inline int pthreads_equal_func(void **first, void **second){
	return pthreads_equal((PTHREAD)*first, (PTHREAD)*second);
} /* }}} */

/* {{{ pthread_self wrapper */
ulong pthreads_self() {
#ifdef _WIN32
	return (ulong) GetCurrentThreadId();
#else
	return (ulong) pthread_self();
#endif
} /* }}} */

/* {{{ begin a loop over a list of threads */
#define PTHREADS_LIST_BEGIN_LOOP(l, s) \
	zend_llist_position position;\
	PTHREAD *pointer;\
	if ((pointer = (PTHREAD*) zend_llist_get_first_ex(l, &position))!=NULL) {\
			do {\
				(s) = (*pointer);
/* }}} */

/* {{{ end a loop over a list of threads */
#define PTHREADS_LIST_END_LOOP(l, s) \
	} while((pointer = (PTHREAD*) zend_llist_get_next_ex(l, &position))!=NULL);\
		} else zend_error(E_WARNING, "pthreads has not yet created any threads, nothing to search");
/* }}} */

/* {{{ insert an item into a list of threads */
#define PTHREADS_LIST_INSERT(l, t) zend_llist_add_element(l, &t)
/* }}} */

/* {{{ remove an item from a list of threads */
#define PTHREADS_LIST_REMOVE(l, t) zend_llist_del_element(l, &t, (int (*)(void *, void *)) pthreads_equal_func);
/* }}} */

#endif /* }}} */ /* HAVE_PTHREADS_THREAD_H */
