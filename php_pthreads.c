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
#ifndef HAVE_PHP_THREADS
#define HAVE_PHP_THREADS
#include "php_pthreads.h"

#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if COMPILE_DL_PTHREADS
ZEND_GET_MODULE(pthreads)
#endif

zend_class_entry *pthreads_class_entry = NULL;
zend_class_entry *pthreads_import_entry = NULL;
zend_class_entry *pthreads_mutex_class_entry = NULL;
zend_class_entry *pthreads_condition_class_entry = NULL;

/* {{{ defmutex setup 
		Choose the NP type if it exists as it targets the current system */
pthread_mutexattr_t		defmutex;
#ifndef _WIN32
#	ifdef PTHREAD_MUTEX_ERRORCHECK_NP
#		define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK_NP
#	elifdef PTHREAD_MUTEX_ERRORCHECK
#		define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK
#	endif
#endif
/* }}} */

#ifndef HAVE_PTHREADS_COMPAT_H
#include "pthreads_compat.h"
#endif

#ifndef HAVE_PTHREADS_SERIAL_H
#include "pthreads_serial.h"
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#include "pthreads_globals.h"
#endif

/* {{{ arginfo */

/* {{{ Thread arginfo */

/* {{{ basic */
ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
	ZEND_ARG_INFO(0, synchronized)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_run, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ synchronization */
ZEND_BEGIN_ARG_INFO_EX(Thread_wait, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_join, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ importing */
ZEND_BEGIN_ARG_INFO_EX(Thread_getThread, 0, 0, 1)
	ZEND_ARG_INFO(0, tid)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ identification */
ZEND_BEGIN_ARG_INFO_EX(Thread_getThreadId, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ state detection */
ZEND_BEGIN_ARG_INFO_EX(Thread_isStarted, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isJoined, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isWaiting, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isBusy, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ stacking */
ZEND_BEGIN_ARG_INFO_EX(Thread_stack, 0, 0, 1)
	ZEND_ARG_INFO(0, work)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Thread_unstack, 0, 0, 0)
	ZEND_ARG_INFO(0, work)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Thread_getStacked, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ statics/globals */
ZEND_BEGIN_ARG_INFO_EX(Thread_getCount, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getMax, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getPeak, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* }}} */

/* {{{ Mutex arginfo */
ZEND_BEGIN_ARG_INFO_EX(Mutex_create, 0, 0, 0)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_lock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_trylock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_unlock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
	ZEND_ARG_INFO(0, destroy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ Cond arginfo */
ZEND_BEGIN_ARG_INFO_EX(Cond_create, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_signal, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_wait, 0, 0, 2)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, mutex)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_broadcast, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()
/* }}} */

/* }}} */

/* {{{ method entries */

