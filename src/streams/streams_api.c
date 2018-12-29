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
#ifndef HAVE_PTHREADS_STREAMS_STREAMS_API
#define HAVE_PTHREADS_STREAMS_STREAMS_API

#ifndef PHP_WIN32
#define php_select(m, r, w, e, t)	select(m, r, w, e, t)
typedef unsigned long long pthreads_timeout_ull;
#else
#include "win32/select.h"
#include "win32/sockets.h"
#include "win32/console.h"
typedef unsigned __int64 pthreads_timeout_ull;
#endif

/**
 * Streams
 */

/* {{{ pthreads_streams_api_get_filters */
void pthreads_streams_api_get_filters(zval *return_value) {
	zend_string *filter_name;
	pthreads_hashtable *filter_hash;

	array_init(return_value);

	filter_hash = pthreads_get_stream_filters_hash();

	if(MONITOR_LOCK(filter_hash)) {
		ZEND_HASH_FOREACH_STR_KEY(&filter_hash->ht, filter_name) {
			if (filter_name) {
				add_next_index_str(return_value, zend_string_dup(filter_name, 1));
			}
		} ZEND_HASH_FOREACH_END();

		MONITOR_UNLOCK(filter_hash);
	}
	/* It's okay to return an empty array if no filters are registered */
}
/* }}} */


/* {{{ pthreads_streams_api_get_transports */
void pthreads_streams_api_get_transports(zval *return_value) {
	pthreads_hashtable *xport_hash;
	zend_string *stream_xport;

	array_init(return_value);

	xport_hash = pthreads_stream_xport_get_hash();

	if(MONITOR_LOCK(xport_hash)) {
		ZEND_HASH_FOREACH_STR_KEY(&xport_hash->ht, stream_xport) {
			if(stream_xport) {
				add_next_index_str(return_value, zend_string_copy(stream_xport));
			}
		} ZEND_HASH_FOREACH_END();

		MONITOR_UNLOCK(xport_hash);
	}
	/* It's okay to return an empty array if no transports are registered */
}
/* }}} */

/* {{{ pthreads_streams_api_get_wrappers */
void pthreads_streams_api_get_wrappers(zval *return_value) {
	pthreads_hashtable *url_stream_wrappers_hash;
	zend_string *stream_protocol;

	array_init(return_value);

	url_stream_wrappers_hash = pthreads_stream_get_url_stream_wrappers_hash();

	if(MONITOR_LOCK(url_stream_wrappers_hash)) {
		ZEND_HASH_FOREACH_STR_KEY(&url_stream_wrappers_hash->ht, stream_protocol) {
			if (stream_protocol) {
				add_next_index_str(return_value, zend_string_copy(stream_protocol));
			}
		} ZEND_HASH_FOREACH_END();

		MONITOR_UNLOCK(url_stream_wrappers_hash);
	}
	/* It's okay to return an empty array if no wrappers are registered */
}
/* }}} */

