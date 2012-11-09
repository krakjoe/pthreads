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
#ifndef HAVE_PTHREADS_H
#define HAVE_PTHREADS_H

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <pthread.h>
#ifndef _WIN32
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/time.h>
#else
#include <win32/time.h>
#endif
#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_ticks.h>
#include <ext/standard/info.h>
#include <ext/standard/php_smart_str.h>
#include <ext/standard/php_smart_str_public.h>
#include <ext/standard/php_var.h>
#include <Zend/zend.h>
#include <Zend/zend_compile.h>
#include <Zend/zend_exceptions.h>
#include <Zend/zend_extensions.h>
#include <Zend/zend_globals.h>
#include <Zend/zend_hash.h>
#include <Zend/zend_interfaces.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>

#if defined(PTHREAD_MUTEX_ERRORCHECK)
#	define PTHREADS_LOCK_TYPE PTHREAD_MUTEX_ERRORCHECK
#elif defined(PTHREAD_MUTEX_ERRORCHECK_NP)
#	define PTHREADS_LOCK_TYPE PTHREAD_MUTEX_ERRORCHECK_NP
#elif defined(PTHREAD_MUTEX_NORMAL)
#	define PTHREADS_LOCK_TYPE PTHREAD_MUTEX_NORMAL
#elif defined(_WIN32)
#	define PTHREADS_LOCK_TYPE 0
#else
#	define PTHREADS_LOCK_TYPE NULL
#endif

#ifndef pthreads_thread_entry
zend_class_entry *pthreads_thread_entry;
#endif

#ifndef pthreads_worker_entry
zend_class_entry *pthreads_worker_entry;
#endif

#ifndef pthreads_stackable_entry
zend_class_entry *pthreads_stackable_entry;
#endif

#ifndef pthreads_mutex_entry
zend_class_entry *pthreads_mutex_entry;
#endif

#ifndef pthreads_condition_entry
zend_class_entry *pthreads_condition_entry;
#endif

#ifndef pthreads_handlers
zend_object_handlers pthreads_handlers;
#endif

#ifndef zend_handlers
zend_object_handlers *zend_handlers;
#endif

#endif
