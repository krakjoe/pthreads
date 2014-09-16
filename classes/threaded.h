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
#ifndef HAVE_PTHREADS_CLASS_THREADED_H
#define HAVE_PTHREADS_CLASS_THREADED_H
PHP_METHOD(Threaded, run);
PHP_METHOD(Threaded, wait);
PHP_METHOD(Threaded, notify);
PHP_METHOD(Threaded, isRunning);
PHP_METHOD(Threaded, isWaiting);
PHP_METHOD(Threaded, isTerminated);
PHP_METHOD(Threaded, getTerminationInfo);
PHP_METHOD(Threaded, synchronized);
PHP_METHOD(Threaded, lock);
PHP_METHOD(Threaded, unlock);
PHP_METHOD(Threaded, merge);
PHP_METHOD(Threaded, shift);
PHP_METHOD(Threaded, chunk);
PHP_METHOD(Threaded, pop);
PHP_METHOD(Threaded, count);
PHP_METHOD(Threaded, extend);
PHP_METHOD(Threaded, from);

ZEND_BEGIN_ARG_INFO_EX(Threaded_run, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_wait, 0, 0, 0)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_notify, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_isRunning, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_isWaiting, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_isTerminated, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Threaded_getTerminationInfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_synchronized, 0, 0, 1)
	ZEND_ARG_INFO(0, function)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Threaded_unlock, 0, 0, 0)
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

ZEND_BEGIN_ARG_INFO_EX(Threaded_from, 0, 0, 1)
    ZEND_ARG_INFO(0, closure)
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
	PHP_ME(Threaded, isWaiting, Threaded_isWaiting, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, isTerminated, Threaded_isTerminated, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, getTerminationInfo, Threaded_getTerminationInfo, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, synchronized, Threaded_synchronized, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, lock, Threaded_lock, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, unlock, Threaded_unlock, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, merge, Threaded_merge, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, shift, Threaded_shift, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, chunk, Threaded_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, pop, Threaded_pop, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, count, Threaded_count, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, extend, Threaded_extend, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Threaded, from, Threaded_from, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};

/* {{{ */
PHP_METHOD(Threaded, run) {} /* }}} */

/* {{{ proto boolean Threaded::wait([long timeout]) 
		Will cause the calling thread to wait for notification from the referenced object
		When a timeout is used and reached boolean false will return
		Otherwise returns a boolean indication of success */
PHP_METHOD(Threaded, wait)
{
	PTHREAD thread = PTHREADS_FETCH;
	long timeout = 0L;
	
	if (thread) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &timeout)==SUCCESS) {
			if (ZEND_NUM_ARGS()) {
				RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, timeout TSRMLS_CC));
			} else RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, 0L TSRMLS_CC));
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
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
		RETURN_BOOL(pthreads_unset_state(thread, PTHREADS_ST_WAITING TSRMLS_CC));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to notify a %s", PTHREADS_NAME);
	}
} /* }}} */

/* {{{ proto boolean Threaded::isRunning() 
	Will return true while the referenced Threaded is being executed by a Worker */
PHP_METHOD(Threaded, isRunning)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING TSRMLS_CC));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);		
	}
} /* }}} */

/* {{{ proto boolean Threaded::isWaiting() 
	Will return true if the referenced Threaded is waiting for notification */
PHP_METHOD(Threaded, isWaiting)
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

/* {{{ proto boolean Threaded::isTerminated() 
	Will return true if the referenced Threaded suffered fatal errors or uncaught exceptions */
PHP_METHOD(Threaded, isTerminated)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_ERROR TSRMLS_CC));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);	
	}
} /* }}} */

/* {{{ proto boolean Threaded::getTerminationInfo() 
	Will return information concerning the location of the termination to aid debugging */
PHP_METHOD(Threaded, getTerminationInfo)
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
		    
		    if (thread->error->message) {
		        add_assoc_string(
		            return_value, "message", (char *)thread->error->message, 1);
		    }
		} else {
		    RETURN_FALSE;
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);	
	}
} /* }}} */

/* {{{ proto void Threaded::synchronized(Callable function, ...)
	Will synchronize the object, call the function, passing anything after the function as parameters
	 */
PHP_METHOD(Threaded, synchronized) 
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

/* {{{ proto boolean Threaded::lock()
	Will acquire the storage lock */
