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
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_ticks.h>
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_smart_str_public.h>
#include <ext/standard/php_var.h>
#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>
#include "php_pthreads.h"
#include "pthreads_event.h"
#include "pthreads_object.h"

#if COMPILE_DL_PTHREADS
ZEND_GET_MODULE(pthreads)
#endif

zend_class_entry 		*pthreads_class_entry;
zend_class_entry 		*pthreads_mutex_class_entry;
zend_class_entry		*pthreads_condition_class_entry;

/* {{{ defmutex setup 
		Choose the NP type if it exists as it targets the current system*/
pthread_mutexattr_t		defmutex;
#ifdef PTHREAD_MUTEX_ERRORCHECK_NP
#	define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK_NP
#elifdef PTHREAD_MUTEX_ERRORCHECK
#	define DEFAULT_MUTEX_TYPE	PTHREAD_MUTEX_ERRORCHECK
#endif
/* }}} */

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_self, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_busy, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_join, 0, 0, 0)
ZEND_END_ARG_INFO()

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
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_create, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_signal, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_wait, 0, 0, 2)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_broadcast, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ method entries */
zend_function_entry pthreads_methods[] = {
	PHP_ME(Thread, start, 		Thread_start,	ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, self,		Thread_self, 	ZEND_ACC_PROTECTED|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, busy,		Thread_busy, 	ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, join,		Thread_join, 	ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(Thread, run,			NULL)
	{NULL, NULL, NULL}
};

zend_function_entry pthreads_mutex_methods[] = {
	PHP_ME(Mutex, create,		Mutex_create, 	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, lock, 		Mutex_lock, 	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, trylock, 		Mutex_trylock,	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, unlock,		Mutex_unlock,	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, destroy,		Mutex_destroy,	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};

zend_function_entry pthreads_condition_methods[] = {
	PHP_ME(Cond, create,		Cond_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, signal,		Cond_signal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, wait,			Cond_wait, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, broadcast,		Cond_broadcast, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, destroy,		Cond_destroy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ module entry */
zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_PTHREADS_EXTNAME,
  NULL,
  PHP_MINIT(pthreads),
  PHP_MSHUTDOWN(pthreads),
  NULL,
  NULL,
  NULL,
  PHP_PTHREADS_VERSION,
  STANDARD_MODULE_PROPERTIES
};
/* }}} */

/* {{{ module initialization 
		Create class entries, initialize any default objects required 
		@TODO look at thread and condition defaults, maybe setup some sensible defaults */
PHP_MINIT_FUNCTION(pthreads)
{
	zend_class_entry te;
	zend_class_entry me; 
	zend_class_entry ce;
	
	INIT_CLASS_ENTRY(te, "Thread", pthreads_methods);
	te.create_object = pthreads_attach_to_instance;
	te.serialize = zend_class_serialize_deny;
	te.unserialize = zend_class_unserialize_deny;
	pthreads_class_entry=zend_register_internal_class(&te TSRMLS_CC);
	
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

	return SUCCESS;
}
/* }}} */

/* {{{ module shutdown Destroy any default objects created during initialization */
PHP_MSHUTDOWN_FUNCTION(pthreads)
{
	pthread_mutexattr_destroy(&defmutex);
	
	return SUCCESS;
}
/* }}} */

/* {{{ proto boolean Thread::start()
		Starts executing your implemented run method in a thread, will return a boolean indication of success */
PHP_METHOD(Thread, start)
{
	PTHREAD thread = PTHREADS_FETCH;
	int result = -1;
	zval *props;
	
	if (thread && !thread->started->fired) {
		if (zend_hash_find(&Z_OBJCE_P(getThis())->function_table, "run", sizeof("run"), (void**) &thread->runnable)==SUCCESS) {		
		
			ALLOC_INIT_ZVAL(props);
			Z_TYPE_P(props)=IS_ARRAY;
			Z_ARRVAL_P(props)=thread->std.properties;
			thread->serial = pthreads_serialize(props TSRMLS_CC);
			FREE_ZVAL(props);
			
			/* even null serializes, so if we get a NULL pointer then the serialzation failed and we should not continue */
			if (thread->serial == NULL) {
				zend_error(E_ERROR, "The implementation detected unsupported symbols in your thread, please amend your parameters");
				RETURN_FALSE;
			}
			
			/* we do not care if this fails or suceeds */
			zend_hash_find(&Z_OBJCE_P(getThis())->function_table, "__prepare", sizeof("__prepare"), (void**) &thread->prepare);
			
			if ((result = pthread_create(
				&thread->thread, NULL, 
				PHP_PTHREAD_ROUTINE, 
				(void*)thread
			)) == SUCCESS) {
				thread->running = 1;
				pthreads_wait_event(thread->started);
			} else zend_error(E_ERROR, "The implementation detected an internal error while creating thread (%d)", result);
			/* this can't really be raised, the declaration of method entries prohibits it, however could show bugs */
		} else zend_error(E_ERROR, "The implementation cannot read the run method from your implementation");
	} else zend_error(E_WARNING, "The implementation detected that this thread has already been started and it cannot reuse the Thread");
	
	if (result==SUCCESS) {
		RETURN_TRUE;
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto long Thread::self() 
		Will return the current thread id from within a thread */
PHP_METHOD(Thread, self)
{ 
	ZVAL_LONG(return_value, (ulong) pthread_self()); 
}
/* }}} */

/* {{{ proto boolean Thread::busy() 
		Will return true while your method is executing
		NOTE: the zend engine has to shut down after your method has executed, so !busy !== finished */
PHP_METHOD(Thread, busy)
{
	PTHREAD thread = PTHREADS_FETCH;

	if (thread) {
		if (thread->running) {
			if (thread->finished->fired) {
				RETURN_FALSE;
			} else RETURN_TRUE;
		} else zend_error(E_WARNING, "The implementation detected that the requested thread has not yet been started");
	} else zend_error(E_ERROR, "The implementation has expereinced an internal error and cannot continue");
	RETURN_NULL();
}
/* }}} */

/* {{{ proto mixed Thread::join() 
		Will cause the calling thread to block and wait for the output of the referenced thread */
PHP_METHOD(Thread, join) 
{ 
	PTHREAD thread = PTHREADS_FETCH;
	char *result = NULL;
	
	if (thread) {
		if (!thread->joined) {
			thread->joined=1;
			if (pthread_join(thread->thread, (void**)&result)==SUCCESS) {
				if (result) {
					pthreads_unserialize_into(result, return_value TSRMLS_CC);
					free(result);
				}
			} else {
				zend_error(E_WARNING, "The implementation detected failure while joining with the referenced thread");
				RETURN_FALSE;
			}
		} else {
			zend_error(E_WARNING, "The implementation has detected that the value specified by thread has already been joined");
			RETURN_TRUE; 
		}
	} else {
		zend_error(E_ERROR, "The implementation has expereinced an internal error and cannot continue");
		RETURN_FALSE;
	}
}
/* }}} */

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
									zend_error(E_ERROR, "The implementation detected an unspecified error while attempting to lock new mutex");
									pthread_mutex_destroy(mutex);
									free(mutex);
							}
						} else { RETURN_LONG((ulong)mutex); }
					}
				} else { RETURN_LONG((ulong)mutex); }
			break;
		
			case EAGAIN:
				zend_error(E_ERROR, "The implementation detected that the system lacks the necessary resources (other than memory) to initialise another mutex"); 
				free(mutex);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_error(E_ERROR, "The implementation detected that the system lacks the necessary memory to initialise another mutex"); 
				free(mutex);
			break;
			
			case EPERM:
				zend_error(E_ERROR, "The implementation detected that the caller does not have permission to initialize mutex"); 
				free(mutex);
			break;
			
			default: 
				zend_error(E_ERROR, "The implementation detected an internal error while attempting to initialize mutex");
				free(mutex);
		}
	} else zend_error(E_ERROR, "The implementation has failed to allocate memory for mutex and cannot continue");
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto boolean Mutex::lock(long mutex) 
		Locks the mutex referenced so that the calling thread owns the mutex */
