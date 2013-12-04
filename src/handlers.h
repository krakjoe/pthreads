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
* These handlers provide thread-safe read/write/call/count/cast for pthreads objects
*/
#ifndef HAVE_PTHREADS_HANDLERS_H
#define HAVE_PTHREADS_HANDLERS_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#define PTHREADS_CAST_PASSTHRU_D zval *from, zval *to, int type TSRMLS_DC
#define PTHREADS_CAST_PASSTHRU_C from, to, type TSRMLS_CC
#define PTHREADS_COUNT_PASSTHRU_D zval *object, long *count TSRMLS_DC
#define PTHREADS_COUNT_PASSTHRU_C object, count TSRMLS_CC
#define PTHREADS_CLONE_PASSTHRU_D zval *object TSRMLS_DC
#define PTHREADS_CLONE_PASSTHRU_C object TSRMLS_CC

/* {{{ these resolve differences in 5.3 and 5.4 object handling API */
#if PHP_VERSION_ID > 50399
#	define PTHREADS_READ_DEBUG_PASSTHRU_D zval *object, int *is_temp TSRMLS_DC
#	define PTHREADS_READ_DEBUG_PASSTHRU_C object, is_temp TSRMLS_CC
#	define PTHREADS_READ_PROPERTIES_PASSTHRU_D zval *object TSRMLS_DC
#	define PTHREADS_READ_PROPERTIES_PASSTHRU_C object TSRMLS_CC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_READ_DIMENSION_PASSTHRU_D zval *object, zval *member, int type TSRMLS_DC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type, key TSRMLS_CC
#	define PTHREADS_READ_DIMENSION_PASSTHRU_C object, member, type, NULL TSRMLS_CC

#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_WRITE_DIMENSION_PASSTHRU_D zval *object, zval *member, zval *value TSRMLS_DC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value, key TSRMLS_CC
#	define PTHREADS_WRITE_DIMENSION_PASSTHRU_C object, member, value, NULL TSRMLS_CC

#	define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_HAS_DIMENSION_PASSTHRU_D zval *object, zval *member, int has_set_exists TSRMLS_DC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists, key TSRMLS_CC
#	define PTHREADS_HAS_DIMENSION_PASSTHRU_C object, member, has_set_exists, NULL TSRMLS_CC

#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_UNSET_DIMENSION_PASSTHRU_D zval *object, zval *member TSRMLS_DC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member, key TSRMLS_CC
#	define PTHREADS_UNSET_DIMENSION_PASSTHRU_C object, member, NULL TSRMLS_CC

#	define PTHREADS_GET_METHOD_PASSTHRU_D zval **pobject, char *method, int methodl, const struct _zend_literal *key TSRMLS_DC
#	define PTHREADS_GET_METHOD_PASSTHRU_C pobject, method, methodl, key TSRMLS_CC
#	define PTHREADS_CALL_METHOD_PASSTHRU_D const char *method, INTERNAL_FUNCTION_PARAMETERS
#	define PTHREADS_CALL_METHOD_PASSTHRU_C method, INTERNAL_FUNCTION_PARAM_PASSTHRU
#else
#	define PTHREADS_READ_DEBUG_PASSTHRU_D zval *object, int *is_temp TSRMLS_DC
#	define PTHREADS_READ_DEBUG_PASSTHRU_C object, is_temp TSRMLS_CC
#	define PTHREADS_READ_PROPERTIES_PASSTHRU_D zval *object TSRMLS_DC
#	define PTHREADS_READ_PROPERTIES_PASSTHRU_C object TSRMLS_CC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type TSRMLS_DC
#	define PTHREADS_READ_DIMENSION_PASSTHRU_D PTHREADS_READ_PROPERTY_PASSTHRU_D
#	define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type TSRMLS_CC
#	define PTHREADS_READ_DIMENSION_PASSTHRU_C PTHREADS_READ_PROPERTY_PASSTHRU_C

#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value TSRMLS_DC
#	define PTHREADS_WRITE_DIMENSION_PASSTHRU_D PTHREADS_WRITE_PROPERTY_PASSTHRU_D
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value TSRMLS_CC
#	define PTHREADS_WRITE_DIMENSION_PASSTHRU_C PTHREADS_WRITE_PROPERTY_PASSTHRU_C

#	define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists TSRMLS_DC
#	define PTHREADS_HAS_DIMENSION_PASSTHRU_D PTHREADS_HAS_PROPERTY_PASSTHRU_D
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists TSRMLS_CC
#	define PTHREADS_HAS_DIMENSION_PASSTHRU_C PTHREADS_HAS_PROPERTY_PASSTHRU_C

#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member TSRMLS_DC
#	define PTHREADS_UNSET_DIMENSION_PASSTHRU_D PTHREADS_UNSET_PROPERTY_PASSTHRU_D
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member TSRMLS_CC
#	define PTHREADS_UNSET_DIMENSION_PASSTHRU_C PTHREADS_UNSET_PROPERTY_PASSTHRU_C

#	define PTHREADS_GET_METHOD_PASSTHRU_D zval **pobject, char *method, int methodl TSRMLS_DC
#	define PTHREADS_GET_METHOD_PASSTHRU_C pobject, method, methodl TSRMLS_CC
#	define PTHREADS_CALL_METHOD_PASSTHRU_D char *method, INTERNAL_FUNCTION_PARAMETERS
#	define PTHREADS_CALL_METHOD_PASSTHRU_C method, INTERNAL_FUNCTION_PARAM_PASSTHRU
#endif /* }}} */

/* {{{ read proeprties from storage */
HashTable* pthreads_read_debug(PTHREADS_READ_DEBUG_PASSTHRU_D); /* }}} */

/* {{{ read proeprties from storage */
HashTable* pthreads_read_properties(PTHREADS_READ_PROPERTIES_PASSTHRU_D); /* }}} */

/* {{{ read a property from the referenced thread */
zval * pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D); 
zval * pthreads_read_dimension(PTHREADS_READ_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ write a property to the referenced thread */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D); 
void pthreads_write_dimension(PTHREADS_WRITE_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ check if the referenced thread has a specific property */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D); 
int pthreads_has_dimension(PTHREADS_HAS_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ unset a property in the referenced thread */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D); 
void pthreads_unset_dimension(PTHREADS_UNSET_DIMENSION_PASSTHRU_D); /* }}} */

/* {{{ count properties in storage */
int pthreads_count_properties(PTHREADS_COUNT_PASSTHRU_D); /* }}} */

/* {{{ fetch a pthreads friendly method */
zend_function * pthreads_get_method(PTHREADS_GET_METHOD_PASSTHRU_D); /* }}} */

/* {{{ make a pthreads method call */
int pthreads_call_method(PTHREADS_CALL_METHOD_PASSTHRU_D); /* }}} */

/* {{{ cast an object to a normal array helper */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D); /* }}} */

/* {{{ clone object handler */
zend_object_value pthreads_clone_object(PTHREADS_CLONE_PASSTHRU_D); /* }}} */
#endif
