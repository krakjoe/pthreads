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
#ifndef HAVE_PTHREADS_CLASS_THREAD_H
#define HAVE_PTHREADS_CLASS_THREAD_H
PHP_METHOD(Thread, start);
PHP_METHOD(Thread, join);
PHP_METHOD(Thread, isStarted);
PHP_METHOD(Thread, isJoined);
PHP_METHOD(Thread, getThreadId);
PHP_METHOD(Thread, getCurrentThreadId);
PHP_METHOD(Thread, getCurrentThread);
PHP_METHOD(Thread, getCreatorId);
PHP_METHOD(Thread, kill);
PHP_METHOD(Thread, __destruct);

ZEND_BEGIN_ARG_INFO_EX(Thread_start, 0, 0, 0)
    ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_run, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Thread_join, 0, 0, 0)
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

ZEND_BEGIN_ARG_INFO_EX(Thread_destruct, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_thread_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREAD
#	define HAVE_PTHREADS_CLASS_THREAD
zend_function_entry pthreads_thread_methods[] = {
	PHP_ME(Thread, start, Thread_start, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, join, Thread_join, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, isStarted, Thread_isStarted, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, isJoined, Thread_isJoined, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getThreadId, Thread_getThreadId, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getCreatorId, Thread_getThreadId, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, getCurrentThreadId, Thread_getCurrentThreadId, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Thread, getCurrentThread, Thread_getCurrentThread, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Thread, kill, Thread_kill, ZEND_ACC_PUBLIC)
	PHP_ME(Thread, __destruct, Thread_destruct, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto boolean Thread::start([long $options = PTHREADS_INHERIT_ALL])
		Starts executing the implementations run method in a thread, will return a boolean indication of success
		$options should be a mask of inheritance constants */
PHP_METHOD(Thread, start)
{
	pthreads_object_t* thread = PTHREADS_FETCH;
	zend_long options = PTHREADS_INHERIT_ALL;
	
	if (ZEND_NUM_ARGS()) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &options) != SUCCESS) {
			return;
		}

		thread->options = options;
	}

	RETURN_BOOL(pthreads_start(thread));
} /* }}} */

/* {{{ proto Thread::isStarted() 
	Will return true if a Thread has been started */
PHP_METHOD(Thread, isStarted)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_STARTED));
} /* }}} */

/* {{{ proto Thread::isJoined()
	Will return true if a Thread has been joined already */
PHP_METHOD(Thread, isJoined)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED));
} /* }}} */

/* {{{ proto boolean Thread::join() 
		Will return a boolean indication of success */
PHP_METHOD(Thread, join) 
{ 
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_BOOL(pthreads_join(thread));
} /* }}} */

/* {{{ proto long Thread::getThreadId()
	Will return the identifier of the referenced Thread */
PHP_METHOD(Thread, getThreadId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(Z_OBJ_P(getThis())))->local.id);
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
	pthreads_current_thread(return_value);
} /* }}} */

/* {{{ proto long Thread::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Thread */
PHP_METHOD(Thread, getCreatorId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(Z_OBJ_P(getThis())))->creator.id);
} /* }}} */

/* {{{ proto boolean Thread::kill()
	Will kill the referenced thread, forcefully */
PHP_METHOD(Thread, kill) 
{
	pthreads_object_t* thread = PTHREADS_FETCH;

    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
	
	RETURN_BOOL(pthread_kill(thread->thread, PTHREADS_KILL_SIGNAL)==SUCCESS);
} /* }}} */

/* {{{ */
PHP_METHOD(Thread, __destruct)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	if (PTHREADS_IN_CREATOR(thread) && !PTHREADS_IS_CONNECTION(thread)) {
		if (!pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED)) {
			pthreads_join(thread);
		}
	}
} /* }}} */
#	endif
#endif
