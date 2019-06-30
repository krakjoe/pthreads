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
#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

/* {{{ */
pthreads_object_t* pthreads_object_init(zend_class_entry *ce); /* }}} */

/* {{{ */
int pthreads_object_compare(pthreads_object_t* left, pthreads_object_t *right); /* }}} */

/* {{{ */
void pthreads_ptr_dtor(pthreads_object_t* threaded);
void pthreads_add_ref(pthreads_object_t* threaded);
void pthreads_del_ref(pthreads_object_t* threaded);
int pthreads_refcount(pthreads_object_t* threaded); /* }}} */

/* {{{ */
zend_object* pthreads_threaded_ctor(zend_class_entry *entry);
zend_object* pthreads_worker_ctor(zend_class_entry *entry);
zend_object* pthreads_thread_ctor(zend_class_entry *entry);
zend_object* pthreads_socket_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_context_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_filter_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_wrapper_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_bucket_ctor(zend_class_entry *entry);
zend_object* pthreads_stream_brigade_ctor(zend_class_entry *entry);
void         pthreads_base_free(zend_object *object);
zend_object* pthreads_base_clone(zval *object);
HashTable*   pthreads_base_gc(zval *object, zval **table, int *n); /* }}} */

/* {{{ */
int pthreads_threaded_unserialize(zval *object, zend_class_entry *ce, const unsigned char *buffer, size_t buflen, zend_unserialize_data *data);
int pthreads_threaded_serialize(zval *object, unsigned char **buffer, size_t *buflen, zend_serialize_data *data); /* }}} */

/* {{{ */
void pthreads_current_thread(zval *return_value); /* }}} */

/* {{{ */
zend_bool pthreads_start(pthreads_object_t* thread);
zend_bool pthreads_join(pthreads_object_t* thread); /* }}} */

/* {{{ */
int pthreads_connect(pthreads_object_t* source, pthreads_object_t* destination); /* }}} */

/* {{{ */
zend_object_iterator* pthreads_object_iterator_create(zend_class_entry *ce, zval *object, int by_ref); /* }}} */

/* {{{ */
#ifndef HAVE_PTHREADS_HANDLERS_H
#	include <src/handlers.h>
#endif /* }}} */

#endif /* HAVE_PTHREADS_OBJECT_H */
