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
PHP_METHOD(Thread, join);
PHP_METHOD(Thread, detach);
PHP_METHOD(Thread, isStarted);
PHP_METHOD(Thread, isJoined);
PHP_METHOD(Thread, getThreadId);
PHP_METHOD(Thread, getCurrentThreadId);
PHP_METHOD(Thread, getCurrentThread);
PHP_METHOD(Thread, getCreatorId);
PHP_METHOD(Thread, kill);
PHP_METHOD(Thread, globally);

ZEND_BEGIN_ARG_INFO_EX(Thread_globally, 0, 0, 1)
	ZEND_ARG_INFO(0, block)
	ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_join, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_detach, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getThreadId, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getCurrentThreadId, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getCurrentThread, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_getCreatorId, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isStarted, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isJoined, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_merge, 0, 0, 1)
    ZEND_ARG_INFO(0, from)
    ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_kill, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_thread_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREAD
#	define HAVE_PTHREADS_CLASS_THREAD
zend_function_entry pthreads_thread_methods[] = {
	PHP_ME(Thread, start, Thread_start, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC)
    PHP_ME(Thread, detach, Thread_detach, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getCreatorId, Thread_getThreadId, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getCurrentThreadId, Thread_getCurrentThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Thread, getCurrentThread, Thread_getCurrentThread, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Thread, kill, Thread_kill, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, globally, Thread_globally, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
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
					zend_throw_exception_ex(
						spl_ce_RuntimeException, 0 TSRMLS_CC, 
						"pthreads has detected an attempt to start %s (%lu), which has been previously started", PTHREADS_FRIENDLY_NAME);
				break;
				
				case EAGAIN:
					zend_throw_exception_ex(
						spl_ce_RuntimeException, 0 TSRMLS_CC, 
						"pthreads has detected that the %s could not be started, the system "
						"lacks the necessary resources or the system-imposed limit would be exceeded", PTHREADS_FRIENDLY_NAME);
				break;
				
				default:
					zend_throw_exception_ex(
						spl_ce_RuntimeException, 0 TSRMLS_CC, 
						"pthreads has detected that the %s could not be started because of an unspecified system error (%d)", PTHREADS_NAME, result);
			}
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has detected an attempt to start %s from an invalid context, the creating context must start %s", PTHREADS_NAME, PTHREADS_NAME);
	}
	
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
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);
	}
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
PHP_METHOD(Thread, isJoined)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);
	}
} /* }}} */

/* {{{ proto boolean Thread::isWaiting() 
	Will return true if the referenced thread is waiting for notification */
PHP_METHOD(Thread, isWaiting)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);
	}
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
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC,
			"pthreads has detected an attempt to shutdown %s (%lu) from an incorrect context", PTHREADS_FRIENDLY_NAME);
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
    } else {
    	zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC,
			"pthreads has experienced an internal error while preparing to detach a %s", PTHREADS_NAME);
    }
} /* }}} */

/* {{{ proto long Thread::getThreadId()
	Will return the identifier of the referenced Thread */
PHP_METHOD(Thread, getThreadId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->tid);
} /* }}} */

/* {{{ proto long Thread::getCurrentThreadId()
	Will return the identifier of the current Thread */
PHP_METHOD(Thread, getCurrentThreadId)
{
	ZVAL_LONG(return_value, pthreads_self());
} /* }}} */

/* {{{ proto Thread Thread::getCurrentThread()
	Will return the currently executing Thread */
PHP_METHOD(Thread, getCurrentThread)
{
	pthreads_current_thread(&return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto long Thread::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Thread */
PHP_METHOD(Thread, getCreatorId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->cid);
} /* }}} */

/* {{{ proto boolean Thread::kill()
	Will kill the referenced thread, forcefully */
PHP_METHOD(Thread, kill) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
#ifdef PTHREADS_KILL_SIGNAL
    {
    	PTHREAD thread = PTHREADS_FETCH;
    	/* allowing sending other signals here is just too dangerous */
    	RETURN_BOOL(
    		pthread_kill(thread->thread, PTHREADS_KILL_SIGNAL)==SUCCESS);
    }
#endif
} /* }}} */

/* {{{ proto mixed Thread::globally(Callable block [, ... args])
	Will execute the block in the global scope */
PHP_METHOD(Thread, globally) 
{
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
	zval ***argv = NULL;
	zend_uint argc = 0;
	zval *retval = NULL;
	zend_bool failed = 0;
	HashTable *symbols = EG(active_symbol_table);
	
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "f|+", &fci, &fcc, &argv, &argc) != SUCCESS) {
        return;
    }
    
    /* set argc and argv for function call */
	zend_fcall_info_argp(&fci TSRMLS_CC, argc, argv);
	
	fci.retval_ptr_ptr = &retval;
	fci.symbol_table = &EG(symbol_table);
	
	zend_try {
		zend_call_function(&fci, &fcc TSRMLS_CC);
	} zend_catch {
		failed = 1;
	} zend_end_try();
	
	zend_fcall_info_args_clear(&fci, 1);
	
	/* return the result */
	if (!failed && retval) {
		ZVAL_ZVAL(return_value, retval, 1, 1);
	} else {
		ZVAL_NULL(return_value);
	}
} /* }}} */
#	endif
#endif