/* {{{ Thread method entries */
zend_function_entry pthreads_methods[] = {
	PHP_ME(Thread, start, Thread_start, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(Thread, run, Thread_run)
	PHP_ME(Thread, wait, Thread_wait, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, notify, Thread_notify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isRunning, Thread_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isWaiting, Thread_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isBusy, Thread_isBusy, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, stack, Thread_stack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, unstack, Thread_unstack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getStacked, Thread_getStacked, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getThread, Thread_getThread, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_STATIC)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getCount, Thread_getCount, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getMax, Thread_getMax, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getPeak, Thread_getPeak, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ Import method entries */
zend_function_entry	pthreads_import_methods[] = {
	PHP_ME(Thread, wait, Thread_wait, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, notify, Thread_notify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isRunning, Thread_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isWaiting, Thread_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isBusy, Thread_isBusy, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getStacked, Thread_getStacked, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	/* }}} */
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ Mutex method entries */
zend_function_entry pthreads_mutex_methods[] = {
	PHP_ME(Mutex, create, Mutex_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, lock, Mutex_lock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, trylock, Mutex_trylock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, unlock, Mutex_unlock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, destroy, Mutex_destroy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ Cond method entries */
zend_function_entry pthreads_condition_methods[] = {
	PHP_ME(Cond, create, Cond_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, signal, Cond_signal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, wait, Cond_wait, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, broadcast, Cond_broadcast, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, destroy, Cond_destroy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
}; /* }}} */

/* }}} */

/* {{{ pthreads module entry */
zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_PTHREADS_EXTNAME,
  NULL,
  PHP_MINIT(pthreads),
  PHP_MSHUTDOWN(pthreads),
  NULL,
  NULL,
  PHP_MINFO(pthreads),
  PHP_PTHREADS_VERSION,
  STANDARD_MODULE_PROPERTIES
}; /* }}} */

/* {{{ INI Entries */
PHP_INI_BEGIN()
	/* {{{ pthreads.max allows admins some control over how many threads a user can create */
	PHP_INI_ENTRY("pthreads.max", "0", PHP_INI_SYSTEM, NULL) /* }}} */
	/* {{{ pthreads.importing allows importing threads to be disabled where it poses a security risk */
	PHP_INI_ENTRY("pthreads.importing", "1", PHP_INI_SYSTEM, NULL) /* }}} */
PHP_INI_END()
/* }}} */

#ifndef HAVE_PTHREADS_OBJECT_H
#	include "pthreads_object.h"
#endif

/* {{{ module initialization */
PHP_MINIT_FUNCTION(pthreads)
{
	zend_class_entry te;
	zend_class_entry ie;
	zend_class_entry me;
	zend_class_entry ce;

	REGISTER_INI_ENTRIES();

	INIT_CLASS_ENTRY(te, "Thread", pthreads_methods);
	te.create_object = pthreads_attach_to_instance;
	te.serialize = zend_class_serialize_deny;
	te.unserialize = zend_class_unserialize_deny;
	pthreads_class_entry=zend_register_internal_class(&te TSRMLS_CC);

	if (INI_INT("pthreads.importing")) {
		INIT_CLASS_ENTRY(ie, "ImportedThread", pthreads_import_methods);
		ie.create_object = pthreads_attach_to_import;
		ie.serialize = zend_class_serialize_deny;
		ie.unserialize = zend_class_unserialize_deny;
		pthreads_import_entry=zend_register_internal_class(&ie TSRMLS_CC);
	}
	
	INIT_CLASS_ENTRY(me, "Mutex", pthreads_mutex_methods);
	me.serialize = zend_class_serialize_deny;
	me.unserialize = zend_class_unserialize_deny;
	pthreads_mutex_class_entry=zend_register_internal_class(&me TSRMLS_CC);
	
	INIT_CLASS_ENTRY(ce, "Cond", pthreads_condition_methods);
	ce.serialize = zend_class_serialize_deny;
	ce.unserialize = zend_class_unserialize_deny;
	pthreads_condition_class_entry=zend_register_internal_class(&ce TSRMLS_CC);
	
	if ( pthread_mutexattr_init(&defmutex)==SUCCESS ) {
#ifdef DEFAULT_MUTEX_TYPE
		pthread_mutexattr_settype(
			&defmutex, 
			DEFAULT_MUTEX_TYPE
		);
#endif
	}
	
	if (!PTHREADS_G(init)) 
		PTHREADS_G_INIT();
	
	return SUCCESS;
} /* }}} */

/* {{{ module shutdown */
PHP_MSHUTDOWN_FUNCTION(pthreads)
{
	UNREGISTER_INI_ENTRIES();
	
	pthread_mutexattr_destroy(
		&defmutex
	);
	
	return SUCCESS;
} /* }}} */

/* {{{ minfo */
PHP_MINFO_FUNCTION(pthreads)
{
	char numbers[10];
	
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_PTHREADS_VERSION);
	if (INI_INT("pthreads.max")) {
		php_info_print_table_row(2, "Maximum Threads", INI_STR("pthreads.max"));
	} else php_info_print_table_row(2, "Maximum Threads", "No Limits");
	
	snprintf(numbers, sizeof(numbers), "%d", PTHREADS_G_COUNT());
	php_info_print_table_row(2, "Current Threads", numbers);
	
	snprintf(numbers, sizeof(numbers), "%d", PTHREADS_G_PEAK());
	php_info_print_table_row(2, "Peak Threads", numbers);

	php_info_print_table_end();
} /* }}} */

/* {{{ Thread methods */

/* {{{ proto boolean Thread::start([boolean sync])
		Starts executing your implemented run method in a thread, will return a boolean indication of success
		If sync is true, your new thread runs synchronized with the current thread ( causing the current thread to block ) until you call notify from within the new thread or the thread ends
		If sync is false or void, the new thread will run asynchronously to the current thread and this call will return as quickly as possible */
PHP_METHOD(Thread, start)
{
	PTHREAD thread = PTHREADS_FETCH;
	int result = -1;
	zval *props;
	
	/*
	* Ensure we are in the correct context
	*/
	if (!PTHREADS_IS_SELF(thread)) {
		/*
		* See if there are any limits in this environment
		*/
		if (INI_INT("pthreads.max") && !(PTHREADS_G_COUNT()<INI_INT("pthreads.max"))) {
			zend_error(E_WARNING, "pthreads has reached the maximum numbers of threads allowed (%lu) by your server administrator", INI_INT("pthreads.max"));
			RETURN_FALSE;
		}
		
		/*
		* Find out if the calling thread will wait for notification before continuing
		*/
		if (ZEND_NUM_ARGS()){
			zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &thread->synchronized);
		} else thread->synchronized = 0;
		
		/*
		* Stop multiple starts from occuring
		*/
		if (!PTHREADS_IS_STARTED(thread)) {
			PTHREADS_SET_STARTED(thread);
			
			/*
			* Serializing here keeps the heap happy, so here is where we'll do it ...
			* @NOTE Verification
			*	I still feel that some kind of verification is needed before we attempt to run the thread as it will help those new to threading
			*/
			ALLOC_INIT_ZVAL(props);
			Z_TYPE_P(props)=IS_ARRAY;
			Z_ARRVAL_P(props)=thread->std.properties;
			thread->serial = pthreads_serialize(props TSRMLS_CC);
			FREE_ZVAL(props);
			
			/* even null serializes, so if we get a NULL pointer then the serialzation failed and we should not continue */
			if (thread->serial == NULL) {
				zend_error(E_ERROR, "pthreads detected unsupported symbols in your thread, please amend your parameters");
				RETURN_FALSE;
			}
			
			/*
			* Set appropriate flags
			*/
			PTHREADS_SET_RUNNING(thread);
			
			/*
			* Acquire Thread Lock
			*/
			PTHREADS_LOCK(thread);
			
			/*
			* Create the thread
			*/
			if ((result = pthread_create(&thread->thread, NULL, PHP_PTHREAD_ROUTINE, (void*)thread)) == SUCCESS) {
			
				/*
				* Release Thread Lock
				*/
				PTHREADS_UNLOCK(thread);
				
				/*
				* Wait for appropriate event
				*/
				PTHREADS_WAIT(thread);
			} else {
			
				/*
				* Do not attempt to join failed threads at dtor time
				*/
				PTHREADS_UNSET_RUNNING(thread);
				zend_error(E_ERROR, "pthreads detected an internal error while creating thread (%d)", result);
			}
		} else zend_error(E_WARNING, "pthreads detected that this thread has already been started and it cannot reuse the Thread");
	} else zend_error(E_WARNING, "pthreads detected an attempt to call an external method, do not attempt to start in this context");
	
	if (result==SUCCESS) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
} /* }}} */

/* {{{ proto int Thread::stack(Thread $work)
	Pushes an item onto the Thread Stack, returns the size of stack */
PHP_METHOD(Thread, stack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval 	*work;
	int		size;
	
	if (thread) {
		if (!PTHREADS_IS_SELF(thread)) {
			if (!PTHREADS_IS_JOINED(thread)) {
				if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, thread->std.ce)==SUCCESS) {
					size = PTHREADS_PUSH(thread, PTHREADS_FETCH_FROM(work));
					if (size) {
						RETURN_LONG(size);
					}
				}
			} else zend_error(E_ERROR, "pthreads has detected an attempt to stack onto a joined Thread (%lu)", thread->tid);
		} else zend_error(E_ERROR, "pthreads has detected an attempt to call and external method on %lu, you cannot stack in this context", thread->tid);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	RETURN_FALSE;
} /* }}} */

/* {{{ proto int Thread::unstack([Thread $work])
	Removes an item from the Thread stack, if no item is specified the stack is cleared, returns the size of stack */
PHP_METHOD(Thread, unstack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval * work;
	
	if (thread) {
		if (!PTHREADS_IS_SELF(thread)) {
			if (ZEND_NUM_ARGS() > 0) {
				if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, thread->std.ce)==SUCCESS) {
					RETURN_LONG(PTHREADS_POP_EX(thread, PTHREADS_FETCH_FROM(work) TSRMLS_CC));
				}
			} else RETURN_LONG(PTHREADS_POP_EX(thread, NULL TSRMLS_CC));
		} else zend_error(E_WARNING, "pthreads has detected an attempt to call an external method, you cannot unstack in this context");
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	RETURN_FALSE;
}

/* {{{ proto int Thread::getStacked()
	Returns the current size of the stack */
PHP_METHOD(Thread, getStacked)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_SELF(thread) || PTHREADS_IS_IMPORT(thread)) {
			RETURN_LONG(PTHREADS_GET_STACKED(thread->sig));
		} else RETURN_LONG(PTHREADS_GET_STACKED(thread));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	RETURN_FALSE;
}

