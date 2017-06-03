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
#ifndef HAVE_PTHREADS_QUEUE
#define HAVE_PTHREADS_QUEUE

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

/* {{{ */
struct pthreads_queue_item_t {
    size_t                       size;
    char                         *data;
	struct pthreads_queue_item_t *next;
	struct pthreads_queue_item_t *prev;
}; /* }}} */

/* {{{ */
struct pthreads_queue_t {
	pthreads_monitor_t   	     *monitor;
    struct pthreads_queue_item_t *head;
    struct pthreads_queue_item_t *tail;
    zend_long size;
}; /* }}} */

pthreads_queue_t* pthreads_queue_alloc(pthreads_monitor_t *monitor) {
	pthreads_queue_t *queue = (pthreads_queue_t*) ecalloc(1, sizeof(pthreads_queue_t));

	queue->monitor = monitor;
	queue->size = 0;
	queue->head = NULL;
	queue->tail = NULL;

	return queue;
}

void pthreads_queue_free(pthreads_queue_t *queue) {
	if (pthreads_monitor_lock(queue->monitor)) {
		pthreads_queue_item_t *item = queue->head;

		while (item != NULL) {
			pthreads_queue_item_t *r = item;
			item = r->next;
			free(r->data);
			free(r);
		}

		efree(queue);
		pthreads_monitor_unlock(queue->monitor);
	}
}

/* {{{ */
int pthreads_queue_push(zval *object, zval *data) {
	int retval = FAILURE;
	pthreads_queue_t *queue = PTHREADS_QUEUE_FETCH_FROM(Z_OBJ_P(object));
	smart_str smart = {0};
	php_serialize_data_t vars;

	PHP_VAR_SERIALIZE_INIT(vars);
	php_var_serialize(&smart, data, &vars);
	PHP_VAR_SERIALIZE_DESTROY(vars);

	pthreads_queue_item_t *item = malloc(sizeof(pthreads_queue_item_t));
	item->data = malloc(ZSTR_LEN(smart.s));
	item->size = ZSTR_LEN(smart.s);
	item->next = NULL;

	memcpy(item->data, ZSTR_VAL(smart.s), item->size);
	/* free string */
	smart_str_free(&smart);

	if (pthreads_monitor_lock(queue->monitor)) {
		if (queue->size == 0) {
			queue->tail = item;
			queue->head = item;
		} else {
			queue->tail->next = item;
			item->prev = queue->tail;
			queue->tail = item;
		}
		queue->size++;

		pthreads_monitor_unlock(queue->monitor);
		retval = SUCCESS;
	} else {
		free(item->data);
		free(item);
	}
	return retval;
} /* }}} */

/* {{{ */
int pthreads_queue_pop(zval *object, zval *data) {
	int retval = FAILURE;
	pthreads_queue_t *queue = PTHREADS_QUEUE_FETCH_FROM(Z_OBJ_P(object));
	php_unserialize_data_t var_hash;

	if (pthreads_monitor_lock(queue->monitor)) {
		if (queue->size > 0) {
			pthreads_queue_item_t *item = queue->tail;
			const unsigned char* pointer = (const unsigned char*) item->data;

			PHP_VAR_UNSERIALIZE_INIT(var_hash);
			if (php_var_unserialize(data, &pointer, pointer + item->size, &var_hash) == 1) {
				if (queue->size == 1) {
					queue->tail = NULL;
					queue->head = NULL;
				} else {
					queue->tail = item->prev;
					queue->tail->next = NULL;
				}
				queue->size--;

				free(item->data);
				free(item);

				retval = SUCCESS;
			}
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		}
		pthreads_monitor_unlock(queue->monitor);
	}
	if (retval == FAILURE) {
		ZVAL_NULL(data);
	}
	return retval;
} /* }}} */

/* {{{ */
int pthreads_queue_shift(zval *object, zval *data) {
	int retval = FAILURE;
	pthreads_queue_t *queue = PTHREADS_QUEUE_FETCH_FROM(Z_OBJ_P(object));
	php_unserialize_data_t var_hash;

	if (pthreads_monitor_lock(queue->monitor)) {
		if (queue->size > 0) {
			pthreads_queue_item_t *item = queue->head;
			const unsigned char* pointer = (const unsigned char*) item->data;

			PHP_VAR_UNSERIALIZE_INIT(var_hash);
			if (php_var_unserialize(data, &pointer, pointer + item->size, &var_hash) == 1) {
				if (queue->size == 1) {
					queue->tail = NULL;
					queue->head = NULL;
				} else {
					queue->head = item->next;
					queue->head->prev = NULL;
				}
				queue->size--;

				free(item->data);
				free(item);

				retval = SUCCESS;
			}
			PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
		}
		pthreads_monitor_unlock(queue->monitor);
	}
	if (retval == FAILURE) {
		ZVAL_NULL(data);
	}
	return retval;
} /* }}} */

/* {{{ */
int pthreads_queue_size(zval *object, zend_long *count) {
	pthreads_queue_t *queue = PTHREADS_QUEUE_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_monitor_lock(queue->monitor)) {
		(*count) = queue->size;
		pthreads_monitor_unlock(queue->monitor);
	} else (*count) = 0L;

	return SUCCESS;
} /* }}} */

#endif
