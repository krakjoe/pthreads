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
#ifndef HAVE_PHP_PTHREADS_H
#define HAVE_PHP_PTHREADS_H
#define PHP_PTHREADS_EXTNAME "pthreads"
#define PHP_PTHREADS_VERSION "3.1.0-dev"

PHP_MINIT_FUNCTION(pthreads);
PHP_MSHUTDOWN_FUNCTION(pthreads);
PHP_RINIT_FUNCTION(pthreads);
PHP_RSHUTDOWN_FUNCTION(pthreads);
PHP_MINFO_FUNCTION(pthreads);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pthreads);

#ifndef HAVE_PTHREADS_CLASS_THREADED_H
#	include <classes/threaded.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_THREAD_H
#	include <classes/thread.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_COLLECTABLE_H
#	include <classes/collectable.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_WORKER_H
#	include <classes/worker.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_POOL_H
#	include <classes/pool.h>
#endif

extern zend_module_entry pthreads_module_entry;
#define phpext_pthreads_ptr &pthreads_module_entry

#endif 
