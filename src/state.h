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
#ifndef HAVE_PTHREADS_STATE_H
#define HAVE_PTHREADS_STATE_H

#define PTHREADS_ST_STARTED 1
#define PTHREADS_ST_RUNNING 2
#define PTHREADS_ST_WAITING	4
#define PTHREADS_ST_JOINED	8
#define PTHREADS_ST_CHECK(st, msk)	((st & msk)==msk)

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_SYNCHRO_H
#	include <src/synchro.h>
#endif

typedef struct _pthreads_state {
	pthread_mutex_t	lock;
	int				bits;
} *pthreads_state;

pthreads_state pthreads_state_alloc(int mask);
int pthreads_state_lock(pthreads_state state, int *acquired);
int pthreads_state_unlock(pthreads_state state, int *acquired);
void pthreads_state_free(pthreads_state state);
int pthreads_state_set(pthreads_state state, int mask);
int pthreads_state_isset(pthreads_state state, int mask);
int pthreads_state_unset(pthreads_state state, int mask);
#endif
