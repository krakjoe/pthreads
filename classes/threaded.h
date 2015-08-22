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
#ifndef HAVE_PTHREADS_CLASS_THREADED_H
#define HAVE_PTHREADS_CLASS_THREADED_H
PHP_METHOD(Threaded, run);
PHP_METHOD(Threaded, wait);
PHP_METHOD(Threaded, notify);
PHP_METHOD(Threaded, isRunning);
PHP_METHOD(Threaded, isTerminated);

PHP_METHOD(Threaded, synchronized);
PHP_METHOD(Threaded, merge);
PHP_METHOD(Threaded, shift);
PHP_METHOD(Threaded, chunk);
PHP_METHOD(Threaded, pop);
PHP_METHOD(Threaded, count);
PHP_METHOD(Threaded, extend);

PHP_METHOD(Threaded, addRef);
PHP_METHOD(Threaded, delRef);
PHP_METHOD(Threaded, getRefCount);

ZEND_BEGIN_ARG_INFO_EX(Threaded_run, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_wait, 0, 0, 0)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_LONG, 1)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_synchronized, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_merge, 0, 0, 1)
    ZEND_ARG_INFO(0, from)
    ZEND_ARG_INFO(0, overwrite)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_chunk, 0, 0, 1)
    ZEND_ARG_INFO(0, size)
    ZEND_ARG_INFO(0, preserve)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_pop, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_count, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_extend, 0, 0, 1)
    ZEND_ARG_INFO(0, class)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_addRef, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_delRef, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_getRefCount, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_threaded_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_THREADED
#	define HAVE_PTHREADS_CLASS_THREADED
zend_function_entry pthreads_threaded_methods[] = {
	PHP_ME(Threaded, run, Threaded_run, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, wait, Threaded_wait, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, notify, Threaded_notify, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, isRunning, Threaded_isRunning, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, isTerminated, Threaded_isTerminated, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, synchronized, Threaded_synchronized, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, merge, Threaded_merge, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, shift, Threaded_shift, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, chunk, Threaded_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, pop, Threaded_pop, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, count, Threaded_count, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, addRef, Threaded_addRef, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, delRef, Threaded_delRef, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, getRefCount, Threaded_getRefCount, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, extend, Threaded_extend, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ */
PHP_METHOD(Threaded, run) {} /* }}} */

/* {{{{ */
PHP_METHOD(Threaded, addRef) 		{ Z_ADDREF_P(getThis()); }
PHP_METHOD(Threaded, delRef) 		{ zval_ptr_dtor(getThis()); }
PHP_METHOD(Threaded, getRefCount) 	{ RETURN_LONG(Z_REFCOUNT_P(getThis())); } /* }}} */

/* {{{ proto boolean Threaded::wait([long timeout]) 
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(Threaded, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	zend_long timeout = 0L;
	
	if (thread) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &timeout)==SUCCESS) {
			RETURN_BOOL(pthreads_monitor_wait(thread->monitor, timeout) == SUCCESS);
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while preparing to wait for a %s", PTHREADS_NAME);
	}
} /* }}} */

/* {{{ proto boolean Threaded::notify()
		Send notification to everyone waiting on the Threaded
		Will return a boolean indication of success */
PHP_METHOD(Threaded, notify)
{
	PTHREAD thread = PTHREADS_FETCH;

	if (thread) {
		RETURN_BOOL(pthreads_monitor_notify(thread->monitor));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while preparing to notify a %s", PTHREADS_NAME);
	}
} /* }}} */

/* {{{ proto boolean Threaded::isRunning() 
	Will return true while the referenced Threaded is being executed by a Worker */
PHP_METHOD(Threaded, isRunning)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_RUNNING));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);		
	}
} /* }}} */

/* {{{ proto boolean Threaded::isTerminated() 
	Will return true if the referenced Threaded suffered fatal errors or uncaught exceptions */
PHP_METHOD(Threaded, isTerminated)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_ERROR));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);	
	}
} /* }}} */

/* {{{ proto void Threaded::synchronized(Callable function, ...)
	Will synchronize the object, call the function, passing anything after the function as parameters
	 */
PHP_METHOD(Threaded, synchronized) 
{
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	uint argc = 0;
	zval *argv = NULL;
	PTHREAD thread = PTHREADS_FETCH;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "f|+", &call.fci, &call.fcc, &argv, &argc) != SUCCESS) {
		return;
	}

	zend_fcall_info_argp(&call.fci, argc, argv);

	call.fci.retval = return_value;
	call.fci.no_separation = 1;
	
	call.fci.object = &thread->std;
	call.fcc.object = &thread->std;

	if (pthreads_monitor_lock(thread->monitor)) {
		zend_try {
			/* call the closure */
			zend_call_function(&call.fci, &call.fcc);
		} zend_catch {
			ZVAL_UNDEF(return_value);
		} zend_end_try ();
		
		pthreads_monitor_unlock(thread->monitor);
	}
	
	zend_fcall_info_args_clear(&call.fci, 1);
} /* }}} */

/* {{{ proto boolean Threaded::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Threaded */
PHP_METHOD(Threaded, merge) 
{
    zval *from;
    zend_bool overwrite = 1;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &from, &overwrite) != SUCCESS) {
        return;
    }
    
	RETURN_BOOL((pthreads_store_merge(getThis(), from, overwrite)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Threaded::shift()
	Will shift the first member from the object */
PHP_METHOD(Threaded, shift) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_shift(getThis(), return_value);
} /* }}} */

/* {{{ proto mixed Threaded::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
PHP_METHOD(Threaded, chunk) 
{
    zend_long size;
    zend_bool preserve = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|b", &size, &preserve) != SUCCESS) {
        return;
    }
    
    pthreads_store_chunk(getThis(), size, preserve, return_value);
} /* }}} */

/* {{{ proto mixed Threaded::pop()
	Will pop the last member from the object */
PHP_METHOD(Threaded, pop) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_pop(getThis(), return_value);
} /* }}} */

/* {{{ proto boolean Threaded::count()
	Will return the size of the properties table */
PHP_METHOD(Threaded, count)
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
	
	ZVAL_LONG(return_value, 0);
	
	pthreads_store_count(
		getThis(), &Z_LVAL_P(return_value));
} /* }}} */

/* {{{ proto bool Threaded::extend(string class) */
PHP_METHOD(Threaded, extend) {
    zend_class_entry *ce = NULL;
    zend_bool is_final = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS(), "C", &ce) != SUCCESS) {
        return;
    }

#ifdef ZEND_ACC_TRAIT
    if ((ce->ce_flags & ZEND_ACC_TRAIT) == ZEND_ACC_TRAIT) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
            "cannot extend trait %s", ce->name);
        return;
    }
#endif

    if (ce->ce_flags & ZEND_ACC_INTERFACE) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
            "cannot extend interface %s", 
            ce->name);
        return;
    }
    
    if (ce->parent) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0, 
            "cannot extend class %s, it already extends %s", 
            ce->name,
            ce->parent->name);
        return;
    }
    
    is_final = ce->ce_flags & ZEND_ACC_FINAL;

    if (is_final)
        ce->ce_flags = ce->ce_flags &~ ZEND_ACC_FINAL;

    zend_do_inheritance(ce, EX(called_scope));

    if (is_final)
        ce->ce_flags |= ZEND_ACC_FINAL;

    RETURN_BOOL(instanceof_function(ce, EX(called_scope)));
} /* }}} */
#	endif
#endif
