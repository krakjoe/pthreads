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
PHP_METHOD(Thread, detach);
PHP_METHOD(Thread, isStarted);
PHP_METHOD(Thread, isRunning);
PHP_METHOD(Thread, isJoined);
PHP_METHOD(Thread, isWaiting);
PHP_METHOD(Thread, isTerminated);
PHP_METHOD(Thread, getThreadId);
PHP_METHOD(Thread, getCreatorId);

PHP_METHOD(Thread, synchronized);
PHP_METHOD(Thread, lock);
PHP_METHOD(Thread, unlock);

PHP_METHOD(Thread, merge);
PHP_METHOD(Thread, shift);
PHP_METHOD(Thread, pop);
PHP_METHOD(Thread, chunk);

PHP_METHOD(Thread, getTerminationInfo);

ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
    ZEND_ARG_INFO(0, options)
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

ZEND_BEGIN_ARG_INFO_EX(Thread_detach, 0, 0, 0)
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

ZEND_BEGIN_ARG_INFO_EX(Thread_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getTerminationInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_synchronized, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_merge, 0, 0, 1)
    ZEND_ARG_INFO(0, from)
    ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_chunk, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
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
    PHP_ME(Thread, detach, Thread_detach, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isRunning, Thread_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isWaiting, Thread_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, isTerminated, Thread_isTerminated, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getTerminationInfo, Thread_getTerminationInfo, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_STATIC)
	PHP_ME(Thread, getCreatorId, Thread_getCreatorId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, synchronized, Thread_synchronized, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, lock, Thread_lock, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, unlock, Thread_lock, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, merge, Thread_merge, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, shift, Thread_shift, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, pop, Thread_pop, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Thread, chunk, Thread_chunk, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};
/* {{{ proto boolean Thread::start([long $options = PTHREADS_INHERIT_ALL])
		Starts executing the implementations run method in a thread, will return a boolean indication of success
		$options should be a mask of inheritance constants */
PHP_METHOD(Thread, start)
{
	PTHREAD thread = PTHREADS_FETCH;
	int result = FAILURE;
	long options = PTHREADS_INHERIT_ALL;
	
	/* get options */
	if (ZEND_NUM_ARGS()) {
	    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &options) != SUCCESS) {
	        return;
	    }
	    
	    /* set thread options */
	    thread->options = options;
	}
	
	/*
	* See if there are any limits in this environment
	*/
	if (PTHREADS_IS_NOT_CONNECTION(thread)) {
		/* attempt to create the thread */
		if ((result = pthreads_start(thread TSRMLS_CC)) != SUCCESS) {
			/*
			* Report the error, there's no chance of recovery ...
			*/
			switch(result) {
				case PTHREADS_ST_STARTED:
					zend_error(E_WARNING, "pthreads has detected an attempt to start %s (%lu), which has been previously started", PTHREADS_FRIENDLY_NAME);
				break;
				
				case EAGAIN:
					zend_error(E_WARNING, "pthreads has detected that the %s could not be started, the system lacks the necessary resources or the system-imposed limit would be exceeded", PTHREADS_NAME);
				break;
				
				default: zend_error(E_WARNING, "pthreads has detected that the %s could not be started because of an unspecified system error (%d)", PTHREADS_NAME, result);
			}
		}
	} else zend_error(E_WARNING, "pthreads has detected an attempt to start %s from an invalid context, the creating context must start %s", PTHREADS_NAME, PTHREADS_NAME);
	
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

/* {{{ proto boolean Thread::isTerminated() 
	Will return true if the referenced thread suffered fatal errors or uncaught exceptions */
PHP_METHOD(Thread, isTerminated)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Thread::getTerminationInfo() 
	Will return information concerning the location of the termination to aid debugging */