/* {{{ pthreads_streams_api_register_wrapper */
void pthreads_streams_api_register_wrapper(zend_string *protocol, zend_string *classname, zend_long flags, zval *return_value) {
	struct pthreads_user_stream_wrapper * uwrap;
	pthreads_stream_wrapper *wrapper;
	zend_class_entry *ce;

	if ((ce = zend_lookup_class(classname)) != NULL) {
		if(!instanceof_function(ce, pthreads_threaded_entry)) {
			php_error_docref(NULL, E_WARNING,
								"user-wrapper \"%s\" must be an instance of Threaded",
								ZSTR_VAL(classname));

			RETURN_FALSE;
		}

		uwrap = (struct pthreads_user_stream_wrapper *)calloc(1, sizeof(*uwrap));
		uwrap->protoname = strndup(ZSTR_VAL(protocol), ZSTR_LEN(protocol));
		uwrap->classname = zend_string_init(ZSTR_VAL(classname), ZSTR_LEN(classname), 1);
		uwrap->threaded_wrapper = pthreads_stream_wrapper_new();

		wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(uwrap->threaded_wrapper);
		wrapper->wops = &pthreads_user_stream_wops;
		wrapper->abstract = uwrap;
		wrapper->is_url = ((flags & PTHREADS_STREAM_IS_URL) != 0);

		if (pthreads_register_url_stream_wrapper(ZSTR_VAL(protocol), uwrap->threaded_wrapper) == SUCCESS) {
			RETURN_TRUE;
		} else {
			pthreads_hashtable *url_stream_wrappers_hash = &PTHREADS_STREAMG(url_stream_wrappers_hash);
			int protocol_exist = 0;

			if(MONITOR_LOCK(url_stream_wrappers_hash)) {
				protocol_exist = zend_hash_exists(&url_stream_wrappers_hash->ht, protocol);
				MONITOR_UNLOCK(url_stream_wrappers_hash);
			}

			/* We failed.  But why? */
			if (protocol_exist) {
				php_error_docref(NULL, E_WARNING, "Protocol %s:// is already defined.", ZSTR_VAL(protocol));
			} else {
				/* Hash doesn't exist so it must have been an invalid protocol scheme */
				php_error_docref(NULL, E_WARNING, "Invalid protocol scheme specified. Unable to register wrapper class %s to %s://", ZSTR_VAL(classname), ZSTR_VAL(protocol));
			}
		}
		pthreads_ptr_dtor(uwrap->threaded_wrapper);
		free(uwrap);
	} else {
		php_error_docref(NULL, E_WARNING, "class '%s' is undefined", ZSTR_VAL(classname));
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ pthreads_streams_api_unregister_wrapper */
void pthreads_streams_api_unregister_wrapper(zend_string *protocol, zval *return_value) {
	if (pthreads_unregister_url_stream_wrapper(ZSTR_VAL(protocol)) == FAILURE) {
		/* We failed */
		php_error_docref(NULL, E_WARNING, "Unable to unregister protocol %s://", ZSTR_VAL(protocol));
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ pthreads_streams_api_register_filter */
void pthreads_streams_api_register_filter(zend_string *filtername, zend_string *classname, zval *return_value) {
	pthreads_hashtable *stream_filters_hash;

	filtername = zend_string_init_interned(ZSTR_VAL(filtername), ZSTR_LEN(filtername), 1);
	classname = zend_string_init_interned(ZSTR_VAL(classname), ZSTR_LEN(classname), 1);

	int added = pthreads_streams_add_user_filter_map_entry(filtername, classname);

	RETVAL_FALSE;

	stream_filters_hash = &PTHREADS_STREAMG(stream_filters_hash);

	if(added == SUCCESS && MONITOR_LOCK(stream_filters_hash)) {
		if (pthreads_stream_filter_register_factory(ZSTR_VAL(filtername), &pthreads_user_filter_factory) == SUCCESS) {
			RETVAL_TRUE;
		} else {
			pthreads_streams_drop_user_filter_map_entry(filtername);
		}
		MONITOR_UNLOCK(stream_filters_hash);
	}
}
/* }}} */

/**
 * Stream
 */

/* {{{ pthreads_streams_api_stream_copy_to_stream */
void pthreads_streams_api_stream_copy_to_stream(zval *object, zval *dest, zend_long maxlen, zend_long offset, zval *return_value) {
	pthreads_stream_t *threaded_src = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_t *threaded_dest = PTHREADS_FETCH_FROM(Z_OBJ_P(dest));
	size_t len;
	int ret;

	if (offset > 0 && pthreads_stream_seek(threaded_src, offset, SEEK_SET) < 0) {
		php_error_docref(NULL, E_WARNING, "Failed to seek to position " ZEND_LONG_FMT " in the stream", offset);
		RETURN_LONG(-1);
	}
	ret = pthreads_stream_copy_to_stream_ex(threaded_src, threaded_dest, maxlen, &len);

	if (ret != SUCCESS) {
		RETURN_LONG(-1);
	}
	RETURN_LONG(len);
}
/* }}} */

/* {{{ pthreads_streams_api_stream_apply_filter_to_stream */
void pthreads_streams_api_stream_apply_filter_to_stream(int append, zval *object, zend_string *filtername, zend_long read_write, zval *params, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_filter_t *threaded_filter = NULL;
	int ret;
	if ((read_write & PTHREADS_STREAM_FILTER_ALL) == 0) {
		/* Chain not specified.
		 * Examine stream->mode to determine which filters are needed
		 * There's no harm in attaching a filter to an unused chain,
		 * but why waste the memory and clock cycles?
		 */
		if (strchr(stream->mode, 'r') || strchr(stream->mode, '+')) {
			read_write |= PTHREADS_STREAM_FILTER_READ;
		}
		if (strchr(stream->mode, 'w') || strchr(stream->mode, '+') || strchr(stream->mode, 'a')) {
			read_write |= PTHREADS_STREAM_FILTER_WRITE;
		}
	}

	if (read_write & PTHREADS_STREAM_FILTER_READ) {
		threaded_filter = pthreads_stream_filter_create(ZSTR_VAL(filtername), params);
		if (threaded_filter == NULL) {
			RETURN_NULL();
		}

		if (append) {
			ret = pthreads_stream_filter_append_ex(pthreads_stream_get_readfilters(threaded_stream), threaded_filter);
		} else {
			ret = pthreads_stream_filter_prepend_ex(pthreads_stream_get_readfilters(threaded_stream), threaded_filter);
		}
		if (ret != SUCCESS) {
			pthreads_stream_filter_remove(threaded_filter);
			RETURN_NULL();
		}
	}

	if (read_write & PTHREADS_STREAM_FILTER_WRITE) {
		threaded_filter = pthreads_stream_filter_create(ZSTR_VAL(filtername), params);
		if (threaded_filter == NULL) {
			RETURN_NULL();
		}

		if (append) {
			ret = pthreads_stream_filter_append_ex(pthreads_stream_get_writefilters(threaded_stream), threaded_filter);
		} else {
			ret = pthreads_stream_filter_prepend_ex(pthreads_stream_get_writefilters(threaded_stream), threaded_filter);
		}
		if (ret != SUCCESS) {
			pthreads_stream_filter_remove(threaded_filter);
			RETURN_NULL();
		}
	}

	if (threaded_filter) {
		RETURN_OBJ(PTHREADS_STD_P(threaded_filter));
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ proto int stream_socket_enable_crypto(resource stream, bool enable [, int cryptokind [, resource sessionstream]])
   Enable or disable a specific kind of crypto on the stream */
void pthreads_streams_api_stream_enable_crypto(zval *object, zend_bool enable, zend_long cryptokind, zval *zsessstream, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_t *threaded_session_stream = NULL;
	int ret;

	if (enable) {
		if (zsessstream) {
			threaded_session_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(zsessstream));
		}

		if (pthreads_stream_xport_crypto_setup(threaded_stream, cryptokind, threaded_session_stream) < 0) {
			RETURN_FALSE;
		}
	}

	ret = pthreads_stream_xport_crypto_enable(threaded_stream, enable);
	switch (ret) {
		case -1:
			RETURN_FALSE;

		case 0:
			RETURN_LONG(0);

		default:
			RETURN_TRUE;
	}
}
/* }}} */

/* {{{ pthreads_streams_api_stream_get_contents */
void pthreads_streams_api_stream_get_contents(zval *object, zend_long maxlen, zend_long desiredpos, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *contents;

	if (desiredpos >= 0) {
		int		seek_res = 0;
		zend_off_t	position;

		position = pthreads_stream_tell(threaded_stream);
		if (position >= 0 && desiredpos > position) {
			/* use SEEK_CUR to allow emulation in streams that don't support seeking */
			seek_res = pthreads_stream_seek(threaded_stream, desiredpos - position, SEEK_CUR);
		} else if (desiredpos < position)  {
			/* desired position before position or error on tell */
			seek_res = pthreads_stream_seek(threaded_stream, desiredpos, SEEK_SET);
		}

		if (seek_res != 0) {
			php_error_docref(NULL, E_WARNING,
				"Failed to seek to position " ZEND_LONG_FMT " in the stream", desiredpos);
			RETURN_NULL();
		}
	}

	if (maxlen > INT_MAX) {
		php_error_docref(NULL, E_WARNING, "maxlen truncated from " ZEND_LONG_FMT " to %d bytes", maxlen, INT_MAX);
		maxlen = INT_MAX;
	}
	if ((contents = pthreads_stream_copy_to_mem(threaded_stream, maxlen, 0))) {
		RETURN_STR(contents);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ pthreads_streams_api_stream_get_line */
void pthreads_streams_api_stream_get_line(zval *object, zend_long max_length, zend_string *ending, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *buf;
	char *str = NULL;
	size_t str_len = 0;

	if (!max_length) {
		max_length = PHP_SOCK_CHUNK_SIZE;
	}

	if(ending) {
		str = ZSTR_VAL(ending);
		str_len = ZSTR_LEN(ending);
	}

	if ((buf = pthreads_stream_get_record(threaded_stream, max_length, str, str_len))) {
		RETURN_STR(buf);
	} else {
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ pthreads_streams_api_stream_get_meta_data */
void pthreads_streams_api_stream_get_meta_data(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

	if (PTHREADS_IS_INVALID_STREAM(stream)) {
		php_error_docref(NULL, E_WARNING, "%s was already closed", ZSTR_VAL(Z_OBJCE_P(object)->name));
		RETURN_NULL();
	}
	array_init(return_value);

	if (!pthreads_stream_populate_meta_data(threaded_stream, return_value)) {
		add_assoc_bool(return_value, "timed_out", 0);
		add_assoc_bool(return_value, "blocked", 1);
		add_assoc_bool(return_value, "eof", pthreads_stream_eof(threaded_stream));
	}

	if(stream_lock(threaded_stream)) {

		pthreads_object_t *threaded_val = pthreads_stream_get_wrapperdata(threaded_stream);

		if (threaded_val != NULL) {
			zval wval, obj;

			ZVAL_OBJ(&obj, PTHREADS_STD_P(threaded_val));

			if(instanceof_function(Z_OBJCE(obj), pthreads_volatile_map_entry)) {
				_pthreads_volatile_map_to_array(threaded_val, &wval);
			} else {
				ZVAL_OBJ(&wval, PTHREADS_STD_P(threaded_val));
			}

			add_assoc_zval(return_value, "wrapper_data", &wval);
		}
		if (stream->wrapper) {
			add_assoc_string(return_value, "wrapper_type", (char *)PTHREADS_FETCH_STREAMS_WRAPPER(stream->wrapper)->wops->label);
		}
		add_assoc_string(return_value, "stream_type", (char *)stream->ops->label);

		add_assoc_string(return_value, "mode", stream->mode);

		if (pthreads_stream_is_filtered(threaded_stream)) {
			pthreads_stream_filter_t *threaded_filter;

			zval newval;
			array_init(&newval);

			for (threaded_filter = pthreads_chain_get_head(pthreads_stream_get_readfilters(threaded_stream)); threaded_filter; threaded_filter = pthreads_filter_get_next(threaded_filter)) {
				add_next_index_string(&newval, (char *)PTHREADS_FETCH_STREAMS_FILTER(threaded_filter)->fops->label);
			}

			for (threaded_filter = pthreads_chain_get_head(pthreads_stream_get_writefilters(threaded_stream)); threaded_filter; threaded_filter = pthreads_filter_get_next(threaded_filter)) {
				add_next_index_string(&newval, (char *)PTHREADS_FETCH_STREAMS_FILTER(threaded_filter)->fops->label);
			}

			add_assoc_zval(return_value, "filters", &newval);
		}

		add_assoc_long(return_value, "unread_bytes", stream->writepos - stream->readpos);

		add_assoc_bool(return_value, "seekable", (stream->ops->seek) && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0);
		if (stream->orig_path) {
			add_assoc_string(return_value, "uri", stream->orig_path);
		}
		stream_unlock(threaded_stream);
	}
}
/* }}} */


/* {{{ pthreads_streams_api_stream_is_local */
void pthreads_streams_api_stream_is_local(zval *object, zval *stream_or_url, zval *return_value) {
	pthreads_stream_t *threaded_stream;
	pthreads_stream *stream;

	pthreads_stream_wrapper_t *threaded_wrapper = NULL;

	if (Z_TYPE_P(stream_or_url) == IS_OBJECT) {
		threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(stream_or_url));
		if (threaded_stream == NULL) {
			RETURN_FALSE;
		}
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
		threaded_wrapper = stream->wrapper;
	} else {
		convert_to_string_ex(stream_or_url);

		threaded_wrapper = pthreads_stream_locate_url_wrapper(Z_STRVAL_P(stream_or_url), NULL, 0);
	}

	if (!threaded_wrapper) {
		RETURN_FALSE;
	}

	RETURN_BOOL(PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper)->is_url == 0);
}
/* }}} */

/* {{{ pthreads_streams_api_stream_isatty */
void pthreads_streams_api_stream_isatty(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	php_socket_t fileno;

	if (pthreads_stream_can_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT) == SUCCESS) {
		pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT, (void*)&fileno, 0);
	} else if (pthreads_stream_can_cast(threaded_stream, PTHREADS_STREAM_AS_FD) == SUCCESS) {
		pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD, (void*)&fileno, 0);
	} else {
		RETURN_FALSE;
	}

#ifdef PHP_WIN32
	/* Check if the Windows standard handle is redirected to file */
	RETVAL_BOOL(php_win32_console_fileno_is_console(fileno));
#elif HAVE_UNISTD_H
	/* Check if the file descriptor identifier is a terminal */
	RETVAL_BOOL(isatty(fileno));
#else
	{
		zend_stat_t stat = {0};
		RETVAL_BOOL(zend_fstat(fileno, &stat) == 0 && (stat.st_mode & /*S_IFMT*/0170000) == /*S_IFCHR*/0020000);
	}
#endif
}
/* }}} */


#ifdef PHP_WIN32
/* {{{ pthreads_streams_api_stream_windows_vt100_support
   Get or set VT100 support for the specified stream associated to an
   output buffer of a Windows console.
*/
void pthreads_streams_api_stream_windows_vt100_support(int argc, zval *object, zend_bool enable, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_long fileno;

	if (pthreads_stream_can_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT) == SUCCESS) {
		pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT, (void*)&fileno, 0);
	} else if (pthreads_stream_can_cast(threaded_stream, PTHREADS_STREAM_AS_FD) == SUCCESS) {
		pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD, (void*)&fileno, 0);
	}
	else {
		zend_internal_type_error(
			ZEND_ARG_USES_STRICT_TYPES(),
			"%s() was not able to analyze the specified stream",
			get_active_function_name()
		);
		RETURN_FALSE;
	}

	/* Check if the file descriptor is a console */
	if (!php_win32_console_fileno_is_console(fileno)) {
		RETURN_FALSE;
	}

	if (argc == 1) {
		/* Check if the Windows standard handle has VT100 control codes enabled */
		if (php_win32_console_fileno_has_vt100(fileno)) {
			RETURN_TRUE;
		}
		else {
			RETURN_FALSE;
		}
	}
	else {
		/* Enable/disable VT100 control codes support for the specified Windows standard handle */
		if (php_win32_console_fileno_set_vt100(fileno, enable ? TRUE : FALSE)) {
			RETURN_TRUE;
		}
		else {
			RETURN_FALSE;
		}
	}
}
#endif


/* {{{ stream_select related functions */
static int pthreads_stream_array_to_fd_set(zval *stream_array, fd_set *fds, php_socket_t *max_fd) {
	zval *elem;
	pthreads_stream_t *threaded_stream;
	int cnt = 0;

	if (Z_TYPE_P(stream_array) != IS_ARRAY) {
		return 0;
	}

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(stream_array), elem) {
		/* Temporary int fd is needed for the STREAM data type on windows, passing this_fd directly to pthreads_stream_cast()
			would eventually bring a wrong result on x64. pthreads_stream_cast() casts to int internally, and this will leave
			the higher bits of a SOCKET variable uninitialized on systems with little endian. */
		php_socket_t this_fd;

		ZVAL_DEREF(elem);
		threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(elem));
		if (threaded_stream == NULL) {
			continue;
		}
		/* get the fd.
		 * NB: Most other code will NOT use the PTHREADS_STREAM_CAST_INTERNAL flag
		 * when casting.  It is only used here so that the buffered data warning
		 * is not displayed.
		 * */
		if (SUCCESS == pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT | PTHREADS_STREAM_CAST_INTERNAL, (void*)&this_fd, 1) && this_fd != -1) {

			PHP_SAFE_FD_SET(this_fd, fds);

			if (this_fd > *max_fd) {
				*max_fd = this_fd;
			}
			cnt++;
		}
	} ZEND_HASH_FOREACH_END();

	return cnt ? 1 : 0;
}

