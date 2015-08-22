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

zend_object_iterator_funcs pthreads_object_iterator_funcs = {
    (void (*) (zend_object_iterator*)) 				pthreads_object_iterator_dtor,
    (int (*)(zend_object_iterator *)) 				pthreads_object_iterator_validate,
    (zval* (*)(zend_object_iterator *)) 			pthreads_object_iterator_current_data,
    (void (*)(zend_object_iterator *, zval *)) 		pthreads_object_iterator_current_key,
    (void (*)(zend_object_iterator *))				pthreads_object_iterator_move_forward,
    NULL 
};

static inline zend_object_iterator* pthreads_object_iterator_ctor(zend_class_entry *ce, zval *object, int by_ref) {
    pthreads_iterator_t *iterator = ecalloc(1, sizeof(pthreads_iterator_t));
    
    zend_iterator_init((zend_object_iterator*)iterator);

	ZVAL_COPY(&iterator->object, object);

    {
        pthreads_store store = (PTHREADS_FETCH_FROM(Z_OBJ(iterator->object)))->store;
		zend_bool locked;		
		
        if (pthreads_lock_acquire(store->lock, &locked)) {
			zend_string *key;

			 zend_hash_init(
        		&iterator->keys, 
				zend_hash_num_elements(&store->table), 
				NULL, ZVAL_PTR_DTOR, 0);			
			
			ZEND_HASH_FOREACH_STR_KEY(&store->table, key) {
				zend_hash_add_empty_element(
					&iterator->keys, key);
			} ZEND_HASH_FOREACH_END();

			pthreads_lock_release(store->lock, locked);
		}

        zend_hash_internal_pointer_reset_ex(
            &iterator->keys, &iterator->position);
    }

    iterator->zit.funcs = &pthreads_object_iterator_funcs;
	ZVAL_UNDEF(&iterator->zit.data);

    return (zend_object_iterator*) iterator;
}

static inline void pthreads_object_iterator_dtor(pthreads_iterator_t* iterator) {
	if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
		zval_ptr_dtor(&iterator->zit.data);
	}
    zend_hash_destroy(&iterator->keys);
	zval_ptr_dtor(&iterator->object);
}

static inline int pthreads_object_iterator_validate(pthreads_iterator_t* iterator) {
   if (zend_hash_num_elements(&iterator->keys)) {
       return iterator->end ? FAILURE : SUCCESS;
   } else return FAILURE;
}

static inline zval* pthreads_object_iterator_current_data(pthreads_iterator_t* iterator) {
	zend_string *key;
	zend_ulong idx;

    if (!iterator->end) {
        switch (zend_hash_get_current_key_ex(&iterator->keys, &key, &idx, &iterator->position)) {
			case HASH_KEY_IS_STRING: {
				if (Z_TYPE(iterator->zit.data) != IS_UNDEF) {
					zval_ptr_dtor(&iterator->zit.data);
				}
				
				pthreads_store_read((PTHREADS_FETCH_FROM(Z_OBJ(iterator->object)))->store, key, &iterator->zit.data);
			} break;
		}
    }

    return &iterator->zit.data;
}

static inline void pthreads_object_iterator_current_key(pthreads_iterator_t* iterator, zval* result) {
    zend_string *key = NULL;
    ulong idx;
    
    switch (zend_hash_get_current_key_ex(
        &iterator->keys, &key, &idx, &iterator->position)) {
        case HASH_KEY_IS_STRING: {
            ZVAL_STR(result, key);
        } break;
            
        default: {
            iterator->end = 1;
        }
    }
}

static inline void pthreads_object_iterator_move_forward(pthreads_iterator_t* iterator) {
    if (!iterator->end) {
        zend_hash_move_forward_ex(
            &iterator->keys, &iterator->position);
        {
            if (!zend_hash_get_current_data_ex(&iterator->keys, &iterator->position)) {
                iterator->end = 1;
            }
        }
    }
}
#endif