PHP_METHOD(Thread, getTerminationInfo)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC)) {
		    array_init(return_value);
		    
		    if (thread->error->clazz) {
		        add_assoc_string(
		            return_value, "scope", thread->error->clazz, 1);       
		    }
		    
		    if (thread->error->method) {
		        add_assoc_string(
		            return_value, "function", thread->error->method, 1);
		    }
		    
		    if (thread->error->file) {
		        add_assoc_string(
		            return_value, "file", thread->error->file, 1);
		        add_assoc_long(return_value, "line", thread->error->line);
		    }
		} else {
		    RETURN_FALSE;
		}
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ 	proto boolean Thread::wait([long timeout])
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(Thread, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	long timeout = 0L;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &timeout)==SUCCESS) {
		if (ZEND_NUM_ARGS()) {
			RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, timeout TSRMLS_CC));
		} else RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, 0L TSRMLS_CC));
	}
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
	if (PTHREADS_IS_NOT_CONNECTION(thread)) {
		/*
		* Ensure this thread was started
		*/
		RETURN_BOOL((pthreads_join(thread TSRMLS_CC)==SUCCESS));
	} else {
		zend_error(E_WARNING, "pthreads has detected an attempt to join from an incorrect context, only the creating context may join with %s (%lu)", PTHREADS_FRIENDLY_NAME);
		RETURN_FALSE;
	}
} /* }}} */

/* {{{ proto boolean Thread::detach()
        Will return a boolean indication of success */

PHP_METHOD(Thread, detach)
{
    PTHREAD thread = PTHREADS_FETCH;
    int result = FAILURE;

    if (thread) {
        result = pthreads_detach(thread, 0);

        if (result != SUCCESS) {
            RETURN_FALSE;
        }
        
        RETURN_TRUE;
    }

    RETURN_FALSE;
} /* }}} */

/* {{{ proto long Thread::getThreadId()
	Will return the identifier of the referenced Thread */
PHP_METHOD(Thread, getThreadId)
{
	if (getThis()) {
		ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->tid);
	} else ZVAL_LONG(return_value, pthreads_self());
} /* }}} */

/* {{{ proto long Thread::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Thread */
PHP_METHOD(Thread, getCreatorId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->cid);
} /* }}} */

/* {{{ proto void Thread::synchronized(Callable function, ...)
	Will synchronize the thread, call the function, passing anything after the function as parameters
	 */
PHP_METHOD(Thread, synchronized) 
{
	zend_fcall_info *info = emalloc(sizeof(zend_fcall_info));
	zend_fcall_info_cache *cache = emalloc(sizeof(zend_fcall_info_cache));
	
	uint argc = 0;
	zval ***argv = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|+", info, cache, &argv, &argc) == SUCCESS) {
		pthreads_synchro_block(getThis(), info, cache, argc, argv, return_value TSRMLS_CC);
	}
	
	if (argc) 
		efree(argv);	
	efree(info);
	efree(cache);
} /* }}} */

/* {{{ proto boolean Thread::lock()
	Will acquire the storage lock */
PHP_METHOD(Thread, lock) 
{
	ZVAL_BOOL(return_value, pthreads_store_lock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Thread::unlock()
	Will release the storage lock */
PHP_METHOD(Thread, unlock) 
{
	ZVAL_BOOL(return_value, pthreads_store_unlock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Thread::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Thread */
PHP_METHOD(Thread, merge) 
{
    zval *from;
    zend_bool overwrite = 1;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &from, &overwrite) != SUCCESS) {
        return;
    }
    
	RETURN_BOOL((pthreads_store_merge(getThis(), from, overwrite TSRMLS_CC)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Thread::shift()
	Will shift the first member from the object */
PHP_METHOD(Thread, shift) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_shift(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Thread::pop()
	Will pop the last member from the object */
PHP_METHOD(Thread, pop) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_pop(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Thread::chunk(integer $size [, boolean $preserve])
	Will shift the first member from the object */
PHP_METHOD(Thread, chunk) 
{
    long size;
    zend_bool preserve = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &size, &preserve) != SUCCESS) {
        return;
    }
    
    pthreads_store_chunk(getThis(), size, preserve, &return_value TSRMLS_CC);
} /* }}} */
#	endif
#endif