static int pthreads_stream_array_from_fd_set(zval *stream_array, fd_set *fds) {
	zval *elem, *dest_elem;
	HashTable *ht;
	pthreads_stream_t *threaded_stream;
	int ret = 0;
	zend_string *key;
	zend_ulong num_ind;

	if (Z_TYPE_P(stream_array) != IS_ARRAY) {
		return 0;
	}
	ht = pthreads_new_array(zend_hash_num_elements(Z_ARRVAL_P(stream_array)));

	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(stream_array), num_ind, key, elem) {
		php_socket_t this_fd;

		ZVAL_DEREF(elem);
		threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(elem));
		if (threaded_stream == NULL) {
			continue;
		}
		/* get the fd
		 * NB: Most other code will NOT use the PTHREADS_STREAM_CAST_INTERNAL flag
		 * when casting.  It is only used here so that the buffered data warning
		 * is not displayed.
		 */
		if (SUCCESS == pthreads_stream_cast(threaded_stream, PTHREADS_STREAM_AS_FD_FOR_SELECT | PTHREADS_STREAM_CAST_INTERNAL, (void*)&this_fd, 1) && this_fd != SOCK_ERR) {
			if (PHP_SAFE_FD_ISSET(this_fd, fds)) {
				if (!key) {
					dest_elem = zend_hash_index_update(ht, num_ind, elem);
				} else {
					dest_elem = zend_hash_update(ht, key, elem);
				}

				zval_add_ref(dest_elem);
				ret++;
				continue;
			}
		}
	} ZEND_HASH_FOREACH_END();

	/* destroy old array and add new one */
	zval_ptr_dtor(stream_array);
	ZVAL_ARR(stream_array, ht);

	return ret;
}

