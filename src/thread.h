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
#define PTHREADS_INHERIT_NONE 0
#define PTHREADS_INHERIT_INI       0x00000001
#define PTHREADS_INHERIT_CONSTANTS 0x00000010
#define PTHREADS_INHERIT_FUNCTIONS 0x00000100
#define PTHREADS_INHERIT_CLASSES   0x00001000
#define PTHREADS_INHERIT_INCLUDES  0x00010000
#define PTHREADS_INHERIT_COMMENTS  0x00100000
#define PTHREADS_INHERIT_ALL (PTHREADS_INHERIT_INI|PTHREADS_INHERIT_CONSTANTS|PTHREADS_INHERIT_FUNCTIONS|PTHREADS_INHERIT_CLASSES|PTHREADS_INHERIT_INCLUDES|PTHREADS_INHERIT_COMMENTS) /* }}} */

/* {{{ scope constants */
#define PTHREADS_SCOPE_UNKNOWN 0
#define PTHREADS_SCOPE_THREAD 1
#define PTHREADS_SCOPE_WORKER 2
#define PTHREADS_SCOPE_STACKABLE 4
#define PTHREADS_SCOPE_CONNECTION 8
#define PTHREADS_SCOPE_DETACHED 16
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
#define PTHREADS_IS_DETACHED(t) (((t)->scope & PTHREADS_SCOPE_DETACHED)==PTHREADS_SCOPE_DETACHED)
#define PTHREADS_IS_NOT_DETACHED(t)  (((t)->scope & PTHREADS_SCOPE_DETACHED)!=PTHREADS_SCOPE_DETACHED)
/* }}} */

/* {{{ pthread_self wrapper */
static inline ulong pthreads_self() {
#ifdef _WIN32
	return (ulong) GetCurrentThreadId();
#else
	return (ulong) pthread_self();
#endif
} /* }}} */

/* {{{ mode enumeration for string length function */
typedef enum {
    PTHREADS_STRLEN_T,
    PTHREADS_STRLEN_NT
} pthreads_strlen_mode_t; /* }}} */

/* {{{ calculate/ascertain or request length of null terminated or not null terminated string */
static inline size_t pthreads_strlen(const char* str, size_t length, pthreads_strlen_mode_t mode) {
    switch (mode) {
        case PTHREADS_STRLEN_T: switch (str[length+1]) {
            case '\0':
                php_printf("(2)found null at %d, return %d\n", length, length+1);
                return length+1;
                
            default: switch (str[length]) {
                case '\0': 
                    php_printf("(3)found null at %d+1, return %d\n", length, length+1);
                    return length;
                    
                default: return strlen(str) + 1;
            }
        } break;
        
        case PTHREADS_STRLEN_NT: {
            
        } break;
    }
} /* }}} */

/* {{{ get null terminated string length */
#define PTHREADS_STRLEN(s, l) \
    (((s)[(l)] == '\0') ? (l) : \
        ((s)[(l)-1] == '\0') ? ((l)+1) : \
            (strlen((s)))) /* }}} */

/* {{{ tell if the calling thread created referenced PTHREAD */
#define PTHREADS_IN_CREATOR(t)	(t->cls == tsrm_ls) /* }}} */

/* {{{ tell if the referenced thread is the threading context */
#define PTHREADS_IN_THREAD(t)	(t->tls == tsrm_ls) /* }}} */

#endif /* }}} */ /* HAVE_PTHREADS_THREAD_H */
