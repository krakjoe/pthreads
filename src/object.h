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
#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H

/*
* @TODO
*	implement verbose debugging mode in threads
*	try and find a way a thread can throw an exception in cid so that ThreadingException can be implemented and not just cause deadlocks
*/

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

/* {{{ object creation and destruction */
zend_object* pthreads_threaded_ctor(zend_class_entry *entry);
zend_object* pthreads_worker_ctor(zend_class_entry *entry);
zend_object* pthreads_thread_ctor(zend_class_entry *entry);
void        pthreads_base_free(zend_object *object);
/* }}} */

/*
* @NOTE
*	pthreads_state_* functions work with pthreads_state directly
*	pthreads_*_state functions work with specific PTHREAD state
*/

/* {{{ state and stack management */
zend_bool pthreads_set_state_ex(PTHREAD thread, int state, long timeout);
zend_bool pthreads_set_state(PTHREAD thread, int state);
zend_bool pthreads_unset_state(PTHREAD thread, int state);
size_t pthreads_stack_pop(PTHREAD thread, PTHREAD work);
size_t pthreads_stack_push(PTHREAD thread, zval *work);
size_t pthreads_stack_next(zval *that);
size_t pthreads_stack_length(PTHREAD thread);
/* }}} */

/* {{{ MISC */
void pthreads_current_thread(zval *return_value);
/* }}} */

/* {{{ start/join */
int pthreads_start(PTHREAD thread);
int pthreads_join(PTHREAD thread);
int pthreads_detach(PTHREAD thread);
/* }}} */

/* {{{ synchronization heplers */
zend_bool pthreads_wait_member(PTHREAD thread, zval *member);
zend_bool pthreads_wait_member_ex(PTHREAD thread, zval *member, ulong timeout); /* }}} */

/* {{{ serialize/unserialize threaded objects */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, size_t *blength, zend_serialize_data *data);
int pthreads_internal_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t blength, zend_unserialize_data *data); /* }}} */

/* {{{ TSRM manipulation */
#define PTHREADS_FETCH_ALL(ls, id, type) ((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])
#define PTHREADS_FETCH_CTX(ls, id, type, element) (((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define PTHREADS_CG(ls, v) PTHREADS_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define PTHREADS_CG_ALL(ls) PTHREADS_FETCH_ALL(ls, compiler_globals_id, zend_compiler_globals*)
#define PTHREADS_EG(ls, v) PTHREADS_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#define PTHREADS_SG(ls, v) PTHREADS_FETCH_CTX(ls, sapi_globals_id, sapi_globals_struct*, v)
#define PTHREADS_PG(ls, v) PTHREADS_FETCH_CTX(ls, core_globals_id, php_core_globals*, v)
#define PTHREADS_EG_ALL(ls) PTHREADS_FETCH_ALL(ls, executor_globals_id, zend_executor_globals*) 
/* }}} */

/* {{{ fetches a PTHREAD from a specific object in the current context */
#define PTHREADS_FETCH_FROM(object) (PTHREAD) (((char*)object) - XtOffsetOf(struct _pthread_construct, std)) /* }}} */

/* {{{ fetches the current PTHREAD from $this */
#define PTHREADS_FETCH (PTHREAD) ((char*) Z_OBJ(EX(This)) - XtOffsetOf(struct _pthread_construct, std)) /* }}} */

/* {{{ handlers included here for access to macros above */
#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <src/handlers.h>
#endif /* }}} */

#endif /* HAVE_PTHREADS_OBJECT_H */