static int pthreads_stream_array_emulate_read_fd_set(zval *stream_array) {
	zval *elem, *dest_elem;
	HashTable *ht;
	pthreads_stream_t *threaded_stream;
	pthreads_stream *stream;
	int ret = 0;

	if (Z_TYPE_P(stream_array) != IS_ARRAY) {
		return 0;
	}
	ht = pthreads_new_array(zend_hash_num_elements(Z_ARRVAL_P(stream_array)));

	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(stream_array), elem) {
		ZVAL_DEREF(elem);
		threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(elem));
		if (threaded_stream == NULL) {
			continue;
		}
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

		if ((stream->writepos - stream->readpos) > 0) {
			/* allow readable non-descriptor based streams to participate in stream_select.
			 * Non-descriptor streams will only "work" if they have previously buffered the
			 * data.  Not ideal, but better than nothing.
			 * This branch of code also allows blocking streams with buffered data to
			 * operate correctly in stream_select.
			 * */
			dest_elem = zend_hash_next_index_insert(ht, elem);
			if (dest_elem) {
				zval_add_ref(dest_elem);
			}
			ret++;
			continue;
		}
	} ZEND_HASH_FOREACH_END();

	if (ret > 0) {
		/* destroy old array and add new one */
		zval_ptr_dtor(stream_array);
		ZVAL_ARR(stream_array, ht);
	} else {
		zend_array_destroy(ht);
	}

	return ret;
}
/* }}} */