/* {{{ proto Thread::isStarted() 
	Will return true if a Thread has been started */
PHP_METHOD(Thread, isStarted)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_IMPORT(thread)){
			RETURN_BOOL(PTHREADS_IS_STARTED(thread->sig));
		} else if (!PTHREADS_IS_SELF(thread)) {
			RETURN_BOOL(PTHREADS_IS_STARTED(thread));
		} else zend_error(E_WARNING, "pthreads has detected an attempt to use Thread::isStarted from the a Thread");
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
} /* }}} */

/* {{{ proto Thread::isRunning() 
	Will return true while a thread is executing */
PHP_METHOD(Thread, isRunning)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_IMPORT(thread)) {
			RETURN_BOOL(PTHREADS_IS_RUNNING(thread->sig));
		} else if (!PTHREADS_IS_SELF(thread)) {
			RETURN_BOOL(PTHREADS_IS_RUNNING(thread));
		} else zend_error(E_WARNING, "pthreads has detected an attempt to use Thread::isRunning from a Thread");
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
PHP_METHOD(Thread, isJoined)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_IMPORT(thread)) {
			RETURN_BOOL(PTHREADS_IS_JOINED(thread->sig));
		} else if (!PTHREADS_IS_SELF(thread)) {
			RETURN_BOOL(PTHREADS_IS_JOINED(thread));
		} else zend_error(E_WARNING, "pthreads has detected an attempt to use Thread::isJoined from a Thread");
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
} /* }}} */

