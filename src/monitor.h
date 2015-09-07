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
#ifndef HAVE_PTHREADS_MONITOR_H
#define HAVE_PTHREADS_MONITOR_H

typedef unsigned long pthreads_monitor_state_t;

typedef struct _pthreads_monitor_t pthreads_monitor_t;

#define PTHREADS_MONITOR_NOTHING (0)
#define PTHREADS_MONITOR_READY   (1<<0)
#define PTHREADS_MONITOR_STARTED (1<<1)
#define PTHREADS_MONITOR_RUNNING (1<<2)
#define PTHREADS_MONITOR_JOINED  (1<<3)
#define PTHREADS_MONITOR_ERROR	 (1<<4)

pthreads_monitor_t* pthreads_monitor_alloc();
zend_bool pthreads_monitor_lock(pthreads_monitor_t *m);
zend_bool pthreads_monitor_unlock(pthreads_monitor_t *m);
pthreads_monitor_state_t pthreads_monitor_check(pthreads_monitor_t *m, pthreads_monitor_state_t state);
int pthreads_monitor_wait(pthreads_monitor_t *m, long timeout);
int pthreads_monitor_notify(pthreads_monitor_t *m);
void pthreads_monitor_wait_until(pthreads_monitor_t *m, pthreads_monitor_state_t state);
void pthreads_monitor_add(pthreads_monitor_t *m, pthreads_monitor_state_t state);
void pthreads_monitor_remove(pthreads_monitor_t *m, pthreads_monitor_state_t state);
void pthreads_monitor_free(pthreads_monitor_t *m);
#endif
