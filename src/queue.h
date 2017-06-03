/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2015                                       |
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
#ifndef HAVE_PTHREADS_QUEUE_H
#define HAVE_PTHREADS_QUEUE_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct pthreads_queue_item_t pthreads_queue_item_t;
typedef struct pthreads_queue_t pthreads_queue_t;

/* {{{ fetches a PTHREAD from a specific object in the current context */
#define PTHREADS_QUEUE_FETCH_FROM(object) PTHREADS_FETCH_FROM(object)->store.queue /* }}} */

pthreads_queue_t* pthreads_queue_alloc(pthreads_monitor_t *monitor);
void pthreads_queue_free(pthreads_queue_t *queue);
int pthreads_queue_push(zval *object, zval *data);
int pthreads_queue_pop(zval *object, zval *data);
int pthreads_queue_shift(zval *object, zval *data);
int pthreads_queue_size(zval *object, zend_long *count);


#endif
