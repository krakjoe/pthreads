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
#ifndef HAVE_PTHREADS_CLASS_COLLECTABLE_H
#define HAVE_PTHREADS_CLASS_COLLECTABLE_H
PHP_METHOD(Collectable, isGarbage);
PHP_METHOD(Collectable, setGarbage);

ZEND_BEGIN_ARG_INFO_EX(Collectable_isGarbage, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Collectable_setGarbage, 0, 0, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_collectable_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_COLLECTABLE
#	define HAVE_PTHREADS_CLASS_COLLECTABLE
zend_function_entry pthreads_collectable_methods[] = {
	PHP_ME(Collectable, isGarbage, 	Collectable_isGarbage, 	ZEND_ACC_PUBLIC)
	PHP_ME(Collectable, setGarbage, Collectable_setGarbage, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto bool Collectable::isGarbage(void)
	Can be called in Pool::collect to determine if this object is garbage */
PHP_METHOD(Collectable, isGarbage) {
	zval *garbage = NULL;
	PTHREAD pobject = PTHREADS_FETCH;
		
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	garbage = zend_read_property(pobject->std.ce, getThis(), ZEND_STRL("garbage"), 1 TSRMLS_CC);
	
	if (!garbage) {
		RETURN_FALSE;
	} else {
#if PHP_VERSION_ID >= 50600
		if (zend_is_true(garbage TSRMLS_CC)) {
#else
		if (zend_is_true(garbage)) {
#endif
			RETVAL_BOOL(1);
		} else RETVAL_BOOL(0);
	}
	
	zval_ptr_dtor(&garbage);
	FREE_ZVAL(garbage);
} /* }}} */

/* {{{ proto bool Collectable::setGarbage(void)
	Should be called once per object when the object is finished being executed or referenced */
PHP_METHOD(Collectable, setGarbage) {
	PTHREAD pobject = PTHREADS_FETCH;
	
	if (zend_parse_parameters_none() != SUCCESS) {
		return;
	}
	
	zend_update_property_bool(pobject->std.ce, getThis(), ZEND_STRL("garbage"), 1 TSRMLS_CC);
} /* }}} */
#	endif
#endif