/* {{{ proto Thread::isBusy()
	Will return true if a Thread has been started and is running but hasn't yet been joined */
PHP_METHOD(Thread, isBusy)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_IMPORT(thread)) {
			PTHREADS_LOCK(thread->sig);
			if (PTHREADS_IN_STATE(thread->sig, PTHREADS_ST_STARTED)) {
				if (!PTHREADS_IN_STATE(thread->sig, PTHREADS_ST_JOINED)) {
					ZVAL_BOOL(return_value, PTHREADS_IN_STATE(thread->sig, PTHREADS_ST_RUNNING));
				} else ZVAL_BOOL(return_value, 0);
			} else ZVAL_BOOL(return_value, 0);
			PTHREADS_UNLOCK(thread->sig);
		} else if (!PTHREADS_IS_SELF(thread)) {
			PTHREADS_LOCK(thread);
			if (PTHREADS_IN_STATE(thread, PTHREADS_ST_STARTED)) {
				if (!PTHREADS_IN_STATE(thread, PTHREADS_ST_JOINED)) {
					ZVAL_BOOL(return_value, PTHREADS_IN_STATE(thread, PTHREADS_ST_RUNNING));
				} else ZVAL_BOOL(return_value, 0);
			} else ZVAL_BOOL(return_value, 0);
			PTHREADS_UNLOCK(thread);
		} else zend_error(E_WARNING, "pthreads has detected an attempt to use Thread::isBusy from a Thread");
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	RETURN_NULL();
} /* }}} */

/* {{{ proto boolean Thread::isWaiting() 
	Will return true if the referenced thread is waiting for notification */
