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
#ifndef HAVE_PHP_PTHREADS
#define HAVE_PHP_PTHREADS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PHP_PTHREADS_H
#	include <php_pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STATE_H
#	include <src/state.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#ifndef HAVE_PTHREADS_SERIAL_H
#	include <src/serial.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if COMPILE_DL_PTHREADS
	ZEND_GET_MODULE(pthreads)
#endif

/* {{{ defmutex setup 
		Choose the NP type if it exists as it targets the current system */
#ifndef _WIN32
#	ifdef PTHREAD_MUTEX_ERRORCHECK_NP
#		define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK_NP
#	elifdef PTHREAD_MUTEX_ERRORCHECK
#		define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK
#	endif
#endif
/* }}} */

#ifndef HAVE_PTHREADS_SERIAL_H
#include <src/serial.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#include <src/globals.h>
#endif

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
ZEND_BEGIN_ARG_INFO_EX(Thread_getCreatorId, 0, 0, 0)
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

ZEND_BEGIN_ARG_INFO_EX(Worker_isWorking, 0, 0, 0)
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

/* {{{ mutex */
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

/* {{{ conditions */
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

/* {{{ Thread method entries */
zend_function_entry pthreads_thread_methods[] = {
	PHP_ME(Thread, start, Thread_start, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(Thread, run, Thread_run)
	PHP_ME(Thread, wait, Thread_wait, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, notify, Thread_notify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isRunning, Thread_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isWaiting, Thread_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getThread, Thread_getThread, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_STATIC)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getCreatorId, Thread_getCreatorId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getCount, Thread_getCount, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getMax, Thread_getMax, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getPeak, Thread_getPeak, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ Worker method entries */
zend_function_entry pthreads_worker_methods[] = {
	PHP_ME(Thread, start, Thread_start, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(Thread, run, Thread_run)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getCreatorId, Thread_getCreatorId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, stack, Thread_stack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, unstack, Thread_unstack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, isWorking, Worker_isWorking, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getStacked, Thread_getStacked, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ Stackable method entries */
zend_function_entry pthreads_stackable_methods[] = {
	PHP_ABSTRACT_ME(Thread, run, Thread_run)
	PHP_ME(Thread, wait, Thread_wait, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, notify, Thread_notify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isRunning, Thread_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isWaiting, Thread_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getCreatorId, Thread_getCreatorId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
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

/* {{{ Brains of the operation */
#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif /* }}} */

/* {{{ Access modification */
#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif /* }}} */

/* {{{ Friendly Thread Display */
#ifndef PTHREADS_NAME
#	define PTHREADS_NAME Z_OBJCE_P(getThis())->name
#endif

#ifndef PTHREADS_TID
#	define PTHREADS_TID thread->tid
#endif

#ifndef PTHREADS_FRIENDLY_NAME
#	define PTHREADS_FRIENDLY_NAME PTHREADS_NAME, PTHREADS_TID
#endif /* }}} */

/* {{{ module initialization */
PHP_MINIT_FUNCTION(pthreads)
{
	zend_class_entry te;
	zend_class_entry me;
	zend_class_entry ce;
	zend_class_entry se;
	zend_class_entry we;
	
	REGISTER_INI_ENTRIES();

	/*
	* Global Init
	*/
	if (!PTHREADS_G(init)) 
		pthreads_globals_init();
	
	INIT_CLASS_ENTRY(te, "Thread", pthreads_thread_methods);
	te.create_object = pthreads_object_thread_ctor;
	te.serialize = zend_class_serialize_deny;
	te.unserialize = zend_class_unserialize_deny;
	pthreads_thread_entry=zend_register_internal_class(&te TSRMLS_CC);
	
	INIT_CLASS_ENTRY(we, "Worker", pthreads_worker_methods);
	we.create_object = pthreads_object_worker_ctor;
	we.serialize = zend_class_serialize_deny;
	we.unserialize = zend_class_unserialize_deny;
	pthreads_worker_entry=zend_register_internal_class(&we TSRMLS_CC);
	
	INIT_CLASS_ENTRY(se, "Stackable", pthreads_stackable_methods);
	se.create_object = pthreads_object_stackable_ctor;
	se.serialize = zend_class_serialize_deny;
	se.unserialize = zend_class_unserialize_deny;
	pthreads_stackable_entry=zend_register_internal_class(&se TSRMLS_CC);
	
	INIT_CLASS_ENTRY(me, "Mutex", pthreads_mutex_methods);
	me.serialize = zend_class_serialize_deny;
	me.unserialize = zend_class_unserialize_deny;
	pthreads_mutex_entry=zend_register_internal_class(&me TSRMLS_CC);
	
	INIT_CLASS_ENTRY(ce, "Cond", pthreads_condition_methods);
	ce.serialize = zend_class_serialize_deny;
	ce.unserialize = zend_class_unserialize_deny;
	pthreads_condition_entry=zend_register_internal_class(&ce TSRMLS_CC);

	if ( pthread_mutexattr_init(&defmutex)==SUCCESS ) {
#ifdef DEFAULT_MUTEX_TYPE
		pthread_mutexattr_settype(
			&defmutex, 
			DEFAULT_MUTEX_TYPE
		);
#endif
	}
	
	/*
	* Setup standard and pthreads object handlers
	*/
	zend_handlers = zend_get_std_object_handlers();
	
	memcpy(&pthreads_handlers, zend_handlers, sizeof(zend_object_handlers));
	
	pthreads_handlers.get_method = pthreads_get_method;
	pthreads_handlers.call_method = pthreads_call_method;
	pthreads_handlers.read_property = pthreads_read_property;
	pthreads_handlers.write_property = pthreads_write_property;
	pthreads_handlers.has_property = pthreads_has_property;
	pthreads_handlers.unset_property = pthreads_unset_property;
	pthreads_handlers.get_property_ptr_ptr = NULL;
	
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
	if (PTHREADS_G(max)) {
		php_info_print_table_row(2, "Maximum Threads", INI_STR("pthreads.max"));
	} else php_info_print_table_row(2, "Maximum Threads", "No Limits");
	
	snprintf(numbers, sizeof(numbers), "%d", pthreads_globals_count());
	php_info_print_table_row(2, "Current Threads", numbers);
	
	snprintf(numbers, sizeof(numbers), "%d", pthreads_globals_peak());
	php_info_print_table_row(2, "Peak Threads", numbers);

	php_info_print_table_end();
} /* }}} */

/* {{{ proto boolean Thread::start([boolean sync])
		Starts executing your implemented run method in a thread, will return a boolean indication of success
		If sync is true, your new thread runs synchronized with the current thread ( causing the current thread to block ) until you call notify from within the new thread or the thread ends
		If sync is false or void, the new thread will run asynchronously to the current thread and this call will return as quickly as possible */
PHP_METHOD(Thread, start)
{
	PTHREAD thread = PTHREADS_FETCH;
	int result = FAILURE;
	
	/*
	* See if there are any limits in this environment
	*/
	if (PTHREADS_G(max) && !(pthreads_globals_count()<PTHREADS_G(max))) {
		zend_error(E_WARNING, "pthreads has reached the maximum numbers of threads allowed (%lu) by your server administrator, and cannot start %s", PTHREADS_G(max), PTHREADS_NAME);
		RETURN_FALSE;
	}
	
	/*
	* Find out if the calling thread will wait for notification before continuing
	*/
	if (ZEND_NUM_ARGS()){
		zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "b", &thread->synchronized);
	} else thread->synchronized = 0;
	
	if (!pthreads_state_isset(thread->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
		int acquire = pthread_mutex_lock(thread->lock);
			
		if (acquire == SUCCESS || acquire == EDEADLK) {
			/*
			* Don't allow restarting this object
			*/ 
			pthreads_set_state(thread, PTHREADS_ST_STARTED TSRMLS_CC);
			
			/*
			* Set running flag and store thread in globals
			*/
			pthreads_set_state(thread, PTHREADS_ST_RUNNING TSRMLS_CC);
			
			/*
			* Attempt to start the thread
			*/
			if ((result = pthread_create(&thread->thread, NULL, PHP_PTHREAD_ROUTINE, (void*)thread)) == SUCCESS) {

				/*
				* Wait for notification to continue
				*/
				pthreads_set_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
				
				/*
				* Release thread lock
				*/
				if (acquire != EDEADLK) 
					pthread_mutex_unlock(thread->lock);
				
			} else {
				/*
				* Do not attempt to join failed threads at dtor time
				*/
				pthreads_unset_state(thread, PTHREADS_ST_RUNNING TSRMLS_CC);
				
				/*
				* Release thread lock
				*/
				if (acquire != EDEADLK)
					pthread_mutex_unlock(thread->lock);
				
				/*
				* Report the error, there's no chance of recovery ...
				*/
				switch(result) {
					case EAGAIN:
						zend_error_noreturn(E_WARNING, "pthreads has detected that the %s could not be started, the system lacks the necessary resources or the system-imposed limit would be exceeded", PTHREADS_NAME);
					break;
					
					default: zend_error_noreturn(E_WARNING, "pthreads has detected that the %s could not be started because of an unspecified system error", PTHREADS_NAME);
				}
			}
		} else zend_error_noreturn(E_WARNING, "pthreads has experienced an internal error while starting %s", PTHREADS_NAME);
	} else zend_error_noreturn(E_WARNING, "pthreads has detected an attempt to start %s (%lu) that has already been started", PTHREADS_FRIENDLY_NAME);	
	
	if (result==SUCCESS) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
} /* }}} */

/* {{{ proto int Thread::stack(Stackable $work)
	Pushes an item onto the Thread Stack, returns the size of stack */
PHP_METHOD(Thread, stack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval 	*work;
	int		size;
	
	if (thread) {
		if (PTHREADS_IS_NOT_CONNECTION(thread)) {
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
				if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, pthreads_stackable_entry)==SUCCESS) {
					size = pthreads_stack_push(thread, PTHREADS_FETCH_FROM(work) TSRMLS_CC);
					if (size) {
						RETURN_LONG(size);
					}
				}
			} else zend_error(E_ERROR, "pthreads has detected an attempt to stack onto %s (%lu) which has already been joined", PTHREADS_FRIENDLY_NAME);
		} else zend_error(E_WARNING, "pthreads has detected an attempt to call an external method on %s (%lu), you cannot stack in this context", PTHREADS_FRIENDLY_NAME);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while stacking onto %s (%lu) and cannot continue", PTHREADS_FRIENDLY_NAME);
	RETURN_FALSE;
} /* }}} */

/* {{{ proto int Thread::unstack([Stackable $work])
	Removes an item from the Thread stack, if no item is specified the stack is cleared, returns the size of stack */
PHP_METHOD(Thread, unstack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval * work;
	
	if (thread) {
		if (PTHREADS_IS_NOT_CONNECTION(thread)) {
			if (ZEND_NUM_ARGS() > 0) {
				if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, pthreads_stackable_entry)==SUCCESS) {
					RETURN_LONG(pthreads_stack_pop(thread, PTHREADS_FETCH_FROM(work) TSRMLS_CC));
				}
			} else RETURN_LONG(pthreads_stack_pop(thread, NULL TSRMLS_CC));
		} else zend_error(E_WARNING, "pthreads has detected an attempt to call an external method on %s (%lu), you cannot unstack in this context", PTHREADS_FRIENDLY_NAME);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while unstacking from %s (%lu) and cannot continue", PTHREADS_FRIENDLY_NAME);
	RETURN_FALSE;
}

/* {{{ proto int Thread::getStacked()
	Returns the current size of the stack */
PHP_METHOD(Thread, getStacked)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_LONG(pthreads_stack_length(thread TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while getting the stack length of a %s and cannot continue", PTHREADS_NAME);
	RETURN_FALSE;
}

/* {{{ proto Thread::isStarted() 
	Will return true if a Thread has been started */
PHP_METHOD(Thread, isStarted)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_STARTED TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto Thread::isRunning() 
	Will return true while a thread is executing */
PHP_METHOD(Thread, isRunning)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
PHP_METHOD(Thread, isJoined)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Thread::isWaiting() 
	Will return true if the referenced thread is waiting for notification */
PHP_METHOD(Thread, isWaiting)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Worker::isWorking() 
	Will return false if the worker is waiting for work */
PHP_METHOD(Worker, isWorking)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(!pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Thread::wait([long timeout]) 
		Will cause the calling process or thread to wait for the referenced thread to notify
		When a timeout is used an reached boolean false will return
		When no timeout is used a boolean indication of success will return */
PHP_METHOD(Thread, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	long timeout = 0L;
	
	if(ZEND_NUM_ARGS()){
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &timeout)!=SUCCESS) {
			RETURN_FALSE;
		}
	}
	
	if (thread) {
		RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, timeout TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to wait for a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Thread::notify()
		Notify anyone waiting that they can continue
		Will return a boolean indication of success */
PHP_METHOD(Thread, notify)
{
	PTHREAD thread = PTHREADS_FETCH;
	if (thread) {
		RETURN_BOOL(pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC));
	}else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to notify a %s and cannot continue", PTHREADS_NAME);
	RETURN_FALSE;
} /* }}} */

/* {{{ proto mixed Thread::join() 
		Will cause the calling thread to block and wait for the exit status of the referenced thread */
PHP_METHOD(Thread, join) 
{ 
	PTHREAD thread = PTHREADS_FETCH;
	
	/*
	* Check that we are in the correct context
	*/
	if (!PTHREADS_IN_THREAD(thread) && PTHREADS_IS_CREATOR(thread)) {

		/*
		* Ensure this thread was started
		*/
		if (pthreads_state_isset(thread->state, PTHREADS_ST_STARTED TSRMLS_CC)) {
		
			/*
			* Ensure this thread wasn't already joined
			*/
			if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			
				/*
				* Ensure nothing attempts to join this thread again
				*/
				pthreads_set_state(thread, PTHREADS_ST_JOINED TSRMLS_CC);
				
				/*
				* We must force workers to close at this point
				*/
				{
					if (PTHREADS_IS_WORKER(thread)) {
						if (pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC)) {
							do {
								pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC);
							} while (pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
						}
					}
				}
				
				/*
				* Carry out joining
				*/
				if (pthread_join(thread->thread, NULL)==SUCCESS) {
					RETURN_LONG(*thread->status);
				} else {
					zend_error(E_WARNING, "pthreads detected failure while joining with %s (%lu)", PTHREADS_FRIENDLY_NAME);
					RETURN_FALSE;
				}
			} else {
				zend_error(E_WARNING, "pthreads has detected that %s (%lu) has already been joined", PTHREADS_FRIENDLY_NAME);
				RETURN_FALSE; 
			}
		} else {
			zend_error(E_WARNING, "pthreads has detected an attempt to join with an unstarted %s", PTHREADS_NAME);
			RETURN_FALSE;
		}
	} else {
		zend_error(E_WARNING, "pthreads has detected an attempt to join from an incorrect context, only the creating context may join with %s (%lu)", PTHREADS_FRIENDLY_NAME);
		RETURN_FALSE;
	}
} /* }}} */

/* {{{ proto boolean Thread::getThread(long $tid)
		Thread::find shall attempt to import the Thread identified by tid into the current context */
PHP_METHOD(Thread, getThread)
{
	PTHREAD found = NULL;
	ulong tid = 0L;
	
	if (PTHREADS_G(importing)) {
		if (EG(called_scope) == pthreads_thread_entry) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &tid)==SUCCESS) {
				if (tid && tid >0L) {
					if ((found=pthreads_globals_find((ulong)tid))!=NULL) {
						if (!PTHREADS_IS_CREATOR(found)) {
							if (pthreads_import(found, &return_value TSRMLS_CC)) {
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
	Will return the identifier of the referenced Thread 
	If executed on a Stackable will return the identifier of the Worker executing the Stackable */
PHP_METHOD(Thread, getThreadId)
{
	if (getThis()) {
		PTHREAD thread = PTHREADS_FETCH;
		if (thread) {
			ZVAL_LONG(return_value, thread->tid);
		} else ZVAL_LONG(return_value, 0L);
	} else ZVAL_LONG(return_value, pthreads_self());
} /* }}} */

/* {{{ proto long Thread::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Thread or Worker */
PHP_METHOD(Thread, getCreatorId)
{
	if (getThis()) {
		PTHREAD thread = PTHREADS_FETCH;
		if (thread) {
			ZVAL_LONG(return_value, thread->cid);
		} else ZVAL_LONG(return_value, 0L);
	} else ZVAL_LONG(return_value, 0L);
} /* }}} */

/* {{{ proto long Thread::getCount()
		Will return the number of threads currently running */
PHP_METHOD(Thread, getCount)
{
	RETURN_LONG(pthreads_globals_count());
} /* }}} */

/* {{{ proto long Thread::getMax()
		Will return the maximum number of threads you may create in this environment
		@NOTE this setting very much depends on the kind of environment you're executing php in */
PHP_METHOD(Thread, getMax)
{
	RETURN_LONG(PTHREADS_G(max));
} /* }}} */

/* {{{ proto long Thread::getPeak() 
	Will return the peak number of threads executing */
PHP_METHOD(Thread, getPeak)
{
	RETURN_LONG(pthreads_globals_peak());
} /* }}} */

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
			if (gettimeofday(&now, NULL)==SUCCESS) {
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

#endif
