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

#ifndef HAVE_PTHREADS_SOCKET_H
#	include <src/socket.h>
#endif

typedef union _pthreads_streams_t pthreads_streams_t;

typedef struct _pthreads_ident_t {
	zend_ulong id;
	void*** ls;
} pthreads_ident_t;

/* {{{ */
typedef struct _pthreads_object_t {
	pthread_t thread;
	uint scope;
	zend_ulong options;
	pthreads_monitor_t	*monitor;
	struct {
		pthreads_store_t	*props;
		pthreads_socket_t	*sock;
		pthreads_streams_t	*streams;
	} store;
	pthreads_storage    *user_exception_handler;
	pthreads_stack_t    *stack;
	pthreads_ident_t 	creator;
	pthreads_ident_t	local;
	zend_object         **running;
	zend_object std;
} pthreads_object_t; /* }}} */

/* {{{ */
typedef struct _pthreads_routine_arg_t {
	pthreads_object_t *thread;
	pthreads_monitor_t *ready;
} pthreads_routine_arg_t; /* }}} */

/* {{{ object iterator structure */
typedef struct _pthreads_iterator_t {
    zend_object_iterator zit;
	zval                 object;
    HashPosition         position;
} pthreads_iterator_t; /* }}} */

static inline pthreads_object_t* _pthreads_fetch_object(zend_object *object) {
	return (pthreads_object_t*) ((char*)object - XtOffsetOf(pthreads_object_t, std));
}

/* {{{ fetches a PTHREAD from a specific object in the current context */
#define PTHREADS_FETCH_FROM(object) _pthreads_fetch_object(object) /* }}} */

/* {{{ fetches the current PTHREAD from $this */
#define PTHREADS_FETCH PTHREADS_FETCH_FROM(Z_OBJ(EX(This))) /* }}} */

/* {{{ option constants */
#define PTHREADS_INHERIT_NONE      0x00000000
#define PTHREADS_INHERIT_INI       0x00000001
#define PTHREADS_INHERIT_CONSTANTS 0x00000010
#define PTHREADS_INHERIT_FUNCTIONS 0x00000100
#define PTHREADS_INHERIT_CLASSES   0x00001000
#define PTHREADS_INHERIT_INCLUDES  0x00010000
#define PTHREADS_INHERIT_COMMENTS  0x00100000
#define PTHREADS_INHERIT_ALL       0x00111111
#define PTHREADS_ALLOW_HEADERS	   0x10000000 /* }}} */

/* {{{ scope constants */
#define PTHREADS_SCOPE_UNKNOWN              (0)
#define PTHREADS_SCOPE_THREADED             (1<<1)
#define PTHREADS_SCOPE_THREAD               (1<<2)
#define PTHREADS_SCOPE_WORKER               (1<<3)
#define PTHREADS_SCOPE_SOCKET	            (1<<4)
#define PTHREADS_SCOPE_STREAM               (1<<5)
#define PTHREADS_SCOPE_STREAM_CONTEXT       (1<<6)
#define PTHREADS_SCOPE_STREAM_FILTER	    (1<<7)
#define PTHREADS_SCOPE_STREAM_WRAPPER       (1<<8)
#define PTHREADS_SCOPE_STREAM_BUCKET   	    (1<<9)
#define PTHREADS_SCOPE_STREAM_BRIGADE       (1<<10)
#define PTHREADS_SCOPE_CONNECTION           (1<<11) /* }}} */

