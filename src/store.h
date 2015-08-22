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

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_MONITOR_H
#	include <src/monitor.h>
#endif

#ifndef IS_CALLABLE
# define IS_CALLABLE 10
#endif

#define IS_CLOSURE  (IS_CALLABLE)
#define IS_PTHREADS (IS_PTR + 1)

/* {{{ buffer structure */
typedef struct _pthreads_store {
	HashTable         table;
	pthreads_monitor_t *monitor;
	zend_ulong        next; /* idx of next anonymous member */
} *pthreads_store; /* }}} */

/* {{{ allocate and initialize buffers */
pthreads_store pthreads_store_alloc(pthreads_monitor_t *monitor); /* }}} */

/* {{{ merges the properties/elements of from into destination */
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite); /* }}} */

/* {{{ delete a value from the buffer */
int pthreads_store_delete(pthreads_store store, zend_string *key); /* }}} */

/* {{{ read value from buffer */
int pthreads_store_read(pthreads_store store, zend_string *key, zval *read); /* }}} */

/* {{{ see if a value isset in buffer */
zend_bool pthreads_store_isset(pthreads_store store, zend_string *key, int has_set_exists); /* }}} */

/* {{{ write value to buffer */
int pthreads_store_write(pthreads_store store, zend_string *key, zval *write); /* }}} */

/* {{{ separate a zval using internals */
int pthreads_store_separate(zval *pzval, zval *seperated, zend_bool complex); /* }}} */

/* {{{ */
void pthreads_store_separate_zval(zval *pzval); /* }}} */

/* {{{ copy store to hashtable */
void pthreads_store_tohash(pthreads_store store, HashTable *hash); /* }}} */

/* {{{ copy keys to hashtable */
void pthreads_store_keys(pthreads_store store, HashTable *keys, HashPosition *position); /* }}} */

/* {{{ store shift */
int pthreads_store_shift(zval *object, zval *member); /* }}} */

/* {{{ store chunk */
int pthreads_store_chunk(zval *object, long size, zend_bool preserve, zval *chunk); /* }}} */

/* {{{ store pop */
int pthreads_store_pop(zval *object, zval *member); /* }}} */

/* {{{ count properties */
int pthreads_store_count(zval *object, long *count); /* }}} */

/* {{{ free buffers */
void pthreads_store_free(pthreads_store store); /* }}} */
#endif
