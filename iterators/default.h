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
#ifndef HAVE_PTHREADS_ITERATOR_DEFAULT
#define HAVE_PTHREADS_ITERATOR_DEFAULT

#ifndef HAVE_PTHREADS_ITERATOR_H
# include <iterators/iterator.h>
#endif

static inline void pthreads_object_iterator_dtor(pthreads_iterator_t* iterator);
static inline int pthreads_object_iterator_validate(pthreads_iterator_t* iterator);
static inline zval* pthreads_object_iterator_current_data(pthreads_iterator_t* iterator);
static inline void pthreads_object_iterator_current_key(pthreads_iterator_t* iterator, zval *key);
static inline void pthreads_object_iterator_move_forward(pthreads_iterator_t* iterator);
static inline void pthreads_object_iterator_rewind(pthreads_iterator_t* iterator);

zend_object_iterator_funcs pthreads_object_iterator_funcs = {
    (void (*) (zend_object_iterator*)) 				pthreads_object_iterator_dtor,
    (int (*)(zend_object_iterator *)) 				pthreads_object_iterator_validate,
    (zval* (*)(zend_object_iterator *)) 			pthreads_object_iterator_current_data,
    (void (*)(zend_object_iterator *, zval *)) 		pthreads_object_iterator_current_key,
    (void (*)(zend_object_iterator *))				pthreads_object_iterator_move_forward,
    (void (*)(zend_object_iterator *)) 				pthreads_object_iterator_rewind
};

static inline zend_object_iterator* pthreads_object_iterator_ctor(zend_class_entry *ce, zval *object, int by_ref) {
    pthreads_iterator_t *iterator = ecalloc(1, sizeof(pthreads_iterator_t));
	
    zend_iterator_init((zend_object_iterator*)iterator);

	ZVAL_COPY(&iterator->object, object);
	ZVAL_UNDEF(&iterator->zit.data);

	pthreads_store_reset(&iterator->object, &iterator->position);

    iterator->zit.funcs = &pthreads_object_iterator_funcs;

    return (zend_object_iterator*) iterator;
}

static inline void pthreads_object_iterator_dtor(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF)
		zval_ptr_dtor(&iterator->zit.data);
	zval_ptr_dtor(&iterator->object);
}

static inline int pthreads_object_iterator_validate(pthreads_iterator_t* iterator) {
	return (iterator->position != HT_INVALID_IDX) ? SUCCESS : FAILURE;
}

static inline zval* pthreads_object_iterator_current_data(pthreads_iterator_t* iterator) {
	pthreads_store_data(&iterator->object, &iterator->zit.data, &iterator->position);

	if (Z_ISUNDEF(iterator->zit.data)) {
		return &EG(uninitialized_zval);
	}

    return &iterator->zit.data;
}

static inline void pthreads_object_iterator_current_key(pthreads_iterator_t* iterator, zval* result) {
    pthreads_store_key(&iterator->object, result, &iterator->position);
}

static inline void pthreads_object_iterator_move_forward(pthreads_iterator_t* iterator) {
    pthreads_store_forward(&iterator->object, &iterator->position);
}

static inline void pthreads_object_iterator_rewind(pthreads_iterator_t* iterator) {
    pthreads_store_reset(&iterator->object, &iterator->position);
}
#endif
