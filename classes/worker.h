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
#ifndef HAVE_PTHREADS_CLASS_WORKER_H
#define HAVE_PTHREADS_CLASS_WORKER_H
PHP_METHOD(Worker, shutdown);
PHP_METHOD(Worker, isShutdown);
PHP_METHOD(Worker, stack);
PHP_METHOD(Worker, unstack);
PHP_METHOD(Worker, getStacked);
PHP_METHOD(Worker, collect);

ZEND_BEGIN_ARG_INFO_EX(Worker_shutdown, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_isShutdown, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Worker_stack, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, work, Collectable, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_unstack, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_getStacked, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Worker_collect, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, function, Closure, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_worker_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_WORKER
#	define HAVE_PTHREADS_CLASS_WORKER
zend_function_entry pthreads_worker_methods[] = {
	PHP_ME(Worker, shutdown, Worker_shutdown, ZEND_ACC_PUBLIC)
	PHP_ME(Worker, stack, Worker_stack, ZEND_ACC_PUBLIC)
	PHP_ME(Worker, unstack, Worker_unstack, ZEND_ACC_PUBLIC)
	PHP_ME(Worker, getStacked, Worker_getStacked, ZEND_ACC_PUBLIC)
	PHP_ME(Worker, isShutdown, Worker_isShutdown, ZEND_ACC_PUBLIC)
	PHP_ME(Worker, collect, Worker_collect, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto int Worker::stack(Collectable $work)
	Pushes an item onto the stack, returns the size of stack */
PHP_METHOD(Worker, stack)
{
	pthreads_object_t* thread = PTHREADS_FETCH;
	zval *work;

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only the creator of this %s may call stack",
			thread->std.ce->name->val);
		return;
	}

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "O", &work, pthreads_collectable_entry) != SUCCESS) {
		return;
	}

	if (!instanceof_function(Z_OBJCE_P(work), pthreads_threaded_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only Threaded objects may be stacked, %s is not Threaded",
			ZSTR_VAL(Z_OBJCE_P(work)->name));
		return;
	}
	
	RETURN_LONG(pthreads_stack_add(thread->stack, work));	
} /* }}} */

/* {{{ proto Collectable Worker::unstack()
	Removes the first item from the stack */
PHP_METHOD(Worker, unstack)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 
			0, "only the creator of this %s may call unstack", 
			thread->std.ce->name->val);
		return;
	}

	pthreads_stack_del(thread->stack, return_value);
}

/* {{{ proto int Worker::getStacked()
	Returns the current size of the stack */
PHP_METHOD(Worker, getStacked)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_LONG(pthreads_stack_size(thread->stack));
}

/* {{{ proto Worker::isShutdown()
	Will return true if the Worker has been shutdown */
PHP_METHOD(Worker, isShutdown)
{
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_BOOL(pthreads_monitor_check(thread->monitor, PTHREADS_MONITOR_JOINED));
} /* }}} */

/* {{{ proto boolean Worker::shutdown()
		Will wait for execution of all Stackables to complete before shutting down the Worker */
PHP_METHOD(Worker, shutdown) 
{ 
	pthreads_object_t* thread = PTHREADS_FETCH;

	RETURN_BOOL(pthreads_join(thread));
} /* }}} */

/* {{{ proto long Worker::getThreadId()
	Will return the identifier of the referenced Worker */
PHP_METHOD(Worker, getThreadId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(Z_OBJ_P(getThis())))->local.id);
} /* }}} */

/* {{{ proto long Worker::getCreatorId() 
	Will return the identifier of the thread ( or process ) that created the referenced Worker */
PHP_METHOD(Worker, getCreatorId)
{
	ZVAL_LONG(return_value, (PTHREADS_FETCH_FROM(Z_OBJ_P(getThis())))->creator.id);
} /* }}} */

/* {{{ */
static zend_bool pthreads_worker_collect_function(pthreads_call_t *call, zval *collectable) {
	zval result;
	zend_bool remove = 0;

	ZVAL_UNDEF(&result);

	call->fci.retval = &result;
	call->fci.no_separation = 1;

	zend_fcall_info_argn(&call->fci, 1, collectable);

	if (zend_call_function(&call->fci, &call->fcc) != SUCCESS) {
		return remove;
	}

	zend_fcall_info_args_clear(&call->fci, 1);

	if (Z_TYPE(result) != IS_UNDEF) {
		if (zend_is_true(&result)) {
			remove = 1;
		}
		zval_ptr_dtor(&result);
	}

	return remove;
} /* }}} */

/* {{{ proto int Worker::collect(callable function) */
PHP_METHOD(Worker, collect)
{
	pthreads_object_t *thread = PTHREADS_FETCH;
	pthreads_call_t call = PTHREADS_CALL_EMPTY;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "f", &call.fci, &call.fcc) != SUCCESS) {
		return;
	}

	if (!PTHREADS_IN_CREATOR(thread) || PTHREADS_IS_CONNECTION(thread)) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,	
			"only the creator of this %s may call collect",
			thread->std.ce->name->val);
		return;
	}

	RETURN_LONG(pthreads_stack_collect(thread->stack, &call, pthreads_worker_collect_function));
}
#	endif
#endif