PHP_METHOD(Threaded, lock) 
{
	ZVAL_BOOL(return_value, pthreads_store_lock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Threaded::unlock()
	Will release the storage lock */
PHP_METHOD(Threaded, unlock) 
{
	ZVAL_BOOL(return_value, pthreads_store_unlock(getThis() TSRMLS_CC));
} /* }}} */

/* {{{ proto boolean Threaded::merge(mixed $data, [boolean $overwrite = true])
	Will merge data with the referenced Threaded */
PHP_METHOD(Threaded, merge) 
{
    zval *from;
    zend_bool overwrite = 1;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &from, &overwrite) != SUCCESS) {
        return;
    }
    
	RETURN_BOOL((pthreads_store_merge(getThis(), from, overwrite TSRMLS_CC)==SUCCESS));
} /* }}} */

/* {{{ proto mixed Threaded::shift()
	Will shift the first member from the object */
PHP_METHOD(Threaded, shift) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_shift(getThis(), &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Threaded::chunk(integer $size [, boolean $preserve = false])
	Will shift the first member from the object */
PHP_METHOD(Threaded, chunk) 
{
    long size;
    zend_bool preserve = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &size, &preserve) != SUCCESS) {
        return;
    }
    
    pthreads_store_chunk(getThis(), size, preserve, &return_value TSRMLS_CC);
} /* }}} */

/* {{{ proto mixed Threaded::pop()
	Will pop the last member from the object */
PHP_METHOD(Threaded, pop) 
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_store_pop(getThis(), &return_value TSRMLS_CC);
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
		getThis(), &Z_LVAL_P(return_value) TSRMLS_CC);
} /* }}} */

/* {{{ proto bool Threaded::extend(string class) */
PHP_METHOD(Threaded, extend) {
    zend_class_entry *ce = NULL;
    zend_bool is_final = 0;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "C", &ce) != SUCCESS) {
        return;
    }

#ifdef ZEND_ACC_TRAIT
    if (ce->ce_flags & ZEND_ACC_TRAIT) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, 
            "cannot extend trait %s", ce->name);
        return;
    }
#endif

    if (ce->ce_flags & ZEND_ACC_INTERFACE) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, 
            "cannot extend interface %s", 
            ce->name);
        return;
    }
    
    if (ce->parent) {
        zend_throw_exception_ex(spl_ce_RuntimeException, 0 TSRMLS_CC, 
            "cannot extend class %s, it already extends %s", 
            ce->name,
            ce->parent->name);
        return;
    }
    
    is_final = ce->ce_flags & ZEND_ACC_FINAL;

    if (is_final)
        ce->ce_flags = ce->ce_flags &~ ZEND_ACC_FINAL;

    zend_do_inheritance(ce, EG(called_scope) TSRMLS_CC);

    if (is_final)
        ce->ce_flags |= ZEND_ACC_FINAL;

    RETURN_BOOL(instanceof_function(ce, EG(called_scope) TSRMLS_CC));
} /* }}} */

/* {{{ proto Threaded Threaded::from(Closure closure) */
PHP_METHOD(Threaded, from)
{
    zval *zclosure;
    const zend_function *run;
    zend_function *runnable;
    zend_class_entry *zce;
    PTHREAD threaded;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zclosure, zend_ce_closure) != SUCCESS) {
        return;
    }
    
    run = zend_get_closure_method_def(zclosure TSRMLS_CC);
    
    zce = (zend_class_entry*) ecalloc(1, sizeof(zend_class_entry));
    zce->name_length = spprintf
        ((char**)&zce->name, 0, "%sClosure@%p", EG(called_scope)->name, ((zend_op_array*) run)->opcodes);
    zce->type = ZEND_USER_CLASS;
    
    zend_initialize_class_data(zce, 1 TSRMLS_CC);
    zce->refcount = 1;
    
    if (zend_hash_update(&zce->function_table, "run", sizeof("run"), (void**)run, sizeof(zend_function), (void**)&runnable) != SUCCESS) {
        zend_throw_exception_ex(
			spl_ce_RuntimeException, 0 TSRMLS_CC, 
			"pthreads has experienced an internal error while injecting the run function for %s", zce->name);
	    efree((char*)zce->name);
	    efree(zce);
	    return;
    } else function_add_ref(runnable);
    
    if (zend_hash_update(EG(class_table), zce->name, zce->name_length, (void**)&zce, sizeof(zend_class_entry*), NULL) != SUCCESS) {
        zend_throw_exception_ex(
            spl_ce_RuntimeException, 0 TSRMLS_CC, 
            "pthreads has experienced an internal error while registering the class entry for %s", zce->name);
        efree((char*)zce->name);
        efree(zce);
        return;
    }

    zend_do_inheritance(zce, EG(called_scope) TSRMLS_CC);
    zce->ce_flags |= ZEND_ACC_FINAL;
    
    object_init_ex(return_value, zce);
} /* }}} */
#	endif
#endif