PHP_METHOD(Thread, isWaiting)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (PTHREADS_IS_IMPORT(thread)){
			PTHREADS_LOCK(thread->sig);
			ZVAL_BOOL(return_value, PTHREADS_IN_STATE(thread->sig, PTHREADS_ST_WAITING));
			PTHREADS_UNLOCK(thread->sig);
		} else if (!PTHREADS_IS_SELF(thread)){
			PTHREADS_LOCK(thread);
			ZVAL_BOOL(return_value, PTHREADS_IN_STATE(thread, PTHREADS_ST_WAITING));
			PTHREADS_UNLOCK(thread);
		} else zend_error(E_WARNING, "pthreads has detected an attempt to use Thread::isWaiting from a Thread");
	} else zend_error(E_ERROR, "pthreads has expereinced an internal error and cannot continue");
	RETURN_NULL();
}

/* {{{ proto boolean Thread::wait([long timeout]) 
		Will cause the calling process or thread to wait for the referenced thread to notify
		When a timeout is used an reached boolean false will return
		When no timeout is used a boolean indication of success will return */
PHP_METHOD(Thread, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	long timeout = 0L;
	
	if(ZEND_NUM_ARGS()){
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout)!=SUCCESS)
			RETURN_FALSE;
	}
	
	if (thread) {
		if (!PTHREADS_IS_SELF(thread) && !PTHREADS_IS_IMPORT(thread)) {
			RETURN_BOOL(PTHREADS_WAIT_EX(thread, timeout)>0);
		} else RETURN_BOOL(PTHREADS_WAIT_EX(thread->sig, timeout)>0);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
} /* }}} */

/* {{{ proto boolean Thread::notify()
		Notify anyone waiting that they can continue
		A call to Thread::notify when no one is waiting will cause the thread or process that called it to block until someone waits
		Will return a boolean indication of success
		@TODO make this handle imported threads */
PHP_METHOD(Thread, notify)
{
	PTHREAD thread = PTHREADS_FETCH;
	if (thread) {
		if (PTHREADS_IS_SELF(thread)||PTHREADS_IS_IMPORT(thread)) {
			RETURN_BOOL(PTHREADS_NOTIFY(thread->sig)>0);
		} else RETURN_BOOL(PTHREADS_NOTIFY(thread)>0);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	RETURN_FALSE;
} /* }}} */

/* {{{ proto mixed Thread::join() 
		Will cause the calling thread to block and wait for the output of the referenced thread */
PHP_METHOD(Thread, join) 
{ 
	PTHREAD thread = PTHREADS_FETCH;
	PTHREAD selected = NULL;
	char *result = NULL;
	
	if (thread) {
	
		/*
		* Select the appropriate way to join the thread with the calling context
		*/
		if ((selected=(!PTHREADS_IS_SELF(thread) && !PTHREADS_IS_IMPORT(thread)) ? thread : (PTHREADS_IS_IMPORT(thread) && !PTHREADS_IS_SELF(thread)) ? thread->sig : NULL) != NULL) {
			
			/*
			* Ensure this thread was started
			*/
			if (PTHREADS_IS_STARTED(selected)) {
				
				/*
				* Ensure this thread wasn't already joined
				*/
				if (!PTHREADS_IS_JOINED(selected)) {
					
					/*
					* We must force threads to wake at this point
					*/
					if (PTHREADS_IS_BLOCKING(selected)) {
						if (!PTHREADS_IS_WORKER(selected)) {
							zend_error(E_WARNING, "pthreads has avoided a deadlock, the referenced thread was waiting for notification");
						}
						do {
							PTHREADS_NOTIFY(selected);
						} while(PTHREADS_IS_BLOCKING(selected));
					}
					
					/*
					* Ensure nothing attempts to join this thread again
					*/
					PTHREADS_SET_JOINED(selected);
					
					/*
					* Carry out joining and deserialize result
					*/
					if (pthread_join(selected->thread, (void**)&result)==SUCCESS) {
						if (result) {
							pthreads_unserialize_into(result, return_value TSRMLS_CC);
							free(result);
						}
					} else {
						zend_error(E_WARNING, "pthreads detected failure while joining with the referenced Thread (%lu)", thread->tid);
						RETURN_FALSE;
					}
				} else {
					zend_error(E_WARNING, "pthreads has detected that the referenced Thread (%lu) has already been joined", thread->tid);
					RETURN_TRUE; 
				}
			} else {
				zend_error(E_WARNING, "pthreads has detected an attempt to join a thread that is not yet started");
			}
		} else {
			zend_error(E_ERROR, "pthreads detected a call to an external method, do not attempt to join in the this context");
		}
	} else {
		zend_error(E_ERROR, "pthreads has expereinced an internal error and cannot continue");
		RETURN_FALSE;
	}
} /* }}} */

/* {{{ proto boolean Thread::getThread([long $tid])
		Thread::find shall attempt to import the Thread identified by tid into the current context, for the purposes of synchronization */
PHP_METHOD(Thread, getThread)
{
	PTHREAD found = NULL;
	unsigned long tid = 0L;
	
	if (pthreads_import_entry != NULL) {
		if (EG(called_scope) == pthreads_class_entry) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &tid)==SUCCESS) {
				if (tid && tid >0L) {
					if ((found=PTHREADS_FIND((unsigned long)tid))!=NULL) {
						if (!PTHREADS_IS_CREATOR(found)) {
							if (PTHREADS_IMPORT(found, &return_value TSRMLS_CC)) {
								return;
							} else zend_error(E_WARNING, "pthreads has detected that Thread::getThread(%lu) has failed", tid);
						} else zend_error(E_WARNING, "pthreads has detected that Thread::getThread(%lu) has nothing to import as the requested thread was created in the current", tid);
						RETURN_FALSE;
					} else zend_error(E_WARNING, "pthreads has detected that Thread::getThread(%lu) has nothing to import as the requested thread is no longer running", tid);
				} else zend_error(E_WARNING, "pthreads has detected incorrect use of Thread::getThread, no valid Thread identifier was specified");
			}
		} else zend_error(E_WARNING, "pthreads has detected incorrect use of Thread::getThread, this method may only be called statically as Thread::getThread");
	} else zend_error(E_WARNING, "pthreads has detected an attempt to use a prohibited method (Thread::getThread), importing Threads is disabled in this environment");
	RETURN_NULL();
}
/* }}} */

