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
#ifndef HAVE_PTHREADS_STREAM_GLOBALS
#define HAVE_PTHREADS_STREAM_GLOBALS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_GLOBALS_H
#	include <src/streams/streamglobals.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

struct _pthreads_stream_globals pthreads_stream_globals;

#ifndef PTHREADS_STREAMG
#	define PTHREADS_STREAMG () ?  : (void***) &pthreads_stream_globals
#endif

static void pthreads_filter_item_dtor(zval *zv) {
	struct pthreads_user_filter_data *fdat = Z_PTR_P(zv);
	free(fdat);
}

static void pthreads_wrapper_list_dtor(zval *item) {
	zend_llist *list = (zend_llist*)Z_PTR_P(item);
	zend_llist_destroy(list);
	pefree(list, 1);
}

static void pthreads_stream_wrapper_dtor(zval *item) {
	pthreads_stream_wrapper_t *threaded_wrapper = (pthreads_stream_wrapper_t*)Z_PTR_P(item);
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper * uwrap = wrapper->abstract;

	if(uwrap) {
		free(uwrap->protoname);
		zend_string_release(uwrap->classname);

		if(uwrap->threaded_wrapper) {
			pthreads_ptr_dtor(uwrap->threaded_wrapper);
		}
		free(uwrap);
	}
}

/* {{{ */
int pthreads_stream_globals_is_main_context() {
	return (!PTHREADS_STREAMG(creatorId) || PTHREADS_STREAMG(creatorId) == pthreads_self()) ? 1 : 0;
} /* }}} */

/* {{{ */
void pthreads_stream_globals_init() {
	PTHREADS_STREAMG(monitor) = pthreads_monitor_alloc();
	pthreads_hashtable_init(&PTHREADS_STREAMG(xport_hash), 8, NULL);
	pthreads_hashtable_init(&PTHREADS_STREAMG(url_stream_wrappers_hash), 8, pthreads_stream_wrapper_dtor);
	pthreads_hashtable_init(&PTHREADS_STREAMG(stream_filters_hash), 8, NULL);
	pthreads_hashtable_init(&PTHREADS_STREAMG(wrapper_errors), 8, pthreads_wrapper_list_dtor);
	pthreads_hashtable_init(&PTHREADS_STREAMG(user_filter_map), 8, pthreads_filter_item_dtor);

	PTHREADS_STREAMG(CurrentStatFile)  	= NULL;
	PTHREADS_STREAMG(CurrentLStatFile) 	= NULL;
	PTHREADS_STREAMG(default_context)	= NULL;

	PTHREADS_STREAMG(creatorId) = 0;
} /* }}} */


/* {{{ */
int pthreads_stream_globals_object_init() {
	if(PTHREADS_STREAMG(creatorId) == 0) {
		PTHREADS_STREAMG(creatorId)				 = pthreads_self();
		PTHREADS_STREAMG(default_context)        = pthreads_object_init(pthreads_stream_context_entry);

		PTHREADS_STREAMG(plain_files_wrapper)    = pthreads_object_init(pthreads_stream_wrapper_entry);
		PTHREADS_STREAMG(stream_php_wrapper)     = pthreads_object_init(pthreads_stream_wrapper_entry);
		PTHREADS_STREAMG(stream_rfc2397_wrapper) = pthreads_object_init(pthreads_stream_wrapper_entry);
		PTHREADS_STREAMG(stream_http_wrapper)    = pthreads_object_init(pthreads_stream_wrapper_entry);
		PTHREADS_STREAMG(stream_ftp_wrapper)     = pthreads_object_init(pthreads_stream_wrapper_entry);
		PTHREADS_STREAMG(glob_stream_wrapper)    = pthreads_object_init(pthreads_stream_wrapper_entry);

		return SUCCESS;
	}
	return FAILURE;
} /* }}} */

/* {{{ */
int pthreads_stream_globals_object_shutdown() {
	if(pthreads_stream_globals_is_main_context()) {
		pthreads_ptr_dtor(PTHREADS_STREAMG(glob_stream_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(stream_ftp_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(stream_http_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(stream_rfc2397_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(stream_php_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(plain_files_wrapper));
		pthreads_ptr_dtor(PTHREADS_STREAMG(default_context));

		pthreads_clear_hashtable(&PTHREADS_STREAMG(xport_hash));
		pthreads_clear_hashtable(&PTHREADS_STREAMG(url_stream_wrappers_hash));
		pthreads_clear_hashtable(&PTHREADS_STREAMG(stream_filters_hash));
		pthreads_clear_hashtable(&PTHREADS_STREAMG(wrapper_errors));
		pthreads_clear_hashtable(&PTHREADS_STREAMG(user_filter_map));

		PTHREADS_STREAMG(default_context)        = NULL;
		PTHREADS_STREAMG(plain_files_wrapper)    = NULL;
		PTHREADS_STREAMG(stream_php_wrapper)     = NULL;
		PTHREADS_STREAMG(stream_rfc2397_wrapper) = NULL;
		PTHREADS_STREAMG(stream_http_wrapper)    = NULL;
		PTHREADS_STREAMG(stream_ftp_wrapper)     = NULL;
		PTHREADS_STREAMG(glob_stream_wrapper)    = NULL;

		return SUCCESS;
	}
	return FAILURE;
} /* }}} */

int pthreads_is_default_context(pthreads_stream_context_t *threaded_context) {
	return pthreads_object_compare(threaded_context, PTHREADS_STREAMG(default_context)) == 0 ? 1 : 0;
}

/* {{{ */
void pthreads_stream_globals_shutdown() {
	pthreads_free_hashtable(&PTHREADS_STREAMG(url_stream_wrappers_hash));
	pthreads_free_hashtable(&PTHREADS_STREAMG(stream_filters_hash));
	pthreads_free_hashtable(&PTHREADS_STREAMG(wrapper_errors));
	pthreads_free_hashtable(&PTHREADS_STREAMG(user_filter_map));
	pthreads_free_hashtable(&PTHREADS_STREAMG(xport_hash));

	if (PTHREADS_STREAMG(CurrentStatFile)) {
		free(PTHREADS_STREAMG(CurrentStatFile));
	}

	if (PTHREADS_STREAMG(CurrentLStatFile)) {
		free(PTHREADS_STREAMG(CurrentLStatFile));
	}
	PTHREADS_STREAMG(CurrentStatFile) = NULL;
	PTHREADS_STREAMG(CurrentLStatFile) = NULL;

	pthreads_monitor_free(PTHREADS_STREAMG(monitor));
} /* }}} */

/* {{{ */
zend_bool pthreads_stream_globals_lock() {
	return pthreads_monitor_lock(PTHREADS_STREAMG(monitor));
} /* }}} */

/* {{{ */
void pthreads_stream_globals_unlock() {
	pthreads_monitor_unlock(PTHREADS_STREAMG(monitor));
} /* }}} */

#endif
