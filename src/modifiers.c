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
#ifndef HAVE_PTHREADS_MODIFIERS
#define HAVE_PTHREADS_MODIFIERS

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

/* {{{ initialize pthreads modifiers for the references thread */
void pthreads_modifiers_init(PTHREAD thread, zval *this_ptr TSRMLS_DC) {
	HashPosition position;
	zend_function *method;
	
	for (zend_hash_internal_pointer_reset_ex(&Z_OBJCE_P(getThis())->function_table, &position);
		 zend_hash_get_current_data_ex(&Z_OBJCE_P(getThis())->function_table, (void**)&method, &position) == SUCCESS;
		 zend_hash_move_forward_ex(&Z_OBJCE_P(getThis())->function_table, &position)) {
		 /*
		 * Check if element is user declared method
		 */
		if (method && (method->type != ZEND_INTERNAL_FUNCTION)) {
			/*
			* Check if method is private
			*/
			if (method->common.fn_flags & ZEND_ACC_PRIVATE){
				pthreads_modifiers_set(
					thread,
					method->common.function_name,
					ZEND_ACC_PRIVATE TSRMLS_CC
				);
				function_add_ref(method);
				method->type = ZEND_OVERLOADED_FUNCTION;
			}
			
			/*
			* Check if method is protected
			*/
			if (method->common.fn_flags & ZEND_ACC_PROTECTED) {
				pthreads_modifiers_set(
					thread,
					method->common.function_name,
					ZEND_ACC_PROTECTED TSRMLS_CC
				);
				function_add_ref(method);
				method->type = ZEND_OVERLOADED_FUNCTION;
			}
		}
	}
} /* }}} */

/* {{{ sets access modifiers for a method by name */
int pthreads_modifiers_set(PTHREAD thread, const char *method, zend_uint modify TSRMLS_DC) {
	if (thread) {
		zend_uint *modified = calloc(1, sizeof(*modified));
		{
			*modified = modify;
			if (zend_hash_add(
				thread->modifiers, 
				method, sizeof(method), 
				(void**) &modified, sizeof(zend_uint*), 
				NULL
			)==SUCCESS) {
				return SUCCESS;
			}
		}
	}
	return FAILURE;
} /* }}} */

/* {{{ retrieve access modifiers for a method by name */
zend_uint pthreads_modifiers_get(PTHREAD thread, const char *method TSRMLS_DC) {
	zend_uint **modifiers;
	if (zend_hash_find(
			thread->modifiers,
			method, sizeof(method), 
			(void**) &modifiers
		)==SUCCESS) {
		return **modifiers;
	}
	return 0;
} /* }}} */

/* {{{ lockdown !! */
int pthreads_modifiers_protect(PTHREAD thread) {
	switch(pthread_mutex_lock(thread->lock)) {
		case SUCCESS:
		case EDEADLK: 
			return SUCCESS;
		
		default: return PTHREADS_PROTECTION_ERROR;
	}
} /* }}} */

/* {{{ release */
int pthreads_modifiers_unprotect(PTHREAD thread) {
	switch(pthread_mutex_unlock(thread->lock)) {
		case SUCCESS: 
			return SUCCESS;
		
		default: return PTHREADS_PROTECTION_ERROR;
	}
} /* }}} */

/* {{{ destructor callback for modifiers hash table */
void pthreads_modifiers_destroy(void **element) {
	zend_uint *modified = (zend_uint *) *element;
	if (modified && *modified) {
		free(modified);
	}
} /* }}} */

#endif
