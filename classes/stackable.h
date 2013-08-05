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
#ifndef HAVE_PTHREADS_CLASS_STACKABLE_H
#define HAVE_PTHREADS_CLASS_STACKABLE_H
PHP_METHOD(Stackable, wait);
PHP_METHOD(Stackable, notify);
PHP_METHOD(Stackable, isRunning);
PHP_METHOD(Stackable, isWaiting);
PHP_METHOD(Stackable, isTerminated);
PHP_METHOD(Stackable, synchronized);
PHP_METHOD(Stackable, lock);
PHP_METHOD(Stackable, unlock);
PHP_METHOD(Stackable, merge);
PHP_METHOD(Stackable, shift);
PHP_METHOD(Stackable, chunk);
PHP_METHOD(Stackable, pop);

ZEND_BEGIN_ARG_INFO_EX(Stackable_run, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Stackable_wait, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Stackable_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Stackable_isWaiting, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Stackable_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_synchronized, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_merge, 0, 0, 1)
    ZEND_ARG_INFO(0, from)
    ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_chunk, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Stackable_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_stackable_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_STACKABLE
#	define HAVE_PTHREADS_CLASS_STACKABLE
zend_function_entry pthreads_stackable_methods[] = {
	PHP_ABSTRACT_ME(Stackable, run, Stackable_run)
	PHP_ME(Stackable, wait, Stackable_wait, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, notify, Stackable_notify, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, isRunning, Stackable_isRunning, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, isWaiting, Stackable_isWaiting, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, isTerminated, Stackable_isTerminated, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, synchronized, Stackable_synchronized, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, lock, Stackable_lock, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, unlock, Stackable_unlock, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, merge, Stackable_merge, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, shift, Stackable_shift, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, chunk, Stackable_chunk, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(Stackable, pop, Stackable_pop, ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};
/* {{{ proto boolean Stackable::wait([long timeout]) 
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(Stackable, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	long timeout = 0L;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &timeout)==SUCCESS) {
		if (ZEND_NUM_ARGS()) {
			RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, timeout TSRMLS_CC));
		} else RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, 0L TSRMLS_CC));
	}
} /* }}} */

/* {{{ proto boolean Stackable::notify()
		Send notification to everyone waiting on the Stackable
		Will return a boolean indication of success */
PHP_METHOD(Stackable, notify)
{
	PTHREAD thread = PTHREADS_FETCH;
	if (thread) {
		RETURN_BOOL(pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC));
	}else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to notify a %s and cannot continue", PTHREADS_NAME);
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Stackable::isRunning() 
	Will return true while the referenced Stackable is being executed by a Worker */
PHP_METHOD(Stackable, isRunning)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Stackable::isWaiting() 
	Will return true if the referenced Stackable is waiting for notification */
PHP_METHOD(Stackable, isWaiting)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto boolean Stackable::isTerminated() 
	Will return true if the referenced Stackable suffered fatal errors or uncaught exceptions */
PHP_METHOD(Stackable, isTerminated)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC));
	} else zend_error(E_ERROR, "pthreads has experienced an internal error while preparing to read the state of a %s and cannot continue", PTHREADS_NAME);
} /* }}} */

/* {{{ proto void Stackable::synchronized(Callable function, ...)
	Will synchronize the object, call the function, passing anything after the function as parameters
	 */
PHP_METHOD(Stackable, synchronized) 
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

/* {{{ proto boolean Stackable::lock()
	Will acquire the storage lock */
PHP_METHOD(Stackable, lock) 
{
	ZVAL_BOOL(return_value, pthreads_store_lock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Stackable::unlock()
	Will release the storage lock */
PHP_METHOD(Stackable, unlock) 
{
	ZVAL_BOOL(return_value, pthreads_store_unlock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Stackable::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Stackable */
PHP_METHOD(Stackable, merge) 
{
    zval *from;
    zend_bool overwrite = 1;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &from, &overwrite) != SUCCESS) {
        return;
    }
    
	RETURN_BOOL((pthreads_store_merge(getThis(), from, overwrite TSRMLS_CC)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Stackable::shift()
	Will shift the first member from the object */
PHP_METHOD(Stackable, shift) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_shift(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Stackable::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
PHP_METHOD(Stackable, chunk) 
{
    long size;
    zend_bool preserve = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &size, &preserve) != SUCCESS) {
        return;
    }
    
    pthreads_store_chunk(getThis(), size, preserve, &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Stackable::pop()
	Will pop the last member from the object */
PHP_METHOD(Stackable, pop) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_pop(getThis(), &return_value TSRMLS_CC);
} /* }}} */
#	endif
#endif
