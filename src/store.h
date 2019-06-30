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
#ifndef HAVE_PTHREADS_STORE_H
#define HAVE_PTHREADS_STORE_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#define IS_CLOSURE  (IS_PTR + 1)
#define IS_PTHREADS (IS_PTR + 2)
#define IS_MAP      (IS_PTR + 3)

pthreads_store_t* pthreads_store_alloc();
void pthreads_store_sync(zval *object);
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite);
int pthreads_store_delete(zval *object, zval *key);
int _pthreads_store_delete(zval *object, zval *key, int std_obj_sync);
int pthreads_store_read(zval *object, zval *key, int type, zval *read);
zend_bool pthreads_store_isset(zval *object, zval *key, int has_set_exists);
int pthreads_store_write(zval *object, zval *key, zval *write);
int _pthreads_store_write(zval *object, zval *key, zval *write, int std_obj_sync);
int pthreads_store_separate(zval *pzval, zval *seperated, zend_bool complex);
void pthreads_store_separate_zval(zval *pzval);
void pthreads_store_tohash(zval *object, HashTable *hash);
int pthreads_store_shift(zval *object, zval *member);
int pthreads_store_chunk(zval *object, zend_long size, zend_bool preserve, zval *chunk);
int pthreads_store_pop(zval *object, zval *member);
int pthreads_store_count(zval *object, zend_long *count);
void pthreads_store_free(pthreads_store_t *store);
//int pthreads_store_unshift(zval *object, zval *args);
//int pthreads_store_clear(zval *object);

/* {{{ * iteration helpers */
void pthreads_store_reset(zval *object, HashPosition *position);
void pthreads_store_key(zval *object, zval *key, HashPosition *position);
void pthreads_store_data(zval *object, zval *value, HashPosition *position);
void pthreads_store_forward(zval *object, HashPosition *position); /* }}} */

pthreads_storage* pthreads_store_create(zval *pzval, zend_bool complex);
int pthreads_store_convert(pthreads_storage *storage, zval *pzval);
void pthreads_store_storage_dtor(pthreads_storage *element);

#endif
