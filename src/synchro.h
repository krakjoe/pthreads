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
#ifndef HAVE_PTHREADS_SYNCHRO_H
#define HAVE_PTHREADS_SYNCHRO_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct _pthreads_synchro {
	pthread_mutex_t wait;
	pthread_cond_t notify;
	pthread_cond_t hang;
	zend_bool waiting;
	zend_bool hanging;
} *pthreads_synchro;

/* {{{ allocate synchronization object */
pthreads_synchro pthreads_synchro_alloc(TSRMLS_D); /* }}} */

/* {{{ wait for notification */
int pthreads_synchro_wait_ex(pthreads_synchro sync, long timeout TSRMLS_DC); /* }}} */

/* {{{ wait for notification */
int pthreads_synchro_wait(pthreads_synchro sync TSRMLS_DC); /* }}} */

/* {{{ send notification */
int pthreads_synchro_notify(pthreads_synchro sync TSRMLS_DC); /* }}} */

/* {{{ free synchronization object */
void pthreads_synchro_free(pthreads_synchro sync TSRMLS_DC); /* }}} */

#endif
