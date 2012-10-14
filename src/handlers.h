/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                                		 |
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

/*
* @TODO
*	has_property should reference the threading context as it saves allocation and will cause isset to work as expected
*/

/*
* These handlers are mutex aware for safer operation in multi-threaded applications
* They also have the ability to reference the threading context and import variables into the current context
*/
#ifndef HAVE_PTHREADS_HANDLERS_H
#define HAVE_PTHREADS_HANDLERS_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_SERIAL_H
#	include <src/serial.h>
#endif

extern zend_object_handlers * zsh;

/* {{{ these resolve differences in 5.3 and 5.4 object handling API */
#if PHP_VERSION_ID > 50399
#	define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type, key TSRMLS_CC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value, key TSRMLS_CC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists, key TSRMLS_CC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member, key TSRMLS_CC
#	define PTHREADS_GET_METHOD_PASSTHRU_D zval **pobject, char *method, int methodl, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_GET_METHOD_PASSTHRU_C pobject, method, methodl, key TSRMLS_CC
#	define PTHREADS_CALL_METHOD_PASSTHRU_D const char *method, INTERNAL_FUNCTION_PARAMETERS
#	define PTHREADS_CALL_METHOD_PASSTHRU_C method, INTERNAL_FUNCTION_PARAM_PASSTHRU
#else
#	define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type TSRMLS_DC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type TSRMLS_CC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value  TSRMLS_DC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value TSRMLS_CC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists TSRMLS_DC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists TSRMLS_CC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member TSRMLS_DC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member TSRMLS_CC
#	define PTHREADS_GET_METHOD_PASSTHRU_D zval **pobject, char *method, int methodl TSRMLS_DC
#	define PTHREADS_GET_METHOD_PASSTHRU_C pobject, method, methodl TSRMLS_CC
#	define PTHREADS_CALL_METHOD_PASSTHRU_D char *method, INTERNAL_FUNCTION_PARAMETERS
#	define PTHREADS_CALL_METHOD_PASSTHRU_C method, INTERNAL_FUNCTION_PARAM_PASSTHRU
#endif /* }}} */

/* {{{ pthreads_read_property will attempt to reference the threading context from whichever context it was called (creator/import) */
zval * pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D); /* }}} */

/* {{{ pthreads_write_property will not attempt to reference the threading context, but does require the thread lock to ensure safe concurrent reads */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D); /* }}} */

/* {{{ pthreads_has_property should attempt to reference the threading context to save allocs and have isset work as expected */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D); /* }}} */

/* {{{ pthreads_unset_property will not attempt to reference any other context, but does require the therad lock to ensure safe concurrent reads */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D); /* }}} */

/* {{{ pthreads_get_method will read modifiers from the correct context and apply them in the current context */
zend_function * pthreads_get_method(PTHREADS_GET_METHOD_PASSTHRU_D); /* }}} */

/* {{{ pthreads_call_method will syncrhonize access to protected methods, such that only one context can call protected methods at a time */
int pthreads_call_method(PTHREADS_CALL_METHOD_PASSTHRU_D); /* }}} */
#endif
