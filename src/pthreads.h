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
#include <win32/time.h>
#include <pthread.h>
#include <signal.h>
#endif

#include <php.h>
#include <php_globals.h>
#include <php_main.h>
#include <php_ticks.h>
#include <ext/standard/info.h>
#include <ext/standard/basic_functions.h>
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
#include <Zend/zend_inheritance.h>
#include <Zend/zend_list.h>
#include <Zend/zend_object_handlers.h>
#include <Zend/zend_smart_str.h>
#include <Zend/zend_variables.h>
#include <Zend/zend_vm.h>
#include <TSRM/TSRM.h>

#ifdef DMALLOC
#include <dmalloc.h>
#endif

extern zend_class_entry *pthreads_threaded_entry;
extern zend_class_entry *pthreads_volatile_entry;
extern zend_class_entry *pthreads_thread_entry;
extern zend_class_entry *pthreads_worker_entry;
extern zend_class_entry *pthreads_mutex_entry;
extern zend_class_entry *pthreads_condition_entry;

#ifndef IS_PTHREADS_CLASS
#define IS_PTHREADS_CLASS(c) \
	(instanceof_function(c, pthreads_threaded_entry))
#endif

#ifndef IS_PTHREADS_OBJECT
#define IS_PTHREADS_OBJECT(o)   \
        (Z_TYPE_P(o) == IS_OBJECT && IS_PTHREADS_CLASS(Z_OBJCE_P(o)))
#endif

#ifndef IS_PTHREADS_VOLATILE
#define IS_PTHREADS_VOLATILE(o)   \
        (Z_TYPE_P(o) == IS_OBJECT && instanceof_function(Z_OBJCE_P(o), pthreads_volatile_entry))
#endif

extern zend_object_handlers pthreads_handlers;
extern zend_object_handlers *zend_handlers;

extern struct _pthreads_globals pthreads_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads)

#ifndef PTHREADS_ZG
ZEND_BEGIN_MODULE_GLOBALS(pthreads)
	pid_t pid;
	int   signal;
	zval  this;
	HashTable resolve;
	HashTable *resources;
ZEND_END_MODULE_GLOBALS(pthreads)
#	define PTHREADS_ZG(v) TSRMG(pthreads_globals_id, zend_pthreads_globals *, v)
#   define PTHREADS_PID() PTHREADS_ZG(pid) ? PTHREADS_ZG(pid) : (PTHREADS_ZG(pid)=getpid())
#endif

#define PTHREADS_FETCH_ALL(ls, id, type) ((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])
#define PTHREADS_FETCH_CTX(ls, id, type, element) (((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define PTHREADS_CG(ls, v) PTHREADS_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define PTHREADS_CG_ALL(ls) PTHREADS_FETCH_ALL(ls, compiler_globals_id, zend_compiler_globals*)
#define PTHREADS_EG(ls, v) PTHREADS_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#define PTHREADS_SG(ls, v) PTHREADS_FETCH_CTX(ls, sapi_globals_id, sapi_globals_struct*, v)
#define PTHREADS_PG(ls, v) PTHREADS_FETCH_CTX(ls, core_globals_id, php_core_globals*, v)
#define PTHREADS_EG_ALL(ls) PTHREADS_FETCH_ALL(ls, executor_globals_id, zend_executor_globals*)

#define zend_string_new(s) zend_string_dup((s), GC_FLAGS((s)) & IS_STR_PERSISTENT)

/* {{{ */
static inline const zend_op* pthreads_check_opline(zend_execute_data *ex, zend_long offset, zend_uchar opcode) {
	if (ex && ex->func && ex->func->type == ZEND_USER_FUNCTION) {
		zend_op_array *ops = &ex->func->op_array;
		const zend_op *opline = ex->opline;

		if ((opline + offset) >= ops->opcodes) {
			opline += offset;
			if (opline->opcode == opcode) {
				return opline;			
			}
		}
	}
	return NULL;
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_check_opline_ex(zend_execute_data *ex, zend_long offset, zend_uchar opcode, uint32_t extended_value) {
	const zend_op *opline = pthreads_check_opline(ex, offset, opcode);
	if (opline && opline->extended_value == extended_value)
		return 1;
	return 0;
} /* }}} */

/* {{{ */
typedef struct _pthreads_call_t {
	zend_fcall_info fci;
	zend_fcall_info_cache fcc;
} pthreads_call_t; /* }}} */

#define PTHREADS_CALL_EMPTY {empty_fcall_info, empty_fcall_info_cache}

#ifndef HAVE_PTHREADS_MONITOR_H
#	include <src/monitor.h>
#endif

#ifndef HAVE_PTHREADS_STACK_H
#	include <src/stack.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#endif