/* {{{ proto int stream_select(array &read_streams, array &write_streams, array &except_streams, int tv_sec[, int tv_usec])
   Runs the select() system call on the sets of streams with a timeout specified by tv_sec and tv_usec */
void pthreads_streams_api_stream_select(zval *r_array, zval *w_array, zval *e_array, zval *sec, zend_long usec, zval *return_value) {
	struct timeval tv, *tv_p = NULL;
	fd_set rfds, wfds, efds;
	php_socket_t max_fd = 0;
	int retval, sets = 0;
	int set_count, max_set_count = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	if (r_array != NULL) {
		set_count = pthreads_stream_array_to_fd_set(r_array, &rfds, &max_fd);
		if (set_count > max_set_count)
			max_set_count = set_count;
		sets += set_count;
	}

	if (w_array != NULL) {
		set_count = pthreads_stream_array_to_fd_set(w_array, &wfds, &max_fd);
		if (set_count > max_set_count)
			max_set_count = set_count;
		sets += set_count;
	}

	if (e_array != NULL) {
		set_count = pthreads_stream_array_to_fd_set(e_array, &efds, &max_fd);
		if (set_count > max_set_count)
			max_set_count = set_count;
		sets += set_count;
	}

	if (!sets) {
		php_error_docref(NULL, E_WARNING, "No stream arrays were passed");
		RETURN_FALSE;
	}

	PHP_SAFE_MAX_FD(max_fd, max_set_count);

	/* If seconds is not set to null, build the timeval, else we wait indefinitely */
	if (sec != NULL) {
		zval tmp;

		if (Z_TYPE_P(sec) != IS_LONG) {
			tmp = *sec;
			zval_copy_ctor(&tmp);
			convert_to_long(&tmp);
			sec = &tmp;
		}

		if (Z_LVAL_P(sec) < 0) {
			php_error_docref(NULL, E_WARNING, "The seconds parameter must be greater than 0");
			RETURN_FALSE;
		} else if (usec < 0) {
			php_error_docref(NULL, E_WARNING, "The microseconds parameter must be greater than 0");
			RETURN_FALSE;
		}

		/* Windows, Solaris and BSD do not like microsecond values which are >= 1 sec */
		tv.tv_sec = (long)(Z_LVAL_P(sec) + (usec / 1000000));
		tv.tv_usec = (long)(usec % 1000000);
		tv_p = &tv;
	}

	/* slight hack to support buffered data; if there is data sitting in the
	 * read buffer of any of the streams in the read array, let's pretend
	 * that we selected, but return only the readable sockets */
	if (r_array != NULL) {
		retval = pthreads_stream_array_emulate_read_fd_set(r_array);
		if (retval > 0) {
			if (w_array != NULL) {
				zval_ptr_dtor(w_array);
				ZVAL_EMPTY_ARRAY(w_array);
			}
			if (e_array != NULL) {
				zval_ptr_dtor(e_array);
				ZVAL_EMPTY_ARRAY(e_array);
			}
			RETURN_LONG(retval);
		}
	}

	retval = php_select(max_fd+1, &rfds, &wfds, &efds, tv_p);

	if (retval == -1) {
		php_error_docref(NULL, E_WARNING, "unable to select [%d]: %s (max_fd=%d)",
				errno, strerror(errno), max_fd);
		RETURN_FALSE;
	}

	if (r_array != NULL) pthreads_stream_array_from_fd_set(r_array, &rfds);
	if (w_array != NULL) pthreads_stream_array_from_fd_set(w_array, &wfds);
	if (e_array != NULL) pthreads_stream_array_from_fd_set(e_array, &efds);

	RETURN_LONG(retval);
}
/* }}} */


