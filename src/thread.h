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
	HashTable    objects;
	ulong        position;
} *pthreads_stack; /* }}} */

/* {{{ address structure */
typedef struct _pthreads_address {
	unsigned char *serial;
	size_t length;
} *pthreads_address; /* }}} */

/* {{{ error structure */
typedef struct _pthreads_error *pthreads_error;
struct _pthreads_error {
    unsigned char           *file;
    unsigned char           *clazz;
    unsigned char           *method;
    uint                    line;
}; /* }}} */

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
	uint scope;
	
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
	* Store Hold
	*/
	zend_bool hold;
	
	/*
	* Options
	*/
	zend_ulong options;
	
	/*
	* Excluded global vars
	*/
	char *excluded_globals;
	
	int excluded_globals_len;
	
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
	
	/*
	* Threading Error
	*/
    pthreads_error error;

	/**
	* Shared Resources
	**/
	pthreads_resources resources;
	
	/*
	* Object Handle
	*/
	zend_object_handle handle;
} *PTHREAD;

/* {{{ comparison function */
static inline int pthreads_equal(PTHREAD first, PTHREAD second) {
	if (first && second) {
		if ((first == second))
		    return 1;
	}
	return 0;
} /* }}} */

/* {{{ comparison callback for llists */
static inline int pthreads_equal_func(void **first, void **second){
	if (first && second)
		return pthreads_equal((PTHREAD)*first, (PTHREAD)*second);
	return 0;
} /* }}} */

/* {{{ option constants */
#define PTHREADS_INHERIT_NONE      0x00000000
#define PTHREADS_INHERIT_INI       0x00000001
#define PTHREADS_INHERIT_CONSTANTS 0x00000010
#define PTHREADS_INHERIT_FUNCTIONS 0x00000100
#define PTHREADS_INHERIT_CLASSES   0x00001000
#define PTHREADS_INHERIT_INCLUDES  0x00010000
#define PTHREADS_INHERIT_COMMENTS  0x00100000
#define PTHREADS_INHERIT_ALL       0x00111111
#define PTHREADS_ALLOW_GLOBALS     0x01000000
#define PTHREADS_ALLOW_HEADERS	   0x10000000 /* }}} */

/* {{{ scope constants */
#define PTHREADS_SCOPE_UNKNOWN     0
#define PTHREADS_SCOPE_THREADED    1
#define PTHREADS_SCOPE_THREAD      2
#define PTHREADS_SCOPE_WORKER      4
#define PTHREADS_SCOPE_CONNECTION  8
#define PTHREADS_SCOPE_DETACHED    16 /* }}} */

/* {{{ scope macros */
#define PTHREADS_IS_KNOWN_ENTRY(t)      ((t)->scope)
#define PTHREADS_IS_CONNECTION(t)       ((t)->scope & PTHREADS_SCOPE_CONNECTION)
#define PTHREADS_IS_NOT_CONNECTION(t)   (!PTHREADS_IS_CONNECTION(t))
#define PTHREADS_IS_THREAD(t)           ((t)->scope & PTHREADS_SCOPE_THREAD)
#define PTHREADS_IS_NOT_THREAD(t)       (!PTHREADS_IS_THREAD(t))
#define PTHREADS_IS_WORKER(t)           ((t)->scope & PTHREADS_SCOPE_WORKER)
#define PTHREADS_IS_NOT_WORKER(t)       (!PTHREADS_IS_WORKER(t))
#define PTHREADS_IS_THREADED(t)         ((t)->scope & PTHREADS_SCOPE_THREADED)
#define PTHREADS_IS_NOT_THREADED(t)     (!PTHREADS_IS_THREADED(t))
#define PTHREADS_IS_DETACHED(t)         ((t)->scope & PTHREADS_SCOPE_DETACHED)
#define PTHREADS_IS_NOT_DETACHED(t)     (!PTHREADS_IS_DETACHED(t))
/* }}} */

#ifdef HAVE_SIGNAL_H
#ifdef _WIN32
#	define PTHREADS_KILL_SIGNAL			SIGBREAK
#else
#	define PTHREADS_KILL_SIGNAL			SIGUSR1
#endif
#endif

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

#endif /* HAVE_PTHREADS_THREAD_H */
