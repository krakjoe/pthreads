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
#ifndef HAVE_PTHREADS_STACK_H
#define HAVE_PTHREADS_STACK_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct pthreads_stack_t pthreads_stack_t;
typedef struct pthreads_stack_item_t pthreads_stack_item_t;
typedef zend_bool (*pthreads_stack_running_function_t) (zval *value);
typedef zend_bool (*pthreads_stack_collect_function_t) (pthreads_call_t *call, zval *value);

pthreads_stack_t* pthreads_stack_alloc(pthreads_monitor_t *monitor);
zend_long pthreads_stack_size(pthreads_stack_t *stack);
void pthreads_stack_free(pthreads_stack_t *stack);
zend_long pthreads_stack_add(pthreads_stack_t *stack, zval *value);
zend_long pthreads_stack_del(pthreads_stack_t *stack, zval *value);
zend_long pthreads_stack_collect(pthreads_stack_t *stack, pthreads_call_t *call, pthreads_stack_running_function_t running, pthreads_stack_collect_function_t collect);
pthreads_monitor_state_t pthreads_stack_next(pthreads_stack_t *stack, zval *value, zend_object **running);

void pthreads_stack_tohash(pthreads_stack_t *stack, HashTable *hash);
#endif