/* {{{ proto long Thread::getThreadId() 
	Will return the identifier of the referenced Thread */
PHP_METHOD(Thread, getThreadId)
{
	if (getThis()) {
		PTHREAD thread = PTHREADS_FETCH;
		if (thread) {
			ZVAL_LONG(return_value, thread->tid);
		} else ZVAL_LONG(return_value, 0);
	} else ZVAL_LONG(return_value, 0);
} /* }}} */

/* {{{ proto long Thread::getCount()
		Will return the number of threads currently running */
PHP_METHOD(Thread, getCount)
{
	RETURN_LONG(PTHREADS_G_COUNT());
} /* }}} */

/* {{{ proto long Thread::getMax()
		Will return the maximum number of threads you may create in this environment
		@NOTE this setting very much depends on the kind of environment you're executing php in */
PHP_METHOD(Thread, getMax)
{
	RETURN_LONG(INI_INT("pthreads.max"));
} /* }}} */

/* {{{ proto long Thread::getPeak() 
	Will return the peak number of threads executing */
PHP_METHOD(Thread, getPeak)
{
	RETURN_LONG(PTHREADS_G_PEAK());
} /* }}} */

/* }}} */

/* {{{ Mutex methods */

/* {{{ proto long Mutex::create([boolean lock]) 
		Will create a new mutex and return it's handle. If lock is true it will lock the mutex before returning the handle to the calling thread.
		Because of their nature you need to destroy these explicitly with Mutex::destroy */
