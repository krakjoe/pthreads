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
#ifndef HAVE_PTHREADS_STORE_H
#define HAVE_PTHREADS_STORE_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

/* {{{ buffer structure */
typedef struct _pthreads_store {
	HashTable         table;
	pthreads_lock     lock;
	zend_ulong        next; /* idx of next anonymous member */
} *pthreads_store; /* }}} */

/* {{{ allocate and initialize buffers */
pthreads_store pthreads_store_alloc(TSRMLS_D); /* }}} */

/* {{{ lock storage, userland only */
zend_bool pthreads_store_lock(zval *this_ptr TSRMLS_DC); /* }}} */

/* {{{ unlock storage, userland only */
zend_bool pthreads_store_unlock(zval *this_ptr TSRMLS_DC); /* }}} */

/* {{{ merges the properties/elements of from into destination */
int pthreads_store_merge(zval *destination, zval *from, zend_bool overwrite TSRMLS_DC); /* }}} */

/* {{{ delete a value from the buffer */
int pthreads_store_delete(pthreads_store store, char *key, int keyl TSRMLS_DC); /* }}} */

/* {{{ read value from buffer */
int pthreads_store_read(pthreads_store store, char *key, int keyl, zval **read TSRMLS_DC); /* }}} */

/* {{{ see if a value isset in buffer */
zend_bool pthreads_store_isset(pthreads_store store, char *key, int keyl, int has_set_exists TSRMLS_DC); /* }}} */

/* {{{ write value to buffer */
int pthreads_store_write(pthreads_store store, char *key, int keyl, zval **write TSRMLS_DC); /* }}} */

/* {{{ separate a zval using internals */
int pthreads_store_separate(zval * pzval, zval **seperated, zend_bool allocate, zend_bool complex TSRMLS_DC); /* }}} */

/* {{{ copy store to hashtable */
void pthreads_store_tohash(pthreads_store store, HashTable *hash TSRMLS_DC); /* }}} */

/* {{{ count properties */
int pthreads_store_count(zval *object, long *count TSRMLS_DC); /* }}} */

/* {{{ free buffers */
void pthreads_store_free(pthreads_store store TSRMLS_DC); /* }}} */
#endif
