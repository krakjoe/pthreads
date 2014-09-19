/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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
#ifndef HAVE_PTHREADS_H
#define HAVE_PTHREADS_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#ifndef _WIN32
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#else
#include <pthread.h>
#include <signal.h>
#endif

#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_ticks.h>
#include <ext/standard/info.h>
#include <ext/standard/basic_functions.h>
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_smart_str_public.h>
#include <ext/standard/php_var.h>
#ifdef HAVE_SPL
#include <ext/spl/spl_exceptions.h>
#include <ext/spl/spl_iterators.h>
#else
extern zend_class_entry *spl_ce_InvalidArgumentException;
extern zend_class_entry *spl_ce_Countable;
#endif

#include <Zend/zend.h>
#include <Zend/zend_closures.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_ts_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>

extern zend_class_entry *pthreads_threaded_entry;
extern zend_class_entry *pthreads_thread_entry;
extern zend_class_entry *pthreads_worker_entry;
extern zend_class_entry *pthreads_mutex_entry;
extern zend_class_entry *pthreads_condition_entry;

#ifndef IS_PTHREADS_CLASS
#define IS_PTHREADS_CLASS(c) \
	(instanceof_function(c, pthreads_threaded_entry TSRMLS_CC))
#endif

#ifndef IS_PTHREADS_OBJECT
#define IS_PTHREADS_OBJECT(o)   \
        (IS_PTHREADS_CLASS(Z_OBJCE_P(o)))
#endif

extern zend_object_handlers pthreads_handlers;
extern zend_object_handlers *zend_handlers;

extern struct _pthreads_globals pthreads_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads)

#ifndef PTHREADS_ZG
ZEND_BEGIN_MODULE_GLOBALS(pthreads)
	pid_t pid;
	int   signal;
	void *pointer;
	HashTable *resolve;
	HashTable *resources;
	HashTable *cache;
ZEND_END_MODULE_GLOBALS(pthreads)
#	define PTHREADS_ZG(v) TSRMG(pthreads_globals_id, zend_pthreads_globals *, v)
#   define PTHREADS_PID() PTHREADS_ZG(pid) ? PTHREADS_ZG(pid) : (PTHREADS_ZG(pid)=getpid())
#endif

#endif
