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
#ifndef HAVE_PTHREADS_STACK
#define HAVE_PTHREADS_STACK

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

pthreads_stack_t* pthreads_stack_alloc(pthreads_monitor_t *monitor) {
	pthreads_stack_t *stack = (pthreads_stack_t*) calloc(1, sizeof(pthreads_stack_t));
	
	if (!stack) {
		return NULL;
	}
	
	stack->monitor = monitor;	
	stack->gc = (pthreads_stack_t*) calloc(1, sizeof(pthreads_stack_t));
	
	if (!stack->gc) {
		free(stack);
		return NULL;
	}

	return stack;
}

size_t pthreads_stack_size(pthreads_stack_t *stack) {
	size_t size = 0;

	if (pthreads_monitor_lock(stack->monitor)) {
		size = stack->size;
		pthreads_monitor_unlock(stack->monitor);
	}
	return size;
}

void pthreads_stack_free(pthreads_stack_t *stack) {
	pthreads_monitor_t *monitor = stack->monitor;

	if (pthreads_monitor_lock(monitor)) {
		pthreads_stack_item_t *item = stack->head;
		while (item) {
			pthreads_stack_item_t *r = item;
			zval_ptr_dtor(&item->value);
			item = r->next;
			free(r);
		}

		if (stack->gc) {
			item = stack->gc->head;		
			while (item) {
				pthreads_stack_item_t *r = item;
				zval_ptr_dtor(&item->value);
				item = r->next;
				free(r);			
			}
		}

		free(stack->gc);
		free(stack);
	
		pthreads_monitor_unlock(monitor);
	}
}

static inline void pthreads_stack_add_item(pthreads_stack_t *stack, pthreads_stack_item_t *item) {
	if (!stack->tail) {
		stack->tail = item;
		stack->head = item;
	} else {
		stack->tail->next = item;
		item->prev = stack->tail;
		stack->tail = item;
	}
	stack->size++;
}

size_t pthreads_stack_add(pthreads_stack_t *stack, zval *value) {
	size_t size = 0;

	if (pthreads_monitor_lock(stack->monitor)) {
		pthreads_stack_item_t *item = 
			(pthreads_stack_item_t*) calloc(1, sizeof(pthreads_stack_item_t));

		ZVAL_COPY(&item->value, value);
		pthreads_stack_add_item(stack, item);
		size = stack->size;

		pthreads_monitor_notify(stack->monitor);
		pthreads_monitor_unlock(stack->monitor);
	}

	return size;
}

static inline size_t pthreads_stack_remove(pthreads_stack_t *stack, pthreads_stack_item_t *item, zval *value, zend_bool garbage) {
	if (!item) {
		value = NULL;
		return 0;
	}

	if (stack->head == item && stack->tail == item) {
		stack->head = NULL;
		stack->tail = NULL;
	} else if (stack->head == item) {
		stack->head = item->next;
		stack->head->prev = NULL;
	} else if (stack->tail == item) {
		stack->tail = item->prev;
		stack->tail->next = NULL;
	} else {
		pthreads_stack_item_t *items[2] = 
			{item->next, item->prev};
		
		items[0]->prev = items[1];
		items[1]->next = items[0];
	}

	stack->size--;

	if (garbage) {
		pthreads_stack_add_item(stack->gc, item);
	}

	if (value) {
		memcpy(value, &item->value, sizeof(zval));
	}

	return stack->size;
}

size_t pthreads_stack_del(pthreads_stack_t *stack, zval *value) {
	size_t size = 0;
	
	if (pthreads_monitor_lock(stack->monitor)) {
		size = pthreads_stack_remove(
			stack, stack->head, value, 0);
		pthreads_monitor_unlock(stack->monitor);
	}

	return size;
}

size_t pthreads_stack_collect(pthreads_stack_t *stack, pthreads_call_t *call, pthreads_stack_collect_function_t collect) {
	size_t size = 0;

	if (pthreads_monitor_lock(stack->monitor)) {
		pthreads_stack_item_t *item;

		if (stack->gc) {
			item = stack->gc->head;
			while (item) {
				pthreads_stack_item_t *garbage = item;
				if (collect(call, &garbage->value)) {
					pthreads_stack_remove(stack->gc, garbage, NULL, 0);
					item = garbage->next;
					zval_ptr_dtor(&garbage->value);
					free(garbage);
				} else item = garbage->next;
			}
			size = stack->gc->size;
		}

		pthreads_monitor_unlock(stack->monitor);
	}

	return size;
}

pthreads_monitor_state_t pthreads_stack_next(pthreads_stack_t *stack, zval *value) {
	pthreads_monitor_state_t state = PTHREADS_MONITOR_RUNNING;
	if (pthreads_monitor_lock(stack->monitor)) {
		do {
			if (!stack->head) {
				if (pthreads_monitor_check(stack->monitor, PTHREADS_MONITOR_JOINED)) {
					state = PTHREADS_MONITOR_JOINED;
					break;
				}

				pthreads_monitor_wait(stack->monitor, 0);
			} else {
				pthreads_stack_remove(stack, stack->head, value, 1);
				break;
			}
		} while (state != PTHREADS_MONITOR_JOINED);
		pthreads_monitor_unlock(stack->monitor);
	}

	return state;
}
#endif