PHP_METHOD(Mutex, lock)
{
	pthread_mutex_t *mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mutex)==SUCCESS && mutex) {
		switch (pthread_mutex_lock(mutex)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EDEADLK:
				zend_error(E_WARNING, "The implementation has detected that the calling thread already owns the mutex");
				RETURN_TRUE;
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "The implementation has detected that the variable passed is not a valid mutex");  			
			break;
			
			/* 	the default type of mutex prohibits this error from being raised 
				there may be more control over mutex types in the future so left for completeness */
			case E_ERROR: 
				zend_error(E_WARNING, "The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded");
			break;
			
			default: 
				zend_error(E_ERROR, "The implementation detected an internal error while locking mutex and cannot continue");
		}
	}
	RETURN_FALSE;
}
/* }}} */

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
				zend_error(E_WARNING, "The implementation has detected that the calling thread already owns the mutex");
				RETURN_TRUE;
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "The implementation has detected that the variable passed is not a valid mutex");  
			break;
			
			/* again the defmutex setup prohibits this from occuring, present for completeness */
			case EAGAIN: 
				zend_error(E_WARNING, "The implementation detected that the mutex could not be acquired because the maximum number of recursive locks has been exceeded");
			break;
			
			default: 
				zend_error(E_ERROR, "The implementation detected an internal error while trying to lock mutex and cannot continue");
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto boolean Mutex::unlock(long mutex) 
		Will unlock the mutex referenced */
