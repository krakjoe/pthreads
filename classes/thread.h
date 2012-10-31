/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                               		 |
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
#ifndef HAVE_PTHREADS_CLASS_THREAD_H
#define HAVE_PTHREADS_CLASS_THREAD_H
PHP_METHOD(Thread, start); 
PHP_METHOD(Thread, wait);
PHP_METHOD(Thread, notify);
PHP_METHOD(Thread, join);
PHP_METHOD(Thread, isStarted);
PHP_METHOD(Thread, isRunning);
PHP_METHOD(Thread, isJoined);
PHP_METHOD(Thread, isWaiting);
PHP_METHOD(Thread, getThread);
PHP_METHOD(Thread, getThreadId);
PHP_METHOD(Thread, getCreatorId);
PHP_METHOD(Thread, getCount);
PHP_METHOD(Thread, getMax);
PHP_METHOD(Thread, getPeak);

ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_wait, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_join, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getThread, 0, 0, 1)
	ZEND_ARG_INFO(0, tid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getThreadId, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Thread_getCreatorId, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isStarted, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isJoined, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isWaiting, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getCount, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getMax, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getPeak, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_thread_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREAD
#	define HAVE_PTHREADS_CLASS_THREAD
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
};
/* {{{ proto boolean Thread::start()
		Starts executing the implementations run method in a thread, will return a boolean indication of success */
PHP_METHOD(Thread, start)
{
	PTHREAD thread = PTHREADS_FETCH;
	int result = FAILURE;
	
	/*
	* See if there are any limits in this environment
	*/
	if (PTHREADS_IS_NOT_CONNECTION(thread)) {
		if (PTHREADS_G(max) && !(pthreads_globals_count()<PTHREADS_G(max))) {
			zend_error(E_WARNING, "pthreads has reached the maximum numbers of threads allowed (%lu) by your server administrator, and cannot start %s", PTHREADS_G(max), PTHREADS_NAME);
			RETURN_FALSE;
		}
		
		/* attempt to create the thread */
		if ((result = pthreads_start(thread TSRMLS_CC)) != SUCCESS) {
			/*
			* Report the error, there's no chance of recovery ...
			*/
			switch(result) {
				case PTHREADS_ST_STARTED:
					zend_error_noreturn(E_WARNING, "pthreads has detected an attempt to start %s (%lu), which has been previously started", PTHREADS_FRIENDLY_NAME);
				break;
				
				case EAGAIN:
					zend_error_noreturn(E_WARNING, "pthreads has detected that the %s could not be started, the system lacks the necessary resources or the system-imposed limit would be exceeded", PTHREADS_NAME);
				break;
				
				default: zend_error_noreturn(E_WARNING, "pthreads has detected that the %s could not be started because of an unspecified system error (%d)", PTHREADS_NAME, result);
			}
		}
	} else zend_error_noreturn(E_WARNING, "pthreads has detected an attempt to start %s from an invalid context, the creating context must start %s", PTHREADS_NAME, PTHREADS_NAME);
	
	if (result==SUCCESS) {
		RETURN_TRUE;
	}
	
	RETURN_FALSE;
} /* }}} */

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

/* {{{ proto boolean Thread::join() 
		Will return a boolean indication of success */
PHP_METHOD(Thread, join) 
{ 
	PTHREAD thread = PTHREADS_FETCH;
	
	/*
	* Check that we are in the correct context
	*/
	if (PTHREADS_IN_CREATOR(thread)) {
		/*
		* Ensure this thread was started
		*/
		RETURN_BOOL((pthreads_join(thread TSRMLS_CC)==SUCCESS));
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
						if (!PTHREADS_IN_CREATOR(found)) {
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
		Will return the number of threads currently accessible */
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
#	endif
#endif
