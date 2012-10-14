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

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STATE_H
#	include <src/state.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

/* {{{ thread structure */
typedef struct _pthread_construct {
	/*
	* Standard Entry
	*/
	zend_object std;

	/*
	* Thread Object
	*/
	pthread_t thread;
	
	/*
	* Thread Identity and LS
	*/
	ulong tid;
	void ***tls;
	
	/*
	* Creator Identity and LS
	*/
	ulong cid;
	void ***cls;
	
	/*
	*  Thread Lock
	*/
	pthread_mutex_t *lock;
	
	/*
	* Thread State
	*/
	pthreads_state state;
	
	/*
	* Method modifiers
	*/
	pthreads_modifiers modifiers;
	
	/*
	* Thread Sync
	*/
	pthread_mutex_t *wait;
	pthread_cond_t	*sync;

	/*
	* Thread Flags
	*/
	zend_bool copy;
	zend_bool synchronized;
	zend_bool worker;
	
	/*
	* Serial Buffer
	*/
	char *serial;
	
	/*
	* Work List
	*/
	zend_llist *stack;
	
	struct _pthread_construct *sig;
} THREAD, *PTHREAD;

/* {{{ comparison function */
static inline int pthreads_equal(PTHREAD first, PTHREAD second) {
	if (first && second) {
		return (first == second);
	}
	return 0;
} /* }}} */

/* {{{ comparison callback for llists */
static inline int pthreads_equal_func(void **first, void **second){
	if (first && second)
		return pthreads_equal((PTHREAD)*first, (PTHREAD)*second);
	return 0;
} /* }}} */

/* {{{ copy an instance to another context */
static inline void pthreads_copy(PTHREAD source, PTHREAD destination){
	if (source && destination) {
		destination->copy = 1;
		//destination->thread = source->thread;
		destination->tid = source->tid;
		destination->tls = source->tls;
		destination->cid = source->cid;
		destination->lock = source->lock;
		destination->state = source->state;
		destination->wait = source->wait;
		destination->sync = source->sync;
		destination->modifiers = source->modifiers;
		destination->synchronized = source->synchronized;
		destination->serial = source->serial;
		destination->stack = source->stack;
	}
} /* }}} */

/* {{{ pthread_self wrapper */
static inline ulong pthreads_self() {
#ifdef _WIN32
	return (ulong) GetCurrentThreadId();
#else
	return (ulong) pthread_self();
#endif
} /* }}} */

/* {{{ tell if the calling thread created referenced PTHREAD */
#define PTHREADS_IS_CREATOR(t)	(t->cid == pthreads_self()) /* }}} */

/* {{{ tell if the referenced thread is the threading context */
#define PTHREADS_IN_THREAD(t)	(t->tls == tsrm_ls) /* }}} */

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

/* {{{ default mutex attributes */
pthread_mutexattr_t		defmutex; /* }}} */

#endif /* }}} */ /* HAVE_PTHREADS_THREAD_H */