/* {{{ pthreads_streams_api_stream_set_blocking */
void pthreads_streams_api_stream_set_blocking(zval *object, zend_bool blocking, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_BLOCKING, blocking, NULL) == -1) {
		RETURN_FALSE;
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ pthreads_streams_api_stream_set_chunk_size */
void pthreads_streams_api_stream_set_chunk_size(zval *object, zend_long csize, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	int ret;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_SET_CHUNK_SIZE, (int)csize, NULL);

	RETURN_LONG(ret > 0 ? (zend_long)ret : (zend_long)EOF);
}
/* }}} */

/* {{{ pthreads_streams_api_stream_set_read_buffer */
void pthreads_streams_api_stream_set_read_buffer(zval *object, size_t buff, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	int ret;

	/* if buff is 0 then set to non-buffered */
	if (buff == 0) {
		ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_READ_BUFFER, PTHREADS_STREAM_BUFFER_NONE, NULL);
	} else {
		ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_READ_BUFFER, PTHREADS_STREAM_BUFFER_FULL, &buff);
	}

	RETURN_LONG(ret == 0 ? 0 : EOF);
}
/* }}} */

/* {{{ pthreads_streams_api_stream_set_timeout */
#if HAVE_SYS_TIME_H || defined(PHP_WIN32)
void pthreads_streams_api_stream_set_timeout(int argc, zval *object, zend_long seconds, zend_long microseconds, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	struct timeval t;

#ifdef PHP_WIN32
	t.tv_sec = (long)seconds;

	if (argc == 3) {
		t.tv_usec = (long)(microseconds % 1000000);
		t.tv_sec +=(long)(microseconds / 1000000);
	} else {
		t.tv_usec = 0;
	}
#else
	t.tv_sec = seconds;

	if (argc == 3) {
		t.tv_usec = microseconds % 1000000;
		t.tv_sec += microseconds / 1000000;
	} else {
		t.tv_usec = 0;
	}
#endif

	if (PTHREADS_STREAM_OPTION_RETURN_OK == pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_READ_TIMEOUT, 0, &t)) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}
#endif /* HAVE_SYS_TIME_H || defined(PHP_WIN32) */
/* }}} */


/* {{{ pthreads_streams_api_stream_set_write_buffer */
void pthreads_streams_api_stream_set_write_buffer(zval *object, size_t buff, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	int ret;

	/* if buff is 0 then set to non-buffered */
	if (buff == 0) {
		ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_WRITE_BUFFER, PTHREADS_STREAM_BUFFER_NONE, NULL);
	} else {
		ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_WRITE_BUFFER, PTHREADS_STREAM_BUFFER_FULL, &buff);
	}

	RETURN_LONG(ret == 0 ? 0 : EOF);
}
/* }}} */