/* {{{ scope macros */
#define PTHREADS_IS_KNOWN_ENTRY(t)              ((t)->scope)
#define PTHREADS_IS_CONNECTION(t)               ((t)->scope & PTHREADS_SCOPE_CONNECTION)
#define PTHREADS_IS_NOT_CONNECTION(t)           (!PTHREADS_IS_CONNECTION(t))
#define PTHREADS_IS_SOCKET(t)       	        ((t)->scope & PTHREADS_SCOPE_SOCKET)
#define PTHREADS_IS_NOT_SOCKET(t)   	        (!PTHREADS_IS_SOCKET(t))
#define PTHREADS_IS_THREAD(t)                   ((t)->scope & PTHREADS_SCOPE_THREAD)
#define PTHREADS_IS_NOT_THREAD(t)               (!PTHREADS_IS_THREAD(t))
#define PTHREADS_IS_WORKER(t)                   ((t)->scope & PTHREADS_SCOPE_WORKER)
#define PTHREADS_IS_NOT_WORKER(t)               (!PTHREADS_IS_WORKER(t))
#define PTHREADS_IS_THREADED(t)                 ((t)->scope & PTHREADS_SCOPE_THREADED)
#define PTHREADS_IS_NOT_THREADED(t)             (!PTHREADS_IS_THREADED(t))
#define PTHREADS_IS_STREAM(t)                   ((t)->scope & PTHREADS_SCOPE_STREAM)
#define PTHREADS_IS_NOT_STREAM(t)   	        (!PTHREADS_IS_STREAM(t))
#define PTHREADS_IS_STREAM_CONTEXT(t)           ((t)->scope & PTHREADS_SCOPE_STREAM_CONTEXT)
#define PTHREADS_IS_NOT_STREAM_CONTEXT(t)       (!PTHREADS_IS_STREAM_CONTEXT(t))
#define PTHREADS_IS_STREAM_FILTER(t)       	    ((t)->scope & PTHREADS_SCOPE_STREAM_FILTER)
#define PTHREADS_IS_NOT_STREAM_FILTER(t)   	    (!PTHREADS_IS_STREAM_FILTER(t))
#define PTHREADS_IS_STREAM_WRAPPER(t)           ((t)->scope & PTHREADS_SCOPE_STREAM_WRAPPER)
#define PTHREADS_IS_NOT_STREAM_WRAPPER(t)       (!PTHREADS_IS_STREAM_WRAPPER(t))
#define PTHREADS_IS_STREAM_BUCKET(t)       	    ((t)->scope & PTHREADS_SCOPE_STREAM_BUCKET)
#define PTHREADS_IS_NOT_STREAM_BUCKET(t)   	    (!PTHREADS_IS_STREAM_BUCKET(t))
#define PTHREADS_IS_STREAM_BRIGADE(t)           ((t)->scope & PTHREADS_SCOPE_STREAM_BRIGADE)
#define PTHREADS_IS_NOT_STREAM_BRIGADE(t)       (!PTHREADS_IS_STREAM_BRIGADE(t))

#define PTHREADS_IS_STREAMS(t)       		(PTHREADS_IS_STREAM(t)  \
												|| PTHREADS_IS_STREAM_CONTEXT(t) 		\
												|| PTHREADS_IS_STREAM_FILTER(t) 		\
												|| PTHREADS_IS_STREAM_WRAPPER(t) 		\
												|| PTHREADS_IS_STREAM_BUCKET(t) 		\
												|| PTHREADS_IS_STREAM_BRIGADE(t))
#define PTHREADS_IS_NOT_STREAMS(t)   		(!PTHREADS_IS_STREAMS(t)) /* }}} */

static char *pthreads_get_object_name(pthreads_object_t* object) {
	if(PTHREADS_IS_SOCKET(object))
		return "socket";
	else if(PTHREADS_IS_THREAD(object))
		return "thread";
	else if(PTHREADS_IS_WORKER(object))
		return "worker";
	else if(PTHREADS_IS_THREADED(object))
		return "threaded";
	else if(PTHREADS_IS_STREAM(object))
		return "stream";
	else if(PTHREADS_IS_STREAM_CONTEXT(object))
		return "streamcontext";
	else if(PTHREADS_IS_STREAM_FILTER(object))
		return "streamfilter";
	else if(PTHREADS_IS_STREAM_WRAPPER(object))
		return "streamwrapper";
	else if(PTHREADS_IS_STREAM_BUCKET(object))
		return "streambucket";
	else if(PTHREADS_IS_STREAM_BRIGADE(object))
		return "streambrigade";
}

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