PHP_METHOD(Mutex, create)
{
	zend_bool lock;
	pthread_mutex_t *mutex;
	
	if ((mutex=(pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t)))!=NULL) {
		switch(pthread_mutex_init(mutex, &defmutex)){
			case SUCCESS: 
				if (ZEND_NUM_ARGS()) {				
					if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &lock)==SUCCESS) {
						if (lock) {
							switch(pthread_mutex_lock(mutex)){
								case SUCCESS: RETURN_LONG((ulong)mutex); break;
								case EDEADLK: RETURN_LONG((ulong)mutex); break;
								
								default: 
									zend_error(E_ERROR, "pthreads detected an unspecified error while attempting to lock new mutex");
									pthread_mutex_destroy(mutex);
									free(mutex);
							}
						} else { RETURN_LONG((ulong)mutex); }
					}
				} else { RETURN_LONG((ulong)mutex); }
			break;
		
			case EAGAIN:
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary resources (other than memory) to initialise another mutex"); 
				free(mutex);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary memory to initialise another mutex"); 
				free(mutex);
			break;
			
			case EPERM:
				zend_error(E_ERROR, "pthreads detected that the caller does not have permission to initialize mutex"); 
				free(mutex);
			break;
			
			default: 
				zend_error(E_ERROR, "pthreads detected an internal error while attempting to initialize mutex");
				free(mutex);
		}
	} else zend_error(E_ERROR, "pthreads has failed to allocate memory for mutex and cannot continue");
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Mutex::lock(long mutex) 
		Locks the mutex referenced so that the calling thread owns the mutex */
PHP_METHOD(Mutex, lock)
{
	pthread_mutex_t *mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mutex)==SUCCESS && mutex) {
		switch (pthread_mutex_lock(mutex)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EDEADLK:
				zend_error(E_WARNING, "pthreads has detected that the calling thread already owns the mutex");
				RETURN_TRUE;
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected that the variable passed is not a valid mutex");  			
			break;
			
			/* 	the default type of mutex prohibits this error from being raised 
				there may be more control over mutex types in the future so left for completeness */
			case E_ERROR: 
				zend_error(E_WARNING, "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded");
			break;
			
			default: 
				zend_error(E_ERROR, "pthreads detected an internal error while locking mutex and cannot continue");
		}
	}
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Mutex::trylock(long mutex) 
		Will attempt to lock the mutex without causing the calling thread to block */
PHP_METHOD(Mutex, trylock)
{
	pthread_mutex_t *mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mutex)==SUCCESS && mutex) {
		switch (pthread_mutex_trylock(mutex)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EBUSY: RETURN_FALSE; break;
			
			case EDEADLK:
				zend_error(E_WARNING, "pthreads has detected that the calling thread already owns the mutex");
				RETURN_TRUE;
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected that the variable passed is not a valid mutex");  
			break;
			
			/* again the defmutex setup prohibits this from occuring, present for completeness */
			case EAGAIN: 
				zend_error(E_WARNING, "pthreads detected that the mutex could not be acquired because the maximum number of recursive locks has been exceeded");
			break;
			
			default: 
				zend_error(E_ERROR, "pthreads detected an internal error while trying to lock mutex and cannot continue");
		}
	}
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Mutex::unlock(long mutex, [bool destroy]) 
		Will unlock the mutex referenced, and optionally destroy it */
PHP_METHOD(Mutex, unlock)
{
	pthread_mutex_t *mutex;
	zend_bool destroy = 0;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &mutex, &destroy)==SUCCESS && mutex) {
		switch (pthread_mutex_unlock(mutex)) {
			case SUCCESS: 
				if (destroy > 0) {
					pthread_mutex_destroy(mutex);
					free(mutex);
				}
				
				RETURN_TRUE; 
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected that the variable passed is not a valid mutex"); 
			break;
			
			case EPERM:
				zend_error(E_WARNING, "pthreads has detected that the calling thread does not own the mutex");
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while unlocking mutex and cannot continue");
		}
	}
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Mutex::destroy(long mutex)
		Will destroy the mutex referenced and free memory associated */
PHP_METHOD(Mutex, destroy)
{
	pthread_mutex_t *mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mutex)==SUCCESS && mutex) {
		switch (pthread_mutex_destroy(mutex)) {
			case SUCCESS: 
				free(mutex);
				RETURN_TRUE;
			break;

			case EBUSY:
				zend_error(E_WARNING, "pthreads has detected an attempt to destroy mutex while it is locked or referenced"); 
			break;
			
			case EINVAL:
				zend_error(E_WARNING, "pthreads has detected that the variable passed is not a valid mutex"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while attempting to destroy mutex and cannot continue");
		}
	}
	RETURN_FALSE;
} /* }}} */