/* {{{ pthreads_streams_api_stream_supports_lock */
void pthreads_streams_api_stream_supports_lock(zval *object, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	if (!pthreads_stream_supports_lock(threaded_stream)) {
		RETURN_FALSE;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ pthreads_streams_api_stream_from_resource */
void pthreads_streams_api_stream_from_resource(php_stream *stream, zval *return_value) {



	RETURN_NULL();
}
/* }}} */

/**
 * SocketStream
 */

/* {{{ pthreads_streams_api_socket_stream_create_client */
void pthreads_streams_api_socket_stream_create_client(zend_string *host, zval *zerrno, zval *zerrstr, double timeout, zend_long flags, zval *zcontext, zval *return_value) {
	pthreads_timeout_ull conv;
	struct timeval tv;
	int err;
	zend_string *errstr = NULL;
	pthreads_stream_t *threaded_stream;
	pthreads_stream_context_t *threaded_context = NULL;

	RETVAL_NULL();

	threaded_context = pthreads_stream_context_from_zval(zcontext, flags & PTHREADS_FILE_NO_DEFAULT_CONTEXT);

	/* prepare the timeout value for use */
	conv = (pthreads_timeout_ull) (timeout * 1000000.0);
#ifdef PHP_WIN32
	tv.tv_sec = (long)(conv / 1000000);
	tv.tv_usec =(long)(conv % 1000000);
#else
	tv.tv_sec = conv / 1000000;
	tv.tv_usec = conv % 1000000;
#endif
	if (zerrno)	{
		zval_ptr_dtor(zerrno);
		ZVAL_LONG(zerrno, 0);
	}
	if (zerrstr) {
		zval_ptr_dtor(zerrstr);
		ZVAL_EMPTY_STRING(zerrstr);
	}

	threaded_stream = pthreads_stream_xport_create(ZSTR_VAL(host), ZSTR_LEN(host), PTHREADS_REPORT_ERRORS,
			PTHREADS_STREAM_XPORT_CLIENT | (flags & PTHREADS_STREAM_CLIENT_CONNECT ? PTHREADS_STREAM_XPORT_CONNECT : 0) |
			(flags & PTHREADS_STREAM_CLIENT_ASYNC_CONNECT ? PTHREADS_STREAM_XPORT_CONNECT_ASYNC : 0),
			&tv, threaded_context, &errstr, &err);

	if (threaded_stream == NULL) {
		/* host might contain binary characters */
		zend_string *quoted_host = pthreads_addslashes(host);

		php_error_docref(NULL, E_WARNING, "unable to connect to %s (%s)", ZSTR_VAL(quoted_host), errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
		zend_string_release(quoted_host);

		if (zerrno) {
			zval_ptr_dtor(zerrno);
			ZVAL_LONG(zerrno, err);
		}
		if (zerrstr && errstr) {
			zval_ptr_dtor(zerrstr);
			ZVAL_STR(zerrstr, errstr);
		} else if (errstr) {
			zend_string_release(errstr);
		}
		RETURN_NULL();
	}

	if (errstr) {
		zend_string_release(errstr);
	}

	pthreads_stream_to_zval(threaded_stream, return_value);

}
/* }}} */

/* {{{ pthreads_streams_api_socket_stream_create_server */
void pthreads_streams_api_socket_stream_create_server(zend_string *host, zval *zerrno, zval *zerrstr, zend_long flags, zval *zcontext, zval *return_value) {
	int err = 0;
	zend_string *errstr = NULL;
	pthreads_stream_t *threaded_stream;
	pthreads_stream_context_t *threaded_context = NULL;

	RETVAL_NULL();

	threaded_context = pthreads_stream_context_from_zval(zcontext, flags & PTHREADS_FILE_NO_DEFAULT_CONTEXT);

	if (zerrno)	{
		zval_ptr_dtor(zerrno);
		ZVAL_LONG(zerrno, 0);
	}
	if (zerrstr) {
		zval_ptr_dtor(zerrstr);
		ZVAL_EMPTY_STRING(zerrstr);
	}

	threaded_stream = pthreads_stream_xport_create(ZSTR_VAL(host), ZSTR_LEN(host), PTHREADS_REPORT_ERRORS,
			PTHREADS_STREAM_XPORT_SERVER | (int)flags, NULL, threaded_context, &errstr, &err);

	if (threaded_stream == NULL) {
		php_error_docref(NULL, E_WARNING, "unable to connect to %s (%s)", host, errstr == NULL ? "Unknown error" : ZSTR_VAL(errstr));
	}

	if (threaded_stream == NULL)	{
		if (zerrno) {
			zval_ptr_dtor(zerrno);
			ZVAL_LONG(zerrno, err);
		}
		if (zerrstr && errstr) {
			zval_ptr_dtor(zerrstr);
			ZVAL_STR(zerrstr, errstr);
		} else if (errstr) {
			zend_string_release(errstr);
		}
		RETURN_NULL();
	}

	if (errstr) {
		zend_string_release(errstr);
	}

	pthreads_stream_to_zval(threaded_stream, return_value);
}
/* }}} */

#if HAVE_SOCKETPAIR
/* {{{ pthreads_streams_api_socket_stream_socket_pair */
void pthreads_streams_api_socket_stream_pair(zend_long domain, zend_long type, zend_long protocol, zval *return_value) {
	pthreads_stream_t *s1, *s2;
	php_socket_t pair[2];
	zval obj1, obj2;

	if (0 != socketpair((int)domain, (int)type, (int)protocol, pair)) {
		char errbuf[256];
		php_error_docref(NULL, E_WARNING, "failed to create sockets: [%d]: %s",
			php_socket_errno(), php_socket_strerror(php_socket_errno(), errbuf, sizeof(errbuf)));
		RETURN_FALSE;
	}

	array_init(return_value);

	s1 = _pthreads_stream_sock_open_from_socket(pair[0], pthreads_socket_stream_entry);
	s2 = _pthreads_stream_sock_open_from_socket(pair[1], pthreads_socket_stream_entry);

	ZVAL_OBJ(&obj1, PTHREADS_STD_P(s1));
	ZVAL_OBJ(&obj2, PTHREADS_STD_P(s2));

	add_next_index_zval(return_value, &obj1);
	add_next_index_zval(return_value, &obj2);
}
/* }}} */
#endif

/* {{{ pthreads_streams_api_socket_stream_accept */
void pthreads_streams_api_socket_stream_accept(zval *object, double timeout, zval *zpeername, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	pthreads_stream_t *clistream = NULL;
	zend_string *peername = NULL;
	pthreads_timeout_ull conv;
	struct timeval tv;
	zend_string *errstr = NULL;

	/* prepare the timeout value for use */
	conv = (pthreads_timeout_ull) (timeout * 1000000.0);
#ifdef PHP_WIN32
	tv.tv_sec = (long)(conv / 1000000);
	tv.tv_usec = (long)(conv % 1000000);
#else
	tv.tv_sec = conv / 1000000;
	tv.tv_usec = conv % 1000000;
#endif
	if (zpeername) {
		zval_ptr_dtor(zpeername);
		ZVAL_NULL(zpeername);
	}

	if (0 == pthreads_stream_xport_accept(threaded_stream, &clistream,
				zpeername ? &peername : NULL,
				NULL, NULL,
				&tv, &errstr
				) && clistream) {

		if (peername) {
			ZVAL_STR(zpeername, peername);
		}
		pthreads_stream_to_zval(clistream, return_value);
	} else {
		php_error_docref(NULL, E_WARNING, "accept failed: %s", errstr ? ZSTR_VAL(errstr) : "Unknown error");
		RETVAL_FALSE;
	}

	if (errstr) {
		zend_string_release(errstr);
	}
}
/* }}} */

/* {{{ pthreads_streams_api_socket_stream_get_name */
void pthreads_streams_api_socket_stream_get_name(zval *object, zend_bool want_peer, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *name = NULL;

	if (0 != pthreads_stream_xport_get_name(threaded_stream, want_peer,
				&name,
				NULL, NULL
				) || !name) {
		RETURN_NULL();
	}

	if ((ZSTR_LEN(name) == 0) || (ZSTR_VAL(name)[0] == 0)) {
		zend_string_release(name);
		RETURN_NULL();
	}

	RETVAL_STR(name);
}
/* }}} */

/* {{{ pthreads_streams_api_socket_stream_recvfrom */
void pthreads_streams_api_socket_stream_recvfrom(zval *object, zend_long to_read, zend_long flags, zval *zremote, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));
	zend_string *remote_addr = NULL;
	zend_string *read_buf;
	int recvd;

	if (zremote) {
		zval_ptr_dtor(zremote);
		ZVAL_NULL(zremote);
	}

	if (to_read <= 0) {
		php_error_docref(NULL, E_WARNING, "Length parameter must be greater than 0");
		RETURN_FALSE;
	}

	read_buf = zend_string_alloc(to_read, 0);

	recvd = pthreads_stream_xport_recvfrom(threaded_stream, ZSTR_VAL(read_buf), to_read, (int)flags, NULL, NULL,
			zremote ? &remote_addr : NULL
			);

	if (recvd >= 0) {
		if (zremote && remote_addr) {
			ZVAL_STR(zremote, remote_addr);
		}
		ZSTR_VAL(read_buf)[recvd] = '\0';
		ZSTR_LEN(read_buf) = recvd;
		RETURN_NEW_STR(read_buf);
	}

	zend_string_efree(read_buf);
	RETURN_FALSE;
}
/* }}} */

/* {{{ pthreads_streams_api_socket_stream_sendto */
void pthreads_streams_api_socket_stream_sendto(zval *object, zend_string *data, zend_long flags, zend_string *target_addr, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	php_sockaddr_storage sa;
	socklen_t sl = 0;

	if (target_addr) {
		/* parse the address */
		if (FAILURE == php_network_parse_network_address_with_port(ZSTR_VAL(target_addr), ZSTR_LEN(target_addr), (struct sockaddr*)&sa, &sl)) {
			php_error_docref(NULL, E_WARNING, "Failed to parse `%s' into a valid network address", target_addr);
			RETURN_FALSE;
		}
	}

	RETURN_LONG(pthreads_stream_xport_sendto(threaded_stream, ZSTR_VAL(data), ZSTR_LEN(data), (int)flags, target_addr ? &sa : NULL, sl));
}
/* }}} */


#ifdef HAVE_SHUTDOWN
/* {{{ proto int SocketStream::shutdown(int how)
	causes all or part of a full-duplex connection on the socket associated
	with stream to be shut down.  If how is SHUT_RD,  further receptions will
	be disallowed. If how is SHUT_WR, further transmissions will be disallowed.
	If how is SHUT_RDWR,  further  receptions and transmissions will be
	disallowed. */
void pthreads_streams_api_socket_stream_shutdown(zval *object, zend_long how, zval *return_value) {
	pthreads_stream_t *threaded_stream = PTHREADS_FETCH_FROM(Z_OBJ_P(object));

	RETURN_BOOL(pthreads_stream_xport_shutdown(threaded_stream, (pthreads_stream_shutdown_t)how) == 0);
}
/* }}} */
#endif


#endif
