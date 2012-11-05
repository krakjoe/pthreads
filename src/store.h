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

#ifndef ZEND_TS_HASH_H
#	include <Zend/zend_ts_hash.h>
#endif

/* {{{ buffer structure */
typedef struct _pthreads_store {
	TsHashTable table;
	pthread_mutex_t lock;
	struct _pthreads_store **line;
} *pthreads_store; /* }}} */

/* {{{ allocate and initialize buffers */
pthreads_store pthreads_store_alloc(TSRMLS_D); /* }}} */

/* {{{ lock buffer */
int pthreads_store_lock(pthreads_store store, int *acquired TSRMLS_DC); /* }}} */

/* {{{ extend line */
int pthreads_store_extend(pthreads_store store, pthreads_store line TSRMLS_DC); /* }}} */

/* {{{ tell if the buffer contains a specific value */
int pthreads_store_contains(pthreads_store store, char *key, int keyl, pthreads_store *line TSRMLS_DC); /* }}} */

/* {{{ delete a value from the buffer */
int pthreads_store_delete(pthreads_store store, char *key, int keyl TSRMLS_DC); /* }}} */

/* {{{ read value from buffer */
int pthreads_store_read(pthreads_store store, char *key, int keyl, zval **read TSRMLS_DC); /* }}} */

/* {{{ see if a value isset in buffer */
int pthreads_store_isset(pthreads_store store, char *key, int keyl, int has_set_exists TSRMLS_DC); /* }}} */

/* {{{ write value to buffer */
int pthreads_store_write(pthreads_store store, char *key, int keyl, zval **write TSRMLS_DC); /* }}} */

/* {{{ unlock buffer */
int pthreads_store_unlock(pthreads_store store, int *acquired TSRMLS_DC); /* }}} */

/* {{{ copy a zval */
int pthreads_store_copy(zval *source, zval *destination TSRMLS_DC); /* }}} */

/* {{{ free buffers */
void pthreads_store_free(pthreads_store store TSRMLS_DC); /* }}} */

/* {{{ storeize instantce of a Thread, Worker or Stackable */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC); /* }}} */

/* {{{ unstoreize an instance of a Thread, Worker or Stackable */
int pthreads_internal_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC); /* }}} */
#endif