/* }}} */

/* {{{ Cond Methods */

/* {{{ proto long Cond::create() 
		This will create a condition, because of their nature you need to destroy these explicitly with Cond::destroy */
PHP_METHOD(Cond, create)
{
	pthread_cond_t *condition;
	
	if ((condition=(pthread_cond_t*) calloc(1, sizeof(pthread_cond_t)))!=NULL) {
		switch (pthread_cond_init(condition, NULL)) {
			case SUCCESS: 
				RETURN_LONG((ulong)condition); 
			break;
			
			case EAGAIN:
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary resources (other than memory) to initialise another condition"); 
				free(condition);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary memory to initialise another condition"); 
				free(condition);
			break;
			
			default: 
				zend_error(E_ERROR, "pthreads detected an internal error while initializing new condition and cannot conitnue");
				free(condition);
		}
	} else zend_error(E_ERROR, "pthreads failed to allocate memory for new condition and cannot continue");
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::signal(long condition) 
		This will signal a condition */
PHP_METHOD(Cond, signal)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition) == SUCCESS && condition) {
		switch (pthread_cond_signal(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while signaling condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::broadcast(long condition) 
		This will broadcast a condition */
PHP_METHOD(Cond, broadcast)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition)==SUCCESS && condition) {
		switch (pthread_cond_broadcast(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL:
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while broadcasting condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::wait(long condition, long mutex, [long timeout]) 
		This will wait for a signal or broadcast on condition, you must have mutex locked by the calling thread
		Timeout should be expressed in microseconds ( millionths ) 
		@NOTE timeouts dodgy in windows because of a lack of proper gettimeofday implementation, dispite it being defined, somewhere ... */
PHP_METHOD(Cond, wait)
{
	pthread_cond_t *condition;
	pthread_mutex_t *mutex;
	long timeout = 0L;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &condition, &mutex, &timeout)==SUCCESS && condition && mutex) {
		if (timeout > 0L) {
			struct timeval now;
			if (pthreads_gettimeofday(&now, NULL)==SUCCESS) {
				struct timespec until;
				long	nsec = timeout * 1000;
				if (nsec > 1000000000L) {
					until.tv_sec = now.tv_sec + (nsec / 1000000000L);
					until.tv_nsec = (now.tv_usec * 1000) + (nsec % 1000000000L);
				} else {
					until.tv_sec = now.tv_sec;
					until.tv_nsec = (now.tv_usec * 1000) + timeout;	
				}
				
				switch(pthread_cond_timedwait(condition, mutex, &until)){
					case SUCCESS: RETURN_TRUE; break;
					case EINVAL: 
						zend_error(E_WARNING, "pthreads has detected that the condition you are attempting to wait on does not refer to a valid condition variable");
						RETURN_FALSE;
					break;
					
					case ETIMEDOUT: 
						zend_error(E_WARNING, "pthreads detected a timeout while waiting for condition"); 
						RETURN_FALSE;
					break;
					
					default: RETURN_FALSE;
				}
			} else {
				zend_error(E_ERROR, "pthreads has detected a failure while attempting to get the time from the system");
				RETURN_FALSE;
			}
		} else {
			switch (pthread_cond_wait(condition, mutex)) {
				case SUCCESS:  RETURN_TRUE; break;
				
				case EINVAL: 
					zend_error(E_WARNING, "pthreads has detected that the condition you are attempting to wait on does not refer to a valid condition variable");
					RETURN_FALSE;
				break;
				
				default: RETURN_FALSE;
			}
		}
	}
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::destroy()
		This will destroy a condition and free the memory associated with it */
PHP_METHOD(Cond, destroy)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition)==SUCCESS && condition) {
		switch (pthread_cond_destroy(condition)) {
			case SUCCESS: 
				free(condition);
				RETURN_TRUE; 
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition variable"); 
			break;
			
			case EBUSY:
				zend_error(E_WARNING, "pthreads has detected an attempt to destroy the object referenced by condition while it is referenced by another thread"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while destroying condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */

/* }}} */
#endif
