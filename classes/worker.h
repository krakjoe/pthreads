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
#ifndef HAVE_PTHREADS_CLASS_WORKER_H
#define HAVE_PTHREADS_CLASS_WORKER_H
PHP_METHOD(Worker, start); 
PHP_METHOD(Worker, shutdown);
PHP_METHOD(Worker, isStarted);
PHP_METHOD(Worker, isShutdown);
PHP_METHOD(Worker, isWorking);
PHP_METHOD(Worker, isTerminated);
PHP_METHOD(Worker, getTerminationInfo);
PHP_METHOD(Worker, stack);
PHP_METHOD(Worker, unstack);
PHP_METHOD(Worker, getStacked);
PHP_METHOD(Worker, getThreadId);
PHP_METHOD(Worker, getCreatorId);
PHP_METHOD(Worker, merge);
PHP_METHOD(Worker, shift);
PHP_METHOD(Worker, pop);
PHP_METHOD(Worker, chunk);

ZEND_BEGIN_ARG_INFO_EX(Worker_start, 0, 0, 0)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_run, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_shutdown, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_getThreadId, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_getCreatorId, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_isStarted, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_isShutdown, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_isWorking, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_getTerminationInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_stack, 0, 0, 1)
	ZEND_ARG_INFO(0, work)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_unstack, 0, 0, 0)
	ZEND_ARG_INFO(0, work)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_getStacked, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_merge, 0, 0, 1)
	ZEND_ARG_INFO(0, from)
	ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_chunk, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_worker_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_WORKER
#	define HAVE_PTHREADS_CLASS_WORKER
zend_function_entry pthreads_worker_methods[] = {
	PHP_ME(Worker, start, Worker_start, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ABSTRACT_ME(Worker, run, Worker_run)
	PHP_ME(Worker, shutdown, Worker_shutdown, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	
	PHP_ME(Worker, getThreadId, Worker_getThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, getCreatorId, Worker_getCreatorId, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	
	PHP_ME(Worker, stack, Worker_stack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, unstack, Worker_unstack, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, getStacked, Worker_getStacked, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	
	PHP_ME(Worker, isShutdown, Worker_isShutdown, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, isStarted, Worker_isStarted, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, isWorking, Worker_isWorking, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, isTerminated, Worker_isTerminated, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, getTerminationInfo, Worker_getTerminationInfo, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	
	PHP_ME(Worker, merge, Worker_merge, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	
	PHP_ME(Worker, shift, Worker_shift, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, pop, Worker_pop, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Worker, chunk, Worker_chunk, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};
/* {{{ proto boolean Worker::start([long $options = PTHREADS_INHERIT_ALL])
		Starts a new Worker Thread, executing the Workers implementation of Worker::run and then waiting for Stackables
		$options should be a mask of inheritance constants */
PHP_METHOD(Worker, start)
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

/* {{{ proto int Worker::stack(Stackable $work)
	Pushes an item onto the Thread Stack, returns the size of stack */
PHP_METHOD(Worker, stack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval 	*work;
	
	if (thread) {
		if (!pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC)) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, pthreads_stackable_entry)==SUCCESS) {
				RETURN_LONG(pthreads_stack_push(thread, work TSRMLS_CC));
			}
		} else zend_error(E_ERROR, "pthreads has detected an attempt to stack onto %s (%lu) which has already been shutdown", PTHREADS_FRIENDLY_NAME);
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while stacking onto %s (%lu) and cannot continue", PTHREADS_FRIENDLY_NAME);
	RETURN_FALSE;
} /* }}} */

/* {{{ proto int Worker::unstack([Stackable $work])
	Removes an item from the Thread stack, if no item is specified the stack is cleared, returns the size of stack */
PHP_METHOD(Worker, unstack)
{
	PTHREAD thread = PTHREADS_FETCH;
	zval * work;
	
	if (thread) {
		if (ZEND_NUM_ARGS() > 0) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &work, pthreads_stackable_entry)==SUCCESS) {
				RETURN_LONG(pthreads_stack_pop(thread, PTHREADS_FETCH_FROM(work) TSRMLS_CC));
			}
		} else RETURN_LONG(pthreads_stack_pop(thread, NULL TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while unstacking from %s (%lu) and cannot continue", PTHREADS_FRIENDLY_NAME);
	RETURN_FALSE;
}

/* {{{ proto int Worker::getStacked()
	Returns the current size of the stack */
PHP_METHOD(Worker, getStacked)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_LONG(pthreads_stack_length(thread TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while getting the stack length of a %s and cannot continue", PTHREADS_NAME);
	RETURN_FALSE;
}

/* {{{ proto Worker::isStarted() 
	Will return true if the Worker has been started */
PHP_METHOD(Worker, isStarted)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_STARTED TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto Worker::isJoined()
	Will return true if the Worker has been shutdown */
PHP_METHOD(Worker, isShutdown)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_JOINED TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Worker::isWorking() 
	Can be used to tell if a Worker is executing any Stackables or waiting */
PHP_METHOD(Worker, isWorking)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(!pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Worker::isTerminated() 
	Will return true if the referenced thread suffered fatal errors or uncaught exceptions */
PHP_METHOD(Worker, isTerminated)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Worker::getTerminationInfo() 
	Will return information concerning the location of the termination to aid debugging */
PHP_METHOD(Worker, getTerminationInfo)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		if (pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC)) {
		    array_init(return_value);
		    
		    if (thread->error->clazz) {
		        add_assoc_string(
		            return_value, "scope", (char *)thread->error->clazz, 1);       
		    }
		    
		    if (thread->error->method) {
		        add_assoc_string(
		            return_value, "function", (char *)thread->error->method, 1);
		    }
		    
		    if (thread->error->file) {
		        add_assoc_string(
		            return_value, "file", (char *)thread->error->file, 1);
		        add_assoc_long(return_value, "line", thread->error->line);
		    }
		} else {
		    RETURN_FALSE;
		}
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Worker::shutdown() 
		Will wait for execution of all Stackables to complete before shutting down the Worker */
PHP_METHOD(Worker, shutdown) 
{ 
	PTHREAD thread = PTHREADS_FETCH;
	
	/*
	* Check that we are in the correct context
	*/
	if (PTHREADS_IN_CREATOR(thread)) {
		RETURN_BOOL((pthreads_join(thread TSRMLS_CC)==SUCCESS));
	} else {
		zend_error(E_WARNING, "pthreads has detected an attempt to shutdown from an incorrect context, only the creating context may shutdown %s (%lu)", PTHREADS_FRIENDLY_NAME);
		RETURN_FALSE;
	}
} /* }}} */

/* {{{ proto long Worker::getThreadId()
	Will return the identifier of the referenced Worker */
PHP_METHOD(Worker, getThreadId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->tid);
} /* }}} */

/* {{{ proto long Worker::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Worker */
PHP_METHOD(Worker, getCreatorId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(getThis()))->cid);
} /* }}} */

/* {{{ proto boolean Worker::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Worker */
PHP_METHOD(Worker, merge) 
{
    zval *from;
    zend_bool overwrite = 1;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &from, &overwrite) != SUCCESS) {
        return;
    }
    
	RETURN_BOOL((pthreads_store_merge(getThis(), from, overwrite TSRMLS_CC)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Worker::shift()
	Will shift the first member from the object */
PHP_METHOD(Worker, shift) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_shift(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Worker::pop()
	Will pop the last member from the object */
PHP_METHOD(Worker, pop) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_pop(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Worker::chunk(integer $size [, boolean $preserve])
	Will shift the first member from the object */
PHP_METHOD(Worker, chunk) 
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

