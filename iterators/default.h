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
#ifndef HAVE_PTHREADS_ITERATOR_DEFAULT
#define HAVE_PTHREADS_ITERATOR_DEFAULT

#ifndef HAVE_PTHREADS_ITERATOR_H
# include <iterators/iterator.h>
#endif

static inline void pthreads_object_iterator_dtor(zend_object_iterator* iterator TSRMLS_DC);
static inline int pthreads_object_iterator_validate(zend_object_iterator* iterator TSRMLS_DC);
static inline void pthreads_object_iterator_current_data(zend_object_iterator* iterator, zval ***data TSRMLS_DC);
#if PHP_VERSION_ID > 50500
static inline void pthreads_object_iterator_current_key(zend_object_iterator* iterator, zval *key TSRMLS_DC);
#else
static inline int pthreads_object_iterator_current_key(zend_object_iterator* iterator, char **key, uint *klen, ulong *ukey TSRMLS_DC);
#endif
static inline void pthreads_object_iterator_move_forward(zend_object_iterator* iterator TSRMLS_DC);

zend_object_iterator_funcs pthreads_object_iterator_funcs = {
    pthreads_object_iterator_dtor,
    pthreads_object_iterator_validate,
    pthreads_object_iterator_current_data,
    pthreads_object_iterator_current_key,
    pthreads_object_iterator_move_forward,
    NULL  
};

static inline zend_object_iterator* pthreads_object_iterator_ctor(zend_class_entry *ce, zval *object, int by_ref TSRMLS_DC) {
    pobject_iterator_t *iterator = emalloc(sizeof(pobject_iterator_t));
    
    iterator->zit.funcs = &pthreads_object_iterator_funcs;  
    
    zend_hash_init(
        &iterator->properties, 8, NULL, ZVAL_PTR_DTOR, 0);

    {
        PTHREAD pobject = PTHREADS_FETCH_FROM(object);
                
        pthreads_store_tohash(
            pobject->store, &iterator->properties TSRMLS_CC);
        
        zend_hash_internal_pointer_reset_ex(
            &iterator->properties, &iterator->position);
            
        iterator->end = 0;
    }

    return (zend_object_iterator*) iterator;
}

static inline void pthreads_object_iterator_dtor(zend_object_iterator* iterator TSRMLS_DC) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    {
        zend_hash_destroy(
            &intern->properties
        );
    }
    
    efree(intern);
}

static inline int pthreads_object_iterator_validate(zend_object_iterator* iterator TSRMLS_DC) {
   return (((pobject_iterator_t*)iterator)->end) ? FAILURE : SUCCESS;
}

static inline void pthreads_object_iterator_current_data(zend_object_iterator* iterator, zval ***data TSRMLS_DC) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    if (!intern->end) {
        if (zend_hash_get_current_data_ex(  
            &intern->properties, (void**) data, &intern->position) != SUCCESS) {
            intern->end = 1;
        }        
    }
}

#if PHP_VERSION_ID >= 50500
static inline void pthreads_object_iterator_current_key(zend_object_iterator* iterator, zval* key TSRMLS_DC) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    char *skey = NULL;
    uint sklen;
    ulong ukey;
    
    switch (zend_hash_get_current_key_ex(
        &intern->properties, &skey, &sklen, &ukey, 0, &intern->position)) {
        case HASH_KEY_IS_STRING: {
            ZVAL_STRINGL(key, skey, sklen - 1, 1);
        } break;
            
        case HASH_KEY_IS_LONG: {
            ZVAL_LONG(key, ukey);
        } break;
            
        default: {
            intern->end = 1;
        }    
    }
}
#else
static inline int pthreads_object_iterator_current_key(zend_object_iterator* iterator, char **key, uint *klen, ulong *ukey TSRMLS_DC) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    switch (zend_hash_get_current_key_ex(
        &intern->properties, key, klen, ukey, 1, &intern->position)) {
        case HASH_KEY_IS_STRING:
            return HASH_KEY_IS_STRING;
        case HASH_KEY_IS_LONG:
            return HASH_KEY_IS_LONG;
            
        default: {
            intern->end = 1;
        }    
    }
    return FAILURE;
}
#endif

static inline void pthreads_object_iterator_move_forward(zend_object_iterator* iterator TSRMLS_DC) {
    pobject_iterator_t *intern = (pobject_iterator_t*) iterator;
    
    if (!intern->end) {
        zend_hash_move_forward_ex(
            &intern->properties, &intern->position);
        {
            if (zend_hash_get_current_data_ex(
                &intern->properties, (void**) &intern->zit.data, &intern->position) != SUCCESS) {
                intern->end = 1;
            }
        }
    }
}
#endif
