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
#ifndef HAVE_PTHREADS_API_CONTEXT
#define HAVE_PTHREADS_API_CONTEXT

/* {{{ stream_context related functions */
static void pthreads_user_space_stream_notifier(pthreads_stream_context *context, int notifycode, int severity,
		char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr) {

	zval callback;
	zval retval;
	zval zvs[6];
	int i;

	pthreads_store_convert(context->notifier->ptr, &callback);

	if(Z_ISNULL(callback) || Z_ISUNDEF(callback)) {
		return;
	}

	ZVAL_LONG(&zvs[0], notifycode);
	ZVAL_LONG(&zvs[1], severity);
	if (xmsg) {
		ZVAL_STRING(&zvs[2], xmsg);
	} else {
		ZVAL_NULL(&zvs[2]);
	}
	ZVAL_LONG(&zvs[3], xcode);
	ZVAL_LONG(&zvs[4], bytes_sofar);
	ZVAL_LONG(&zvs[5], bytes_max);

	if (FAILURE == call_user_function_ex(EG(function_table), NULL, &callback, &retval, 6, zvs, 0, NULL)) {
		php_error_docref(NULL, E_WARNING, "failed to call user notifier");
	}
	for (i = 0; i < 6; i++) {
		zval_ptr_dtor(&zvs[i]);
	}
	zval_ptr_dtor(&retval);
}

static void pthreads_user_space_stream_notifier_dtor(pthreads_stream_notifier *notifier) {
	if (notifier && notifier->ptr) {
		pthreads_store_storage_dtor(notifier->ptr);
		notifier->ptr = NULL;
	}
}

static int pthreads_parse_context_options(pthreads_stream_context_t *threaded_context, zval *options) {
	zval *wval, *oval;
	zend_string *wkey, *okey;
	int ret = SUCCESS;

	if(MONITOR_LOCK(threaded_context)) {
		ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(options), wkey, wval) {
			ZVAL_DEREF(wval);
			if (wkey && Z_TYPE_P(wval) == IS_ARRAY) {
				ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(wval), okey, oval) {
					if (okey) {
						pthreads_stream_context_set_option(threaded_context, ZSTR_VAL(wkey), ZSTR_VAL(okey), oval);
					}
				} ZEND_HASH_FOREACH_END();
			} else {
				php_error_docref(NULL, E_WARNING, "options should have the form [\"wrappername\"][\"optionname\"] = $value");
			}
		} ZEND_HASH_FOREACH_END();

		MONITOR_UNLOCK(threaded_context);
	}
	return ret;
}

static int pthreads_parse_context_params(pthreads_stream_context_t *threaded_context, zval *params) {
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);
	int ret = SUCCESS;
	zval *tmp;

	if(MONITOR_LOCK(threaded_context)) {
		if (NULL != (tmp = zend_hash_str_find(Z_ARRVAL_P(params), "notification", sizeof("notification")-1))) {
			if (context->notifier) {
				pthreads_stream_notification_free(context->notifier);
				context->notifier = NULL;
			}

			context->notifier = pthreads_stream_notification_alloc();
			context->notifier->func = pthreads_user_space_stream_notifier;
			context->notifier->ptr = pthreads_store_create(tmp, 0);
			context->notifier->dtor = pthreads_user_space_stream_notifier_dtor;
		}

		if (NULL != (tmp = zend_hash_str_find(Z_ARRVAL_P(params), "options", sizeof("options")-1))) {
			if (Z_TYPE_P(tmp) == IS_ARRAY) {
				pthreads_parse_context_options(threaded_context, tmp);
			} else {
				php_error_docref(NULL, E_WARNING, "Invalid stream/context parameter");
			}
		}
		MONITOR_UNLOCK(threaded_context);
	}
	return ret;
}

/* {{{ */
void pthreads_streams_api_context_get_options(zval *object, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);

	ZVAL_NULL(return_value);

	if(MONITOR_LOCK(threaded_context)) {
		_pthreads_volatile_map_to_array(context->options, return_value);
		MONITOR_UNLOCK(threaded_context);
	}
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_set_option(zval *object, zend_string *wrapper, zend_string *option, zval *zvalue, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_BOOL(pthreads_stream_context_set_option(threaded_context, ZSTR_VAL(wrapper), ZSTR_VAL(option), zvalue) == SUCCESS);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_set_options(zval *object, zval *options, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_BOOL(pthreads_parse_context_options(threaded_context, options) == SUCCESS);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_set_params(zval *object, zval *params, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETVAL_BOOL(pthreads_parse_context_params(threaded_context, params) == SUCCESS);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_get_params(zval *object, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_context *context = PTHREADS_FETCH_STREAMS_CONTEXT(threaded_context);
	zval pzval;

	array_init(return_value);
	if (context->notifier && context->notifier->ptr && context->notifier->func == pthreads_user_space_stream_notifier
			&& pthreads_store_convert(context->notifier->ptr, &pzval) == SUCCESS) {
		add_assoc_zval_ex(return_value, "notification", sizeof("notification")-1, &pzval);
	}
	zval options;

	if(MONITOR_LOCK(threaded_context)) {
		_pthreads_volatile_map_to_array(context->options, &options);
		MONITOR_UNLOCK(threaded_context);
	}
	add_assoc_zval_ex(return_value, "options", sizeof("options")-1, &options);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_get_default(zval *options, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_STREAMG(default_context);

	if (options) {
		pthreads_parse_context_options(threaded_context, options);
	}
	pthreads_stream_context_to_zval(PTHREADS_STD_P(threaded_context), return_value);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_set_default(zval *options, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_STREAMG(default_context);

	pthreads_parse_context_options(threaded_context, options);

	pthreads_stream_context_to_zval(PTHREADS_STD_P(threaded_context), return_value);
}
/* }}} */

/* {{{ */
void pthreads_streams_api_context_create(zval *object, zval *options, zval *params, zval *return_value) {
	pthreads_stream_context_t *threaded_context = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (options) {
		pthreads_parse_context_options(threaded_context, options);
	}

	if (params) {
		pthreads_parse_context_params(threaded_context, params);
	}
}
/* }}} */

#endif
