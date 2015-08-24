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
#ifndef HAVE_PTHREADS_THREAD_H
#define HAVE_PTHREADS_THREAD_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

typedef struct _pthreads_ident_t {
	zend_ulong id;
	void*** ls;
} pthreads_ident_t;

/* {{{ threaded structure */
typedef struct _pthreads_object_t {
	pthread_t thread;
	uint scope;
	zend_ulong options;
	pthreads_monitor_t	*monitor;
	pthreads_store_t	*store;
	pthreads_stack_t    *stack;
	pthreads_ident_t 	creator;
	pthreads_ident_t	local;
	zend_object std;
} pthreads_object_t;

/* {{{ fetches a PTHREAD from a specific object in the current context */
#define PTHREADS_FETCH_FROM(object) ((pthreads_object_t*) (((char*)object) - XtOffsetOf(pthreads_object_t, std))) /* }}} */

/* {{{ fetches the current PTHREAD from $this */
#define PTHREADS_FETCH ((pthreads_object_t*) ((char*) Z_OBJ(EX(This)) - XtOffsetOf(pthreads_object_t, std))) /* }}} */

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
#define PTHREADS_SCOPE_UNKNOWN     (0)
#define PTHREADS_SCOPE_THREADED    (1<<1)
#define PTHREADS_SCOPE_THREAD      (1<<2)
#define PTHREADS_SCOPE_WORKER      (1<<3)
#define PTHREADS_SCOPE_CONNECTION  (1<<4) /* }}} */

/* {{{ scope macros */
#define PTHREADS_IS_KNOWN_ENTRY(t)      ((t)->scope)
#define PTHREADS_IS_CONNECTION(t)       ((t)->scope & PTHREADS_SCOPE_CONNECTION)
#define PTHREADS_IS_NOT_CONNECTION(t)   (!PTHREADS_IS_CONNECTION(t))
#define PTHREADS_IS_THREAD(t)           ((t)->scope & PTHREADS_SCOPE_THREAD)
#define PTHREADS_IS_NOT_THREAD(t)       (!PTHREADS_IS_THREAD(t))
#define PTHREADS_IS_WORKER(t)           ((t)->scope & PTHREADS_SCOPE_WORKER)
#define PTHREADS_IS_NOT_WORKER(t)       (!PTHREADS_IS_WORKER(t))
#define PTHREADS_IS_THREADED(t)         ((t)->scope & PTHREADS_SCOPE_THREADED)
#define PTHREADS_IS_NOT_THREADED(t)     (!PTHREADS_IS_THREADED(t)) /* }}} */

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
#define PTHREADS_IN_CREATOR(t)	((t)->creator.ls == TSRMLS_CACHE) /* }}} */

/* {{{ tell if the referenced thread is the threading context */
#define PTHREADS_IN_THREAD(t)	((t)->local.ls == TSRMLS_CACHE) /* }}} */

#endif /* HAVE_PTHREADS_THREAD_H */
