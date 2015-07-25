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

static inline void pthreads_object_iterator_dtor(zend_object_iterator* iterator);
static inline int pthreads_object_iterator_validate(zend_object_iterator* iterator);
static inline zval* pthreads_object_iterator_current_data(zend_object_iterator* iterator);
static inline void pthreads_object_iterator_current_key(zend_object_iterator* iterator, zval *key);
static inline void pthreads_object_iterator_move_forward(zend_object_iterator* iterator);

zend_object_iterator_funcs pthreads_object_iterator_funcs = {
    pthreads_object_iterator_dtor,
    pthreads_object_iterator_validate,
    pthreads_object_iterator_current_data,
    pthreads_object_iterator_current_key,
    pthreads_object_iterator_move_forward,
    NULL  
};

static inline zend_object_iterator* pthreads_object_iterator_ctor(zend_class_entry *ce, zval *object, int by_ref) {
    pobject_iterator_t *iterator = emalloc(sizeof(pobject_iterator_t));
    
    zend_iterator_init((zend_object_iterator*)iterator);
    zend_hash_init(
        &iterator->properties, 8, NULL, ZVAL_PTR_DTOR, 0);

    {
        PTHREAD pobject = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
         
        pthreads_store_tohash(
            pobject->store, &iterator->properties);
        
        zend_hash_internal_pointer_reset_ex(
            &iterator->properties, &iterator->position);
            
        iterator->end = 0;
    }

    iterator->zit.funcs = &pthreads_object_iterator_funcs;  

    return (zend_object_iterator*) iterator;
}

static inline void pthreads_object_iterator_dtor(zend_object_iterator* iterator) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    {
        zend_hash_destroy(
            &intern->properties
        );
    }
    
    efree(intern);
}

static inline int pthreads_object_iterator_validate(zend_object_iterator* iterator) {
   pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
   
   if (zend_hash_num_elements(&intern->properties)) {
       return (((pobject_iterator_t*)iterator)->end) ? FAILURE : SUCCESS;
   } else return FAILURE;
}

static inline zval* pthreads_object_iterator_current_data(zend_object_iterator* iterator) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    zval *data = NULL;

    if (!intern->end) {
        if (!(data = zend_hash_get_current_data_ex(&intern->properties, &intern->position))) {
            intern->end = 1;
        }
    }

    return data;
}

static inline void pthreads_object_iterator_current_key(zend_object_iterator* iterator, zval* key) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    zend_string *skey = NULL;
    ulong ukey;
    
    switch (zend_hash_get_current_key_ex(
        &intern->properties, &skey, &ukey, &intern->position)) {
        case HASH_KEY_IS_STRING: {
            ZVAL_STRINGL(key, skey->val, skey->len - 1);
        } break;
            
        case HASH_KEY_IS_LONG: {
            ZVAL_LONG(key, ukey);
        } break;
            
        default: {
            intern->end = 1;
        }    
    }
}

static inline void pthreads_object_iterator_move_forward(zend_object_iterator* iterator) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    if (!intern->end) {
        zend_hash_move_forward_ex(
            &intern->properties, &intern->position);
        {
            if (!zend_hash_get_current_data_ex(&intern->properties, &intern->position)) {
                intern->end = 1;
            }
        }
    }
}
#endif
