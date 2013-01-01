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

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef HAVE_PTHREADS_STATE_H
#	include <src/state.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

/* {{{ stack structure */
typedef struct _pthreads_stack {
	zend_llist objects;
} *pthreads_stack; /* }}} */

/* {{{ address structure */
typedef struct _pthreads_address {
	unsigned char *serial;
	size_t length;
} *pthreads_address; /* }}} */

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
	* Thread Scope
	*/
	int scope;
	
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
	pthreads_lock lock;
	
	/*
	* Thread State
	*/
	pthreads_state state;
	
	/*
	* Thread Sync
	*/
	pthreads_synchro synchro;
	
	/*
	* Method modifiers
	*/
	pthreads_modifiers modifiers;
	
	/*
	* Serial Buffers
	*/
	pthreads_store store;
	
	/*
	* Work List
	*/
	pthreads_stack stack;
	
	/*
	* Thread Address
	*/
	pthreads_address address;

	/**
	* Shared Resources
	**/
	pthreads_resources resources;

	/*
	* Store Hold
	*/
	zend_bool hold;
} *PTHREAD;

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

/* {{{ scope constants */
#define PTHREADS_SCOPE_UNKNOWN 0
#define PTHREADS_SCOPE_THREAD 1
#define PTHREADS_SCOPE_WORKER 2
#define PTHREADS_SCOPE_STACKABLE 4
#define PTHREADS_SCOPE_CONNECTION 8 
/* }}} */

/* {{{ scope macros */
#define PTHREADS_IS_KNOWN_ENTRY(t) (((t)->scope & PTHREADS_SCOPE_UNKNOWN)!=PTHREADS_SCOPE_UNKNOWN)
#define PTHREADS_IS_CONNECTION(t) (((t)->scope & PTHREADS_SCOPE_CONNECTION)==PTHREADS_SCOPE_CONNECTION)
#define PTHREADS_IS_NOT_CONNECTION(t) (((t)->scope & PTHREADS_SCOPE_CONNECTION)!=PTHREADS_SCOPE_CONNECTION)
#define PTHREADS_IS_THREAD(t) (((t)->scope & PTHREADS_SCOPE_THREAD)==PTHREADS_SCOPE_THREAD)
#define PTHREADS_IS_NOT_THREAD(t) (((t)->scope & PTHREADS_SCOPE_THREAD)!=PTHREADS_SCOPE_THREAD)
#define PTHREADS_IS_WORKER(t) (((t)->scope & PTHREADS_SCOPE_WORKER)==PTHREADS_SCOPE_WORKER)
#define PTHREADS_IS_NOT_WORKER(t) (((t)->scope & PTHREADS_SCOPE_WORKER)!=PTHREADS_SCOPE_WORKER)
#define PTHREADS_IS_STACKABLE(t) (((t)->scope & PTHREADS_SCOPE_STACKABLE)==PTHREADS_SCOPE_STACKABLE)
#define PTHREADS_IS_NOT_STACKABLE(t) (((t)->scope & PTHREADS_SCOPE_STACKABLE)!=PTHREADS_SCOPE_STACKABLE)
/* }}} */

/* {{{ pthread_self wrapper */
static inline ulong pthreads_self() {
#ifdef _WIN32
	return (ulong) GetCurrentThreadId();
#else
	return (ulong) pthread_self();
#endif
} /* }}} */

/* {{{ tell if the calling thread created referenced PTHREAD */
#define PTHREADS_IN_CREATOR(t)	(t->cls == tsrm_ls) /* }}} */

/* {{{ tell if the referenced thread is the threading context */
#define PTHREADS_IN_THREAD(t)	(t->tls == tsrm_ls) /* }}} */

#endif /* }}} */ /* HAVE_PTHREADS_THREAD_H */