PHP_METHOD(Mutex, unlock)
{
	pthread_mutex_t *mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mutex)==SUCCESS && mutex) {
		switch (pthread_mutex_unlock(mutex)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL: 
				zend_error(E_WARNING, "The implementation has detected that the variable passed is not a valid mutex"); 
			break;
			
			case EPERM:
				zend_error(E_WARNING, "The implementation has detected that the calling thread does not own the mutex");
			break;
			
			default:
				zend_error(E_ERROR, "The implementation detected an internal error while unlocking mutex and cannot continue");
		}
	}
	RETURN_FALSE;
}
/* }}} */

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
				zend_error(E_WARNING, "The implementation has detected an attempt to destroy mutex while it is locked or referenced"); 
			break;
			
			case EINVAL:
				zend_error(E_WARNING, "The implementation has detected that the variable passed is not a valid mutex"); 
			break;
			
			default:
				zend_error(E_ERROR, "The implementation detected an internal error while attempting to destroy mutex and cannot continue");
		}
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto long Cond::create() 
		This will create a condition, because of their nature you need to destroy these explicitly with Cond::destroy */
PHP_METHOD(Cond, create)
{
	pthread_cond_t *condition;
	
	if ((condition=(pthread_cond_t*) calloc(1, sizeof(pthread_cond_t)))!=NULL) {
		switch (pthread_cond_init(condition, NULL)) {
			case SUCCESS: RETURN_LONG((ulong)condition); break;
			
			case EAGAIN:
				zend_error(E_ERROR, "The implementation detected that the system lacks the necessary resources (other than memory) to initialise another condition"); 
				free(condition);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_error(E_ERROR, "The implementation detected that the system lacks the necessary memory to initialise another condition"); 
				free(condition);
			break;
			
			default: 
				zend_error(E_ERROR, "The implementation detected an internal error while initializing new condition and cannot conitnue");
				free(condition);
		}
	} else zend_error(E_ERROR, "The implementation failed to allocate memory for new condition and cannot continue");
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto boolean Cond::signal(long condition) 
		This will signal a condition */
PHP_METHOD(Cond, signal)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition) == SUCCESS && condition) {
		switch (pthread_cond_signal(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL: 
				zend_error(E_WARNING, "The implementation has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "The implementation detected an internal error while signaling condition and cannot continue");
		}
	} 
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto boolean Cond::broadcast(long condition) 
		This will broadcast a condition */
PHP_METHOD(Cond, broadcast)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition)==SUCCESS && condition) {
		switch (pthread_cond_broadcast(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL:
				zend_error(E_WARNING, "The implementation has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "The implementation detected an internal error while broadcasting condition and cannot continue");
		}
	} 
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto boolean Cond::wait(long condition, long mutex) 
		This will wait for a signal or broadcast on condition, you must have mutex locked by the calling thread */
PHP_METHOD(Cond, wait)
{
	pthread_cond_t 		*condition;
	pthread_mutex_t 	*mutex;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &condition, &mutex)==SUCCESS && condition && mutex) {
		switch (pthread_cond_wait(condition, mutex)) {
			case SUCCESS:  RETURN_TRUE;  break;
			
			case EINVAL: 
				zend_error(E_WARNING, "The implementation has detected the condition referenced does not refer to a valid conditiond"); 
			break;
			
			default:
				zend_error(E_ERROR, "Yhe implementation detected an internal error while waiting for condition and cannot continue");
		}
	} 
	RETURN_FALSE;
}
/* }}} */

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
				zend_error(E_WARNING, "The implementation has detected the condition referenced does not refer to a valid condition variable"); 
			break;
			
			case EBUSY:
				zend_error(E_WARNING, "The implementation has detected an attempt to destroy the object referenced by condition while it is referenced by another thread"); 
			break;
			
			default:
				zend_error(E_ERROR, "The implementation detected an internal error while destroying condition and cannot continue");
		}
	} 
	RETURN_FALSE;
}
/* }}} */
#endif
