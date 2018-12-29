/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2018                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Bastian Schneider <b.schneider@badnoob.com>                 |
  | Borrowed code from php-src                                           |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_STREAM_GLOBALS_H
#define HAVE_PTHREADS_STREAM_GLOBALS_H

/* {{{ pthreads_stream_globals */
struct _pthreads_stream_globals {
	pthreads_monitor_t *monitor;
	pthreads_stream_context_t *default_context;
	pthreads_hashtable xport_hash;
	pthreads_hashtable url_stream_wrappers_hash;
	pthreads_hashtable stream_filters_hash;
	pthreads_hashtable wrapper_errors;
	pthreads_hashtable user_filter_map;

	pthreads_stream_wrapper_t *plain_files_wrapper;
	pthreads_stream_wrapper_t *stream_php_wrapper;
	pthreads_stream_wrapper_t *stream_rfc2397_wrapper;
	pthreads_stream_wrapper_t *stream_http_wrapper;
	pthreads_stream_wrapper_t *stream_ftp_wrapper;
	pthreads_stream_wrapper_t *glob_stream_wrapper;

	/* filestat.c && main/streams/streams.c */
	char *CurrentStatFile, *CurrentLStatFile;
	pthreads_stream_statbuf ssb, lssb;

	ulong creatorId;
}; /* }}} */

extern struct _pthreads_stream_globals pthreads_stream_globals;

ZEND_EXTERN_MODULE_GLOBALS(pthreads_stream)

#define PTHREADS_GET_DEF_CONTEXT PTHREADS_STREAMG(default_context)

/* {{{ PTHREADS_STREAMG */
#define PTHREADS_STREAMG(v) pthreads_stream_globals.v
/* }}} */

/* {{{  */
int pthreads_stream_globals_is_main_context(); /* }}} */

/* {{{ initialize (true) file globals */
void pthreads_stream_globals_init(); /* }}} */

/* {{{  */
int pthreads_stream_globals_object_init(); /* }}} */

/* {{{  */
int pthreads_stream_globals_object_shutdown(); /* }}} */

int pthreads_is_default_context(pthreads_stream_context_t *threaded_context);

/* {{{  */
void pthreads_stream_globals_shutdown(); /* }}} */

/* {{{  */
zend_bool pthreads_stream_globals_lock(); /* }}} */

/* {{{  */
void pthreads_stream_globals_unlock(); /* }}} */

#define INIT_GLOBAL_WRAPPER(wrapper, ops, abst, isurl) do { \
	PTHREADS_FETCH_STREAMS_WRAPPER(PTHREADS_STREAMG(wrapper))->wops = &(ops); \
	PTHREADS_FETCH_STREAMS_WRAPPER(PTHREADS_STREAMG(wrapper))->abstract = (abst); \
	PTHREADS_FETCH_STREAMS_WRAPPER(PTHREADS_STREAMG(wrapper))->is_url = (isurl); \
} while(0)

#endif
