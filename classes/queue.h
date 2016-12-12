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
#ifndef HAVE_PTHREADS_CLASS_QUEUE_H
#define HAVE_PTHREADS_CLASS_QUEUE_H
PHP_METHOD(Queue, push);

PHP_METHOD(Queue, pop);
PHP_METHOD(Queue, shift);

PHP_METHOD(Queue, size);

ZEND_BEGIN_ARG_INFO_EX(Queue_push, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Queue_pop, 0, 0, 0)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(Queue_shift, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Queue_size, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_queue_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_QUEUE
#	define HAVE_PTHREADS_CLASS_QUEUE
zend_function_entry pthreads_queue_methods[] = {
	PHP_ME(Queue, push, Queue_push, ZEND_ACC_PUBLIC)
	PHP_ME(Queue, pop, Queue_pop, ZEND_ACC_PUBLIC)
	PHP_ME(Queue, shift, Queue_shift, ZEND_ACC_PUBLIC)
	PHP_ME(Queue, size, Queue_size, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};


/* {{{ proto void Threaded::merge(mixed $data, [boolean $overwrite = true])
	Push data to the queue */
PHP_METHOD(Queue, push)
{
	zval *data;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &data) == FAILURE) {
		return;
	}
    
	pthreads_queue_push(getThis(), data);
} /* }}} */

/* {{{ proto mixed Queue::pop()
	Will pop the last member from the queue */
PHP_METHOD(Queue, pop)
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_queue_pop(getThis(), return_value);
} /* }}} */

/* {{{ proto mixed Queue::shift()
	Will shift the first member from the queue */
PHP_METHOD(Queue, shift)
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
    
    pthreads_queue_shift(getThis(), return_value);
} /* }}} */

/* {{{ proto boolean Queue::size()
	Will return the size of the queue */
PHP_METHOD(Queue, size)
{
    if (zend_parse_parameters_none() != SUCCESS) {
        return;
    }
	
	ZVAL_LONG(return_value, 0);
	
	pthreads_queue_size(getThis(), &Z_LVAL_P(return_value));
} /* }}} */

#	endif
#endif
