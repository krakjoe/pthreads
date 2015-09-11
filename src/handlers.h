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

/*
* These handlers provide thread-safe read/write/call/count/cast for pthreads objects
*/
#ifndef HAVE_PTHREADS_HANDLERS_H
#define HAVE_PTHREADS_HANDLERS_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#define PTHREADS_CAST_PASSTHRU_D zval *from, zval *to, int type
#define PTHREADS_CAST_PASSTHRU_C from, to, type
#define PTHREADS_COUNT_PASSTHRU_D zval *object, long *count
#define PTHREADS_COUNT_PASSTHRU_C object, count
#define PTHREADS_CLONE_PASSTHRU_D zval *object
#define PTHREADS_CLONE_PASSTHRU_C object
#define PTHREADS_COMPARE_PASSTHRU_D zval *op1, zval *op2
#define PTHREADS_COMPARE_PASSTHRU_C op1, op2

/* {{{  */
#define PTHREADS_READ_DEBUG_PASSTHRU_D zval *object, int *is_temp
#define PTHREADS_READ_DEBUG_PASSTHRU_C object, is_temp
#define PTHREADS_READ_PROPERTIES_PASSTHRU_D zval *object
#define PTHREADS_READ_PROPERTIES_PASSTHRU_C object
#define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type, void **cache, zval *rv
#define PTHREADS_READ_DIMENSION_PASSTHRU_D zval *object, zval *member, int type, zval *rv
#define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type, cache, rv
#define PTHREADS_READ_DIMENSION_PASSTHRU_C object, member, type, NULL, rv

#define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value, void **cache
#define PTHREADS_WRITE_DIMENSION_PASSTHRU_D zval *object, zval *member, zval *value
#define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value, cache
#define PTHREADS_WRITE_DIMENSION_PASSTHRU_C object, member, value, NULL

#define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists, void **cache
#define PTHREADS_HAS_DIMENSION_PASSTHRU_D zval *object, zval *member, int has_set_exists
#define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists, cache
#define PTHREADS_HAS_DIMENSION_PASSTHRU_C object, member, has_set_exists, NULL

#define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member, void **cache
#define PTHREADS_UNSET_DIMENSION_PASSTHRU_D zval *object, zval *member
#define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member, cache
#define PTHREADS_UNSET_DIMENSION_PASSTHRU_C object, member, NULL /* }}} */

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

/* {{{ cast an object to a normal array helper */
int pthreads_cast_object(PTHREADS_CAST_PASSTHRU_D); /* }}} */

/* {{{ clone object handler */
zend_object* pthreads_clone_object(PTHREADS_CLONE_PASSTHRU_D); /* }}} */

/* {{{ */
int pthreads_compare_objects(PTHREADS_COMPARE_PASSTHRU_D); /* }}} */
#endif
