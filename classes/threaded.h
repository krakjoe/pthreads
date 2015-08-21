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
PHP_METHOD(Threaded, isWaiting);
PHP_METHOD(Threaded, isTerminated);

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

PHP_METHOD(Threaded, addRef);
PHP_METHOD(Threaded, delRef);
PHP_METHOD(Threaded, getRefCount);

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
    ZEND_ARG_OBJ_INFO(0, run, Closure, 0)
    ZEND_ARG_OBJ_INFO(0, construct, Closure, 1)
    ZEND_ARG_INFO(0, args)
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
	PHP_ME(Threaded, isWaiting, Threaded_isWaiting, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, isTerminated, Threaded_isTerminated, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, synchronized, Threaded_synchronized, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, lock, Threaded_lock, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, unlock, Threaded_unlock, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, merge, Threaded_merge, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, shift, Threaded_shift, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, chunk, Threaded_chunk, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, pop, Threaded_pop, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, count, Threaded_count, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, addRef, Threaded_addRef, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, delRef, Threaded_delRef, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, getRefCount, Threaded_getRefCount, ZEND_ACC_PUBLIC)
	PHP_ME(Threaded, extend, Threaded_extend, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(Threaded, from, Threaded_from, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
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
			if (ZEND_NUM_ARGS()) {
				RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, timeout));
			} else RETURN_BOOL(pthreads_set_state_ex(thread, PTHREADS_ST_WAITING, 0L));
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
		RETURN_BOOL(pthreads_unset_state(thread, PTHREADS_ST_WAITING));
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
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_RUNNING));
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while preparing to read the state of a %s", PTHREADS_NAME);		
	}
} /* }}} */

/* {{{ proto boolean Threaded::isWaiting() 
	Will return true if the referenced Threaded is waiting for notification */
PHP_METHOD(Threaded, isWaiting)
{
	PTHREAD thread = PTHREADS_FETCH;
	
	if (thread) {
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_WAITING));
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
		RETURN_BOOL(pthreads_state_isset(thread->state, PTHREADS_ST_ERROR));
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
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	uint argc = 0;
	zval *argv = NULL;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "f|+", &fci, &fcc, &argv, &argc) == SUCCESS) {
		pthreads_synchro_block(getThis(), &fci, &fcc, argc, argv, return_value);
	}
} /* }}} */

/* {{{ proto boolean Threaded::lock()
	Will acquire the storage lock */
PHP_METHOD(Threaded, lock) 
{
	ZVAL_BOOL(return_value, pthreads_store_lock(getThis()));
} /* }}} */

/* {{{ proto boolean Threaded::unlock()
	Will release the storage lock */
PHP_METHOD(Threaded, unlock) 
{
	ZVAL_BOOL(return_value, pthreads_store_unlock(getThis()));
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

/* {{{ proto Threaded Threaded::from(Closure closure [, Closure ctor [, array args = []]]) */
PHP_METHOD(Threaded, from)
{
    zval *zrun, *zconstruct = NULL, *zargs = NULL;
    zend_function *run, *constructor;
    zend_class_entry zce, *pce;
    PTHREAD threaded;
    char *named;
    size_t nlen;
	
    TSRMLS_CACHE_UPDATE();

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "O|Oa", &zrun, zend_ce_closure, &zconstruct, zend_ce_closure, &zargs) != SUCCESS) {
        return;
    }
    
    run = (zend_function*) 
	zend_get_closure_method_def(zrun);
    run = pthreads_copy_function(run);

    if (run->common.num_args > 0) {
        zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error, %s::run must not have arguments", EX(called_scope)->name->val);
	    return;
    }
    
    zce.type = ZEND_USER_CLASS;
    zend_initialize_class_data(&zce, 1);
    zce.refcount = 1;
    nlen = spprintf
        ((char**)&named, 0, "%sClosure@%p", 
	EX(called_scope)->name->val, ((zend_op_array*) run)->opcodes);
    zce.name = zend_string_init(named, nlen, 0);
    zce.info.user.filename = NULL;

    efree(named);

    if (zconstruct) {
        constructor = (zend_function*) 
		zend_get_closure_method_def(zconstruct);
	constructor = pthreads_copy_function(constructor);
	
        if (!zend_hash_str_update_ptr(&zce.function_table, "__construct", sizeof("__construct")-1, constructor)) {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0, 
			"pthreads has experienced an internal error while injecting the constructor function for %s", zce.name->val);
		zend_string_release(zce.name);
		return;
        }

        zce.constructor = constructor;
    }
    
    if (!zend_hash_str_update_ptr(&zce.function_table, "run", sizeof("run")-1, run)) {
	zend_throw_exception_ex(
		spl_ce_RuntimeException, 0, 
		"pthreads has experienced an internal error while injecting the run function for %s", zce.name->val);
	if (zconstruct) {
		destroy_op_array((zend_op_array*) constructor);
	}
	destroy_op_array((zend_op_array*) run);
	zend_string_release(zce.name);
	return;
    }

    if (!(pce = zend_hash_add_mem(EG(class_table), zce.name, &zce, sizeof(zend_class_entry)))) {
        zend_throw_exception_ex(
            spl_ce_RuntimeException, 0, 
            "pthreads has experienced an internal error while registering the class entry for %s", zce.name->val);
        zend_string_release(zce.name);
	if (zconstruct) {
		destroy_op_array((zend_op_array*) constructor);
	}
	destroy_op_array((zend_op_array*)run);
        return;
    }

    zend_do_inheritance(pce, EX(called_scope));
    pce->ce_flags |= ZEND_ACC_FINAL;
    
    object_init_ex(return_value, pce);
    
    if (zconstruct) {
	zend_fcall_info fci = empty_fcall_info;
	zend_fcall_info_cache fcc = empty_fcall_info_cache;
	zend_class_entry *scope = EG(scope);
	zval retval;

	ZVAL_UNDEF(&retval);
	EG(scope) = pce;
	
	fci.size = sizeof(zend_fcall_info);
	fci.function_table = EG(function_table);
	fci.object = Z_OBJ_P(return_value);
	fci.retval = &retval;
	fci.no_separation = 1;
	
	fcc.initialized = 1;
	fcc.function_handler = constructor;
	fcc.calling_scope = EG(scope);
	fcc.called_scope = Z_OBJCE_P(return_value);
	fcc.object = Z_OBJ_P(return_value);
	
	if (zargs)
		zend_fcall_info_args(&fci, zargs);
	
	if (zend_call_function(&fci, &fcc) != SUCCESS) {
		return;
	}
	
	if (zargs)
		zend_fcall_info_args_clear(&fci, 1);
	
	if (Z_TYPE(retval) != IS_UNDEF)
		zval_dtor(&retval);
	
	EG(scope) = scope;
    }
} /* }}} */
#	endif
#endif
