/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                               		 |
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
#ifndef HAVE_PHP_PTHREADS_H
#define HAVE_PHP_PTHREADS_H
#define PHP_PTHREADS_EXTNAME "pthreads"
#define PHP_PTHREADS_VERSION "0.37-rc"

/* {{{ php internals */
PHP_MINIT_FUNCTION(pthreads);
PHP_MSHUTDOWN_FUNCTION(pthreads);
PHP_MINFO_FUNCTION(pthreads);
/* }}} */

/* {{{ basic */
PHP_METHOD(Thread, start); 
/* }}} */

/* {{{ synchronization */
PHP_METHOD(Thread, wait);
PHP_METHOD(Thread, notify);
PHP_METHOD(Thread, join);
/* }}} */

/* {{{ state detection */
PHP_METHOD(Thread, isStarted);
PHP_METHOD(Thread, isRunning);
PHP_METHOD(Thread, isJoined);
PHP_METHOD(Thread, isWaiting);
PHP_METHOD(Worker, isWorking);
/* }}} */

/* {{{ stacking */
PHP_METHOD(Thread, stack);
PHP_METHOD(Thread, unstack);
PHP_METHOD(Thread, getStacked);
/* }}} */

/* {{{ importing */
PHP_METHOD(Thread, getThread);
/* }}} */

/* {{{ identification */
PHP_METHOD(Thread, getThreadId);
PHP_METHOD(Thread, getCreatorId);
/* }}} */

/* {{{ globals */
PHP_METHOD(Thread, getCount);
PHP_METHOD(Thread, getMax);
PHP_METHOD(Thread, getPeak);
/* }}} */

/* {{{ mutex */
PHP_METHOD(Mutex, create);
PHP_METHOD(Mutex, lock);
PHP_METHOD(Mutex, trylock);
PHP_METHOD(Mutex, unlock);
PHP_METHOD(Mutex, destroy);
/* }}} */

/* {{{ conditions */
PHP_METHOD(Cond, create);
PHP_METHOD(Cond, signal);
PHP_METHOD(Cond, broadcast);
PHP_METHOD(Cond, wait);
PHP_METHOD(Cond, destroy);
/* }}} */

extern zend_module_entry pthreads_module_entry;
#define phpext_pthreads_ptr &pthreads_module_entry

#endif 
