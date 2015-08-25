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

typedef struct _pthreads_store_t {
	HashTable         table;
	pthreads_monitor_t *monitor;
	zend_ulong        next;
} pthreads_store_t;

pthreads_store_t* pthreads_store_alloc(pthreads_monitor_t *monitor);
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite);
int pthreads_store_delete(pthreads_store_t *store, zend_string *key);
int pthreads_store_read(pthreads_store_t *store, zend_string *key, zval *read);
zend_bool pthreads_store_isset(pthreads_store_t *store, zend_string *key, int has_set_exists);
int pthreads_store_write(pthreads_store_t *store, zend_string *key, zval *write);
int pthreads_store_separate(zval *pzval, zval *seperated, zend_bool complex);
void pthreads_store_separate_zval(zval *pzval);
void pthreads_store_tohash(pthreads_store_t *store, HashTable *hash);
void pthreads_store_keys(pthreads_store_t *store, HashTable *keys, HashPosition *position);
int pthreads_store_shift(zval *object, zval *member);
int pthreads_store_chunk(zval *object, zend_long size, zend_bool preserve, zval *chunk);
int pthreads_store_pop(zval *object, zval *member);
int pthreads_store_count(zval *object, zend_long *count);
void pthreads_store_free(pthreads_store_t *store);
#endif
