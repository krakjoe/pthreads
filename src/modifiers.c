/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

/* {{{ statics */
static void pthreads_modifiers_modifiers_dtor(zval *element);
static void pthreads_modifiers_protection_dtor(zval *element); /* }}} */

/* {{{ allocate modifiers */
pthreads_modifiers pthreads_modifiers_alloc() {
	pthreads_modifiers modifiers = calloc(1, sizeof(*modifiers));
	
	if (modifiers) {
		/*
		* Initialize modifiers
		*/
		zend_hash_init(&modifiers->modified, 32, NULL, (dtor_func_t) pthreads_modifiers_modifiers_dtor, 1);
		zend_hash_init(&modifiers->protection, 32, NULL, (dtor_func_t) pthreads_modifiers_protection_dtor, 1);
	}
	
	return modifiers;
} /* }}} */

/* {{{ initialize pthreads modifiers using the referenced class entry */
void pthreads_modifiers_init(pthreads_modifiers modifiers, zend_class_entry *entry) {
	HashPosition position;
	zend_function *method;
	zend_string   *name;

#define SET(m) 	pthreads_modifiers_set(modifiers, name, m) 
	ZEND_HASH_FOREACH_STR_KEY_PTR(&entry->function_table, name, method) {
		if (method->type != ZEND_USER_FUNCTION)
			continue;

		switch (method->common.fn_flags & ZEND_ACC_PPP_MASK) {
			case ZEND_ACC_PRIVATE: SET(ZEND_ACC_PRIVATE); break;
			case ZEND_ACC_PROTECTED: SET(ZEND_ACC_PROTECTED); break;
		}
		
	} ZEND_HASH_FOREACH_END();
#undef SET
} /* }}} */

/* {{{ set access modifier for method */
int pthreads_modifiers_set(pthreads_modifiers modifiers, zend_string *method, int32_t modify) {
	int32_t *modified = 
		(int32_t*) malloc(sizeof(int32_t));
	
	memcpy(modified, &modify, sizeof(int32_t));

	if (zend_hash_add_ptr(
		&modifiers->modified, method, modified)) {
		pthreads_lock lock = pthreads_lock_alloc();
		if (lock) {
			if (zend_hash_add_ptr(
				&modifiers->protection, method, lock)) {
				return SUCCESS;
			}
		}

		zend_hash_del(&modifiers->modified, method);
	}

	free(modified);
	
	return FAILURE;
} /* }}} */

/* {{{ get access modifier for method */
int32_t pthreads_modifiers_get(pthreads_modifiers modifiers, zend_string *method) {
	int32_t *modified;
	if ((modified = zend_hash_find_ptr(&modifiers->modified, method))) {
		return *modified;
	}
	return 0;
} /* }}} */

/* {{{ protect a method call */
zend_bool pthreads_modifiers_protect(pthreads_modifiers modifiers, zend_string *method, zend_bool *unprotect) {
	pthreads_lock *protection;
	if ((protection = zend_hash_find_ptr(&modifiers->protection, method))) {
		return pthreads_lock_acquire(*protection, unprotect);
	} else return 0;
} /* }}} */

/* {{{ unprotect a method call */
zend_bool pthreads_modifiers_unprotect(pthreads_modifiers modifiers, zend_string *method, zend_bool unprotect) {
	pthreads_lock *protection;
	if ((protection = zend_hash_find_ptr(&modifiers->protection, method))) {
		return pthreads_lock_release(*protection, unprotect);
	} else return 0;
} /* }}} */

/* {{{ free modifiers */
void pthreads_modifiers_free(pthreads_modifiers modifiers) {
	if (modifiers) {
	    zend_hash_destroy(&modifiers->modified);
	    zend_hash_destroy(&modifiers->protection);
	    free(modifiers);
	}
} /* }}} */

/* {{{ destructor callback for modifiers (definition) hash table */
static void pthreads_modifiers_modifiers_dtor(zval *zv) {
	free(Z_PTR_P(zv));	
} /* }}} */

/* {{{ destructor callback for modifiers (protection) hash table */
static void pthreads_modifiers_protection_dtor(zval *zv) {
	pthreads_lock_free((pthreads_lock) Z_PTR_P(zv));
} /* }}} */

#endif

