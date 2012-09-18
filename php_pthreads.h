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
#define PHP_PTHREADS_VERSION "0.32"

#include <stdio.h>
#include <pthread.h>
#ifndef _WIN32
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/time.h>
#else
#include <time.h>
#endif
#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_ticks.h>
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_smart_str_public.h>
#include <ext/standard/php_var.h>
#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

PHP_MINIT_FUNCTION(pthreads);
PHP_MSHUTDOWN_FUNCTION(pthreads);
PHP_MINFO_FUNCTION(pthreads);

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
PHP_METHOD(Thread, isBusy);
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
/* }}} */

/* {{{ instance globals */
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

#define PTHREADS_FETCH_ALL(ls, id, type) ((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])
#define PTHREADS_FETCH_CTX(ls, id, type, element) (((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define PTHREADS_CG(ls, v) PTHREADS_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define PTHREADS_CG_ALL(ls) PTHREADS_FETCH_ALL(ls, compiler_globals_id, zend_compiler_globals*)
#define PTHREADS_EG(ls, v) PTHREADS_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#define PTHREADS_EG_ALL(ls) PTHREADS_FETCH_ALL(ls, executor_globals_id, zend_executor_globals*)

#define phpext_pthreads_ptr &pthreads_module_entry

#endif 
