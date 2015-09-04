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

#define PTHREADS_STACK_GARBAGE 2
#define PTHREADS_STACK_FREE    1
#define PTHREADS_STACK_NOTHING 0

struct pthreads_stack_item_t {
	struct pthreads_stack_item_t *next;
	struct pthreads_stack_item_t *prev;
	zval value;
};

struct pthreads_stack_t {
	zend_long 				size;
	pthreads_monitor_t   	*monitor;
	struct pthreads_stack_t	*gc;
	pthreads_stack_item_t	*head;
	pthreads_stack_item_t	*tail;
};

pthreads_stack_t* pthreads_stack_alloc(pthreads_monitor_t *monitor) {
	pthreads_stack_t *stack = 
		(pthreads_stack_t*) ecalloc(1, sizeof(pthreads_stack_t));
	
	stack->monitor = monitor;
	stack->gc = (pthreads_stack_t*) ecalloc(1, sizeof(pthreads_stack_t));

	return stack;
}

zend_long pthreads_stack_size(pthreads_stack_t *stack) {
	zend_long size = 0;

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
			efree(r);
		}

		if (stack->gc) {
			item = stack->gc->head;		
			while (item) {
				pthreads_stack_item_t *r = item;
				zval_ptr_dtor(&item->value);
				item = r->next;
				efree(r);	
			}
		}

		efree(stack->gc);
		efree(stack);
	
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

zend_long pthreads_stack_add(pthreads_stack_t *stack, zval *value) {
	zend_long size = 0;
	pthreads_stack_item_t *item = 
			(pthreads_stack_item_t*) ecalloc(1, sizeof(pthreads_stack_item_t));

	ZVAL_COPY(&item->value, value);

	if (pthreads_monitor_lock(stack->monitor)) {
		size = stack->size;
		pthreads_stack_add_item(stack, item);
		if (!size) {
			pthreads_monitor_notify(stack->monitor);
		}
		size = stack->size;
		pthreads_monitor_unlock(stack->monitor);
	} else {
		zval_ptr_dtor(&item->value);
		efree(item);
		size = -1;
	}

	return size;
}

static inline zend_long pthreads_stack_remove(pthreads_stack_t *stack, pthreads_stack_item_t *item, zval *value, zend_bool garbage) {
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

	if (value) {
		memcpy(value, &item->value, sizeof(zval));
	}
	
	switch (garbage) {
		case PTHREADS_STACK_GARBAGE:
			pthreads_stack_add_item(stack->gc, item);
		break;

		case PTHREADS_STACK_FREE:
			efree(item);
		break;
	}

	return stack->size;
}

zend_long pthreads_stack_del(pthreads_stack_t *stack, zval *value) {
	zend_long size = 0;
	
	if (pthreads_monitor_lock(stack->monitor)) {
		size = pthreads_stack_remove(
			stack, stack->head, value, PTHREADS_STACK_FREE);
		pthreads_monitor_unlock(stack->monitor);
	}

	return size;
}

zend_long pthreads_stack_collect(pthreads_stack_t *stack, pthreads_call_t *call, pthreads_stack_collect_function_t collect) {
	zend_long size = 0;

	if (pthreads_monitor_lock(stack->monitor)) {
		pthreads_stack_item_t *item;

		if (stack->gc) {
			item = stack->gc->head;
			while (item) {
				pthreads_stack_item_t *garbage = item;

				if (collect(call, &garbage->value)) {
					pthreads_stack_remove(stack->gc, garbage, NULL, PTHREADS_STACK_NOTHING);
					item = garbage->next;
					zval_ptr_dtor(&garbage->value);
					efree(garbage);
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
				pthreads_stack_remove(stack, stack->head, value, PTHREADS_STACK_GARBAGE);
				break;
			}
		} while (state != PTHREADS_MONITOR_JOINED);
		
		pthreads_monitor_unlock(stack->monitor);
	}

	return state;
}

void pthreads_stack_tohash(pthreads_stack_t *stack, HashTable *hash) {
	zval stacked;
	zval waiting;
	zval gc;

	array_init(&stacked);
	array_init(&waiting);	
	array_init(&gc);

	zend_hash_str_add(Z_ARRVAL(stacked), ":stacked:", sizeof(":stacked:")-1, &waiting);
	zend_hash_str_add(Z_ARRVAL(stacked), ":gc:", sizeof(":gc:")-1, &gc);

	if (pthreads_monitor_lock(stack->monitor)) {
		pthreads_stack_item_t *item = stack->head;
		
		while (item) {
			if (add_next_index_zval(
					&waiting, &item->value)) {
				Z_ADDREF(item->value);
			}
			item = item->next;
		}

		item = stack->gc->head;
		while (item) {
			if (add_next_index_zval(
					&gc, &item->value)) {
				Z_ADDREF(item->value);
			}
			item = item->next;
		}
		pthreads_monitor_unlock(stack->monitor);
	}

	zend_hash_str_add(hash, ":stack:", sizeof(":stack:")-1, &stacked);
}
#endif

