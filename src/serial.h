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
#ifndef HAVE_PTHREADS_SERIAL_H
#define HAVE_PTHREADS_SERIAL_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef ZEND_TS_HASH_H
#	include <Zend/zend_ts_hash.h>
#endif

#define PTHREADS_SERIAL_EMPTY 0
#define PTHREADS_SERIAL_INLINE 1
#define PTHREADS_SERIAL_LINE 2

/* {{{ serial buffer structure */
typedef struct _pthreads_serial {
	TsHashTable store;
	pthread_mutex_t lock;
	struct _pthreads_serial **line;
} *pthreads_serial; /* }}} */

/* {{{ allocate and initialize serial buffers */
pthreads_serial pthreads_serial_alloc(TSRMLS_D); /* }}} */

/* {{{ lock serial buffer */
int pthreads_serial_lock(pthreads_serial serial, int *acquired TSRMLS_DC); /* }}} */

/* {{{ extend serial line */
int pthreads_serial_extend(pthreads_serial serial, pthreads_serial line TSRMLS_DC); /* }}} */

/* {{{ tell if the serial buffer contains a specific value */
int pthreads_serial_contains(pthreads_serial serial, char *key, int keyl, pthreads_serial *line TSRMLS_DC); /* }}} */

/* {{{ delete a value from the serial buffer */
int pthreads_serial_delete(pthreads_serial serial, char *key, int keyl TSRMLS_DC); /* }}} */

/* {{{ read value from serial buffer */
int pthreads_serial_read(pthreads_serial serial, char *key, int keyl, zval **read TSRMLS_DC); /* }}} */

/* {{{ see if a value isset in serial buffer */
int pthreads_serial_isset(pthreads_serial serial, char *key, int keyl, int has_set_exists TSRMLS_DC); /* }}} */

/* {{{ write value to serial buffer */
int pthreads_serial_write(pthreads_serial serial, char *key, int keyl, zval **write TSRMLS_DC); /* }}} */

/* {{{ unlock serial buffer */
int pthreads_serial_unlock(pthreads_serial serial, int *acquired TSRMLS_DC); /* }}} */

/* {{{ free serial buffers */
void pthreads_serial_free(pthreads_serial serial TSRMLS_DC); /* }}} */

/* {{{ serialize instantce of a Thread, Worker or Stackable */
int pthreads_internal_serialize(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC); /* }}} */

/* {{{ unserialize an instance of a Thread, Worker or Stackable */
int pthreads_internal_unserialize(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC); /* }}} */
#endif
