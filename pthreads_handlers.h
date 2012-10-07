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
* These handlers are mutex aware for safer operation in multi-threaded applications
*/
#ifndef HAVE_PTHREADS_HANDLERS_H
#define HAVE_PTHREADS_HANDLERS_H
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
#else
#	define PTHREADS_READ_PROPERTY_PASSTHRU_D zval *object, zval *member, int type TSRMLS_DC
#	define PTHREADS_READ_PROPERTY_PASSTHRU_C object, member, type TSRMLS_CC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_D zval *object, zval *member, zval *value  TSRMLS_DC
#	define PTHREADS_WRITE_PROPERTY_PASSTHRU_C object, member, value TSRMLS_CC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_D zval *object, zval *member, int has_set_exists TSRMLS_DC
#	define PTHREADS_HAS_PROPERTY_PASSTHRU_C object, member, has_set_exists TSRMLS_CC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_D zval *object, zval *member TSRMLS_DC
#	define PTHREADS_UNSET_PROPERTY_PASSTHRU_C object, member TSRMLS_CC
#endif /* }}} */

zval * pthreads_read_property(PTHREADS_READ_PROPERTY_PASSTHRU_D);
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D);
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D);
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D);

/* {{ read_property */
zval * pthreads_read_property (PTHREADS_READ_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	int oacquire = 0;
	
	zval **lookup = NULL;
	zval *property = NULL;
	char *serial = NULL;
	
	if (thread) {
		acquire = pthread_mutex_lock(PTHREADS_IS_IMPORT(thread) ? thread->sig->lock : thread->lock);
		
		if (acquire == SUCCESS || acquire == EDEADLK) {
			if (!PTHREADS_IS_SELF(thread) && !PTHREADS_IS_IMPORT(thread)) {
				if (!PTHREADS_IS_JOINED(thread->sig)) {
					oacquire = pthread_mutex_lock(thread->sig->lock);

					if (oacquire == SUCCESS || oacquire == EDEADLK) {
						if (!thread->sig->std.properties->inconsistent && /* check for table being destroyed */
							zend_hash_find(thread->sig->std.properties, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void**)&lookup)==SUCCESS) {
							switch(Z_TYPE_PP(lookup)){
								case IS_LONG:
								case IS_BOOL:
								case IS_STRING:
								case IS_NULL:
								case IS_ARRAY:
									serial = pthreads_serialize(*lookup TSRMLS_CC);
							
									if (serial) {
										if ((property = pthreads_unserialize(serial TSRMLS_CC)) != NULL)
											Z_SET_REFCOUNT_P(property, 0);
										free(serial);
									}
								break;
								
								/* it's possible to allow access to an object, however I feel it's unsafe and too heavy */
								/* objects will become available when the thread is joined and we know for sure the object is no longer being manipulated by another context */
								case IS_OBJECT:
								case IS_RESOURCE:
									zend_error(E_WARNING, "pthreads detected an attempt to fetch an unsupported symbol (%s)", Z_STRVAL_P(member));
								break;
							}
						}
						
						if (oacquire != EDEADLK)
							pthread_mutex_unlock(thread->sig->lock);
					} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
				}
			} else if (PTHREADS_IS_IMPORT(thread)) {
				oacquire = pthread_mutex_lock(thread->sig->sig->lock);
				
				if (oacquire == SUCCESS || oacquire == EDEADLK) {
					if (!PTHREADS_ST_CHECK(thread->state, PTHREADS_ST_JOINED)) {
						if (!thread->sig->sig->std.properties->inconsistent && /* check for table being destroyed */
							zend_hash_find(thread->sig->sig->std.properties, Z_STRVAL_P(member), Z_STRLEN_P(member)+1, (void**)&lookup)==SUCCESS) {
							switch(Z_TYPE_PP(lookup)){
								case IS_LONG:
								case IS_BOOL:
								case IS_STRING:
								case IS_NULL:
								case IS_ARRAY:
									serial = pthreads_serialize(*lookup TSRMLS_CC);
							
									if (serial) {
										if ((property = pthreads_unserialize(serial TSRMLS_CC)) != NULL)
											Z_SET_REFCOUNT_P(property, 0);
										free(serial);
									}
								break;
								
								/* it's possible to allow access to an object, however I feel it's unsafe and too heavy */
								/* objects will become available when the thread is joined and we know for sure the object is no longer being manipulated by another context */
								case IS_OBJECT:
								case IS_RESOURCE:
									zend_error(E_WARNING, "pthreads detected an attempt to fetch an unsupported symbol (%s)", Z_STRVAL_P(member));
								break;
							}
						}
					}
					if (oacquire != EDEADLK)
						pthread_mutex_unlock(thread->sig->sig->lock);
				}
			}
			
			if (property == NULL)
				property = zsh->read_property(PTHREADS_READ_PROPERTY_PASSTHRU_C);
			
			if (acquire != EDEADLK)
				pthread_mutex_unlock(PTHREADS_IS_IMPORT(thread) ? thread->sig->lock : thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return property;
} /* }}} */

/* {{{ write_property */
void pthreads_write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->write_property(PTHREADS_WRITE_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

/* {{{ has_property */
int pthreads_has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	int result = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			result = zsh->has_property(PTHREADS_HAS_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
	
	return result;
} /* }}} */

/* {{{ unset_property */
void pthreads_unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_D) {
	PTHREAD thread = PTHREADS_FETCH_FROM(object);
	int acquire = 0;
	
	if (thread) {
		acquire = pthread_mutex_lock(thread->lock);
		if (acquire == SUCCESS || acquire == EDEADLK) {
			zsh->unset_property(PTHREADS_UNSET_PROPERTY_PASSTHRU_C);
			if (acquire != EDEADLK)
				pthread_mutex_unlock(thread->lock);
		} else zend_error(E_ERROR, "pthreads has experienced an internal error and cannot continue");
	}
} /* }}} */

#endif
