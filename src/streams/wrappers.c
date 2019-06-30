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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS
#define HAVE_PTHREADS_STREAMS_WRAPPERS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_H
#	include <src/streams/wrappers.h>
#endif

int pthreads_init_stream_wrappers() {
	return (pthreads_register_url_stream_wrapper("php", PTHREADS_STREAMG(stream_php_wrapper)) == SUCCESS &&
			pthreads_register_url_stream_wrapper("file", PTHREADS_STREAMG(plain_files_wrapper)) == SUCCESS &&
#ifdef HAVE_GLOB
			pthreads_register_url_stream_wrapper("glob", PTHREADS_STREAMG(glob_stream_wrapper)) == SUCCESS &&
#endif
			pthreads_register_url_stream_wrapper("data", PTHREADS_STREAMG(stream_rfc2397_wrapper)) == SUCCESS &&
			pthreads_register_url_stream_wrapper("http", PTHREADS_STREAMG(stream_http_wrapper)) == SUCCESS &&
			pthreads_register_url_stream_wrapper("ftp", PTHREADS_STREAMG(stream_ftp_wrapper)) == SUCCESS
		) ? SUCCESS : FAILURE;
}

int pthreads_shutdown_stream_wrappers() {
	return (pthreads_unregister_url_stream_wrapper("php") == SUCCESS &&
			pthreads_unregister_url_stream_wrapper("file") == SUCCESS &&
#ifdef HAVE_GLOB
			pthreads_unregister_url_stream_wrapper("glob") == SUCCESS &&
#endif
			pthreads_unregister_url_stream_wrapper("data") == SUCCESS &&
			pthreads_unregister_url_stream_wrapper("http") == SUCCESS &&
			pthreads_unregister_url_stream_wrapper("ftp") == SUCCESS
		) ? SUCCESS : FAILURE;
}

/* {{{ wrapper error reporting */
static zend_llist *pthreads_get_wrapper_errors_list(pthreads_stream_wrapper_t *threaded_wrapper) {
	pthreads_hashtable *wrapper_errors = &PTHREADS_STREAMG(wrapper_errors);
	zend_llist *result = NULL;

	if(MONITOR_LOCK(wrapper_errors)) {
		result = (zend_llist*) zend_hash_str_find_ptr(&wrapper_errors->ht, (const char*)&threaded_wrapper, sizeof(threaded_wrapper));

        MONITOR_UNLOCK(wrapper_errors);
	}
	return result;
}

void pthreads_stream_display_wrapper_errors(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *caption) {
	char *tmp = estrdup(path);
	char *msg;
	int free_msg = 0;

	if (threaded_wrapper) {
		pthreads_hashtable *pht = &PTHREADS_STREAMG(wrapper_errors);

		if(pthreads_monitor_lock(pht->monitor)) {
			zend_llist *err_list = pthreads_get_wrapper_errors_list(threaded_wrapper);
			if (err_list) {
				size_t l = 0;
				int brlen;
				int i;
				int count = (int)zend_llist_count(err_list);
				const char *br;
				const char **err_buf_p;
				zend_llist_position pos;

				if (PG(html_errors)) {
					brlen = 7;
					br = "<br />\n";
				} else {
					brlen = 1;
					br = "\n";
				}

				for (err_buf_p = zend_llist_get_first_ex(err_list, &pos), i = 0;
						err_buf_p;
						err_buf_p = zend_llist_get_next_ex(err_list, &pos), i++) {
					l += strlen(*err_buf_p);
					if (i < count - 1) {
						l += brlen;
					}
				}
				msg = emalloc(l + 1);
				msg[0] = '\0';
				for (err_buf_p = zend_llist_get_first_ex(err_list, &pos), i = 0;
						err_buf_p;
						err_buf_p = zend_llist_get_next_ex(err_list, &pos), i++) {
					strcat(msg, *err_buf_p);
					if (i < count - 1) {
						strcat(msg, br);
					}
				}

				free_msg = 1;
			} else {
				if (threaded_wrapper == PTHREADS_STREAMG(plain_files_wrapper)) {
					msg = strerror(errno); /* TODO: not ts on linux */
				} else {
					msg = "operation failed";
				}
			}
	        pthreads_monitor_unlock(pht->monitor);
		}
	} else {
		msg = "no suitable wrapper could be found";
	}

	php_strip_url_passwd(tmp);
	php_error_docref1(NULL, tmp, E_WARNING, "%s: %s", caption, msg);
	efree(tmp);
	if (free_msg && msg) {
		efree(msg);
	}
}

void pthreads_stream_tidy_wrapper_error_log(pthreads_stream_wrapper_t *threaded_wrapper) {
	if (!threaded_wrapper) {
		return;
	}
	pthreads_hashtable *wrapper_errors = &PTHREADS_STREAMG(wrapper_errors);

	if(MONITOR_LOCK(wrapper_errors)) {
		zend_hash_str_del(&wrapper_errors->ht, (const char*)&threaded_wrapper, sizeof(threaded_wrapper));
		MONITOR_UNLOCK(wrapper_errors);
	}
}

static void wrapper_error_dtor(void *error) {
	free(*(char**)error);
}

void pthreads_stream_wrapper_log_error(const pthreads_stream_wrapper_t *threaded_wrapper, int options, const char *fmt, ...) {
	va_list args;
	char *buffer = NULL;

	va_start(args, fmt);
	vspprintf(&buffer, 0, fmt, args);
	va_end(args);

	if (options & PTHREADS_REPORT_ERRORS || threaded_wrapper == NULL) {
		php_error_docref(NULL, E_WARNING, "%s", buffer);
		efree(buffer);
	} else {
		pthreads_hashtable *wrapper_errors = &PTHREADS_STREAMG(wrapper_errors);

		if(MONITOR_LOCK(wrapper_errors)) {
			zend_llist *list = zend_hash_str_find_ptr(&wrapper_errors->ht, (const char*)&threaded_wrapper, sizeof(threaded_wrapper));

			if (!list) {
				zend_llist new_list;
				zend_llist_init(&new_list, sizeof(buffer), wrapper_error_dtor, 1);
				list = zend_hash_str_update_mem(&wrapper_errors->ht, (const char*)&threaded_wrapper,
						sizeof(threaded_wrapper), &new_list, sizeof(new_list));
			}
			char *buf = strdup(buffer);
			efree(buffer);

			/* append to linked list */
			zend_llist_add_element(list, &buf);
			MONITOR_UNLOCK(wrapper_errors);
		}
	}
}

/* }}} */

/* Validate protocol scheme names during registration
 * Must conform to /^[a-zA-Z0-9+.-]+$/
 */
static inline int pthreads_stream_wrapper_scheme_validate(const char *protocol, unsigned int protocol_len) {
	unsigned int i;

	for(i = 0; i < protocol_len; i++) {
		if (!isalnum((int)protocol[i]) &&
			protocol[i] != '+' &&
			protocol[i] != '-' &&
			protocol[i] != '.') {
			return FAILURE;
		}
	}

	return SUCCESS;
}

void pthreads_stream_wrapper_free(pthreads_stream_wrapper *wrapper) {
	free(wrapper);
}

pthreads_stream_wrapper *pthreads_stream_wrapper_alloc(void) {
	pthreads_stream_wrapper *wrapper = calloc(1, sizeof(pthreads_stream_wrapper));

	return wrapper;
}

/* {{{  API for registering GLOBAL wrappers */
pthreads_hashtable *_pthreads_stream_get_url_stream_wrappers_hash(void) {
	return &PTHREADS_STREAMG(url_stream_wrappers_hash);
}

int pthreads_register_url_stream_wrapper(const char *protocol, pthreads_stream_wrapper_t *threaded_wrapper) {
	pthreads_hashtable *url_stream_wrappers_hash = &PTHREADS_STREAMG(url_stream_wrappers_hash);
	unsigned int protocol_len = (unsigned int)strlen(protocol);
	int result = FAILURE;

	if (pthreads_stream_wrapper_scheme_validate(protocol, protocol_len) == FAILURE) {
		return result;
	}

	if(MONITOR_LOCK(url_stream_wrappers_hash)) {
		result = zend_hash_add_ptr(&url_stream_wrappers_hash->ht, zend_string_init_interned(protocol, protocol_len, 1), threaded_wrapper) ? SUCCESS : FAILURE;
		MONITOR_UNLOCK(url_stream_wrappers_hash);
	}
	return result;
}

int pthreads_unregister_url_stream_wrapper(const char *protocol) {
	pthreads_hashtable *url_stream_wrappers_hash = &PTHREADS_STREAMG(url_stream_wrappers_hash);
	int result = FAILURE;

	if(MONITOR_LOCK(url_stream_wrappers_hash)) {
		result = zend_hash_str_del(&url_stream_wrappers_hash->ht, protocol, strlen(protocol));
		MONITOR_UNLOCK(url_stream_wrappers_hash);
	}
	return result;
}
/* }}} */

/* {{{ pthreads_stream_locate_url_wrapper */
pthreads_stream_wrapper_t *pthreads_stream_locate_url_wrapper(const char *path, const char **path_for_open, int options) {
	pthreads_hashtable *wrapper_hash = &PTHREADS_STREAMG(url_stream_wrappers_hash);
	pthreads_stream_wrapper *wrapper = NULL;
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	const char *p, *protocol = NULL;
	size_t n = 0;

	if (path_for_open) {
		*path_for_open = (char*)path;
	}

	if (options & IGNORE_URL) {
		return (options & PTHREADS_STREAM_LOCATE_WRAPPERS_ONLY) ? NULL : PTHREADS_STREAMG(plain_files_wrapper);
	}

	for (p = path; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++) {
		n++;
	}

	if ((*p == ':') && (n > 1) && (!strncmp("//", p+1, 2) || (n == 4 && !memcmp("data:", path, 5)))) {
		protocol = path;
	}

	if (protocol) {
		char *tmp = estrndup(protocol, n);
		if(MONITOR_LOCK(wrapper_hash)) {
			if (NULL == (threaded_wrapper = zend_hash_str_find_ptr(&wrapper_hash->ht, (char*)tmp, n))) {
				php_strtolower(tmp, n);
				if (NULL == (threaded_wrapper = zend_hash_str_find_ptr(&wrapper_hash->ht, (char*)tmp, n))) {
					char wrapper_name[32];

					if (n >= sizeof(wrapper_name)) {
						n = sizeof(wrapper_name) - 1;
					}
					PHP_STRLCPY(wrapper_name, protocol, sizeof(wrapper_name), n);

					php_error_docref(NULL, E_WARNING, "Unable to find the wrapper \"%s\" - did you forget to enable it when you configured PHP?", wrapper_name);

					threaded_wrapper = NULL;
					protocol = NULL;
				}
			}
			MONITOR_UNLOCK(wrapper_hash);
		}
		efree(tmp);
	}
	/* TODO: curl based streams probably support file:// properly */
	if (!protocol || !strncasecmp(protocol, "file", n))	{
		/* fall back on regular file access */
		pthreads_stream_wrapper_t *plain_files_wrapper = PTHREADS_STREAMG(plain_files_wrapper);

		if (protocol) {
			int localhost = 0;

			if (!strncasecmp(path, "file://localhost/", 17)) {
				localhost = 1;
			}

#ifdef PHP_WIN32
			if (localhost == 0 && path[n+3] != '\0' && path[n+3] != '/' && path[n+4] != ':')	{
#else
			if (localhost == 0 && path[n+3] != '\0' && path[n+3] != '/') {
#endif
				if (options & REPORT_ERRORS) {
					php_error_docref(NULL, E_WARNING, "remote host file access not supported, %s", path);
				}
				return NULL;
			}

			if (path_for_open) {
				/* skip past protocol and :/, but handle windows correctly */
				*path_for_open = (char*)path + n + 1;
				if (localhost == 1) {
					(*path_for_open) += 11;
				}
				while (*(++*path_for_open)=='/') {
					/* intentionally empty */
				}
#ifdef PHP_WIN32
				if (*(*path_for_open + 1) != ':')
#endif
					(*path_for_open)--;
			}
		}

		if (options & PTHREADS_STREAM_LOCATE_WRAPPERS_ONLY) {
			return NULL;
		}

		if (threaded_wrapper) {
			/* It was found so go ahead and provide it */
			return threaded_wrapper;
		}

		/* Check again, the original check might have not known the protocol name */
		if(MONITOR_LOCK(wrapper_hash)) {
			threaded_wrapper = zend_hash_str_find_ptr(&wrapper_hash->ht, "file", sizeof("file")-1);
			MONITOR_UNLOCK(wrapper_hash);

			if (threaded_wrapper != NULL) {
				return threaded_wrapper;
			}
		}

		return plain_files_wrapper;
	}

	if(threaded_wrapper) {
		wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

		if (wrapper && wrapper->is_url &&
			(options & PTHREADS_STREAM_DISABLE_URL_PROTECTION) == 0 &&
			(!PG(allow_url_fopen) ||
			 (((options & PTHREADS_STREAM_OPEN_FOR_INCLUDE) ||
			   PG(in_user_include)) && !PG(allow_url_include)))) {
			if (options & PTHREADS_REPORT_ERRORS) {
				/* protocol[n] probably isn't '\0' */
				char *protocol_dup = estrndup(protocol, n);
				if (!PG(allow_url_fopen)) {
					php_error_docref(NULL, E_WARNING, "%s:// wrapper is disabled in the server configuration by allow_url_fopen=0", protocol_dup);
				} else {
					php_error_docref(NULL, E_WARNING, "%s:// wrapper is disabled in the server configuration by allow_url_include=0", protocol_dup);
				}
				efree(protocol_dup);
			}
			return NULL;
		}
	}
	return threaded_wrapper;
}
/* }}} */

/* {{{ _pthreads_stream_mkdir */
int _pthreads_stream_mkdir(const char *path, int mode, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	pthreads_stream_wrapper *wrapper = NULL;
	int result = FAILURE;

	threaded_wrapper = pthreads_stream_locate_url_wrapper(path, NULL, 0);
	if (!threaded_wrapper) {
		return result;
	}
	wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

	if(MONITOR_LOCK(threaded_wrapper)) {
		if(!wrapper->wops || !wrapper->wops->stream_mkdir) {
			MONITOR_UNLOCK(threaded_wrapper);
			return result;
		}
		result = wrapper->wops->stream_mkdir(threaded_wrapper, path, mode, options, threaded_context);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return result;
}
/* }}} */

/* {{{ _pthreads_stream_rmdir */
int _pthreads_stream_rmdir(const char *path, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	pthreads_stream_wrapper *wrapper = NULL;
	int result = FAILURE;

	threaded_wrapper = pthreads_stream_locate_url_wrapper(path, NULL, 0);
	if (!threaded_wrapper) {
		return result;
	}
	wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

	if(MONITOR_LOCK(threaded_wrapper)) {
		if(!wrapper->wops || !wrapper->wops->stream_rmdir) {
			MONITOR_UNLOCK(threaded_wrapper);
			return result;
		}
		result = wrapper->wops->stream_rmdir(threaded_wrapper, path, options, threaded_context);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return result;
}
/* }}} */

/* {{{ pthreads_stream_stat_path */
int _pthreads_stream_stat_path(const char *path, int flags, pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	pthreads_stream_wrapper *wrapper = NULL;
	const char *path_to_open = path;
	int ret;

	if (!(flags & PTHREADS_STREAM_URL_STAT_NOCACHE)) {
		if(pthreads_stream_globals_lock()) {
			/* Try to hit the cache first */
			if (flags & PTHREADS_STREAM_URL_STAT_LINK) {
				if (PTHREADS_STREAMG(CurrentLStatFile) && strcmp(path, PTHREADS_STREAMG(CurrentLStatFile)) == 0) {
					memcpy(ssb, &PTHREADS_STREAMG(lssb), sizeof(pthreads_stream_statbuf));
					return 0;
				}
			} else {
				if (PTHREADS_STREAMG(CurrentStatFile) && strcmp(path, PTHREADS_STREAMG(CurrentStatFile)) == 0) {
					memcpy(ssb, &PTHREADS_STREAMG(ssb), sizeof(pthreads_stream_statbuf));
					return 0;
				}
			}
			pthreads_stream_globals_unlock();
		}
	}

	threaded_wrapper = pthreads_stream_locate_url_wrapper(path, &path_to_open, 0);
	if (threaded_wrapper) {
		wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);

		if(MONITOR_LOCK(threaded_wrapper)) {
			if(wrapper->wops->url_stat) {
				ret = wrapper->wops->url_stat(threaded_wrapper, path_to_open, flags, ssb, threaded_context);
				if (ret == 0) {
					if (!(flags & PTHREADS_STREAM_URL_STAT_NOCACHE)) {
						if(pthreads_stream_globals_lock()) {
							/* Drop into cache */
							if (flags & PTHREADS_STREAM_URL_STAT_LINK) {
								if (PTHREADS_STREAMG(CurrentLStatFile)) {
									free(PTHREADS_STREAMG(CurrentLStatFile));
								}
								PTHREADS_STREAMG(CurrentLStatFile) = strdup(path);
								memcpy(&PTHREADS_STREAMG(lssb), ssb, sizeof(pthreads_stream_statbuf));
							} else {
								if (PTHREADS_STREAMG(CurrentStatFile)) {
									free(BG(CurrentStatFile));
								}
								PTHREADS_STREAMG(CurrentStatFile) = strdup(path);
								memcpy(&PTHREADS_STREAMG(ssb), ssb, sizeof(pthreads_stream_statbuf));
							}
							pthreads_stream_globals_unlock();
						}
					}
				}
			}
			MONITOR_UNLOCK(threaded_wrapper);
		}
		return ret;
	}
	return -1;
}
/* }}} */

/* {{{ pthreads_stream_opendir */
pthreads_stream_t *_pthreads_stream_opendir(const char *path, int options, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	pthreads_stream_wrapper *wrapper = NULL;
	const char *path_to_open;

	if (!path || !*path) {
		return NULL;
	}
	path_to_open = path;

	threaded_wrapper = pthreads_stream_locate_url_wrapper(path, &path_to_open, options);

	if (threaded_wrapper) {
		wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
		if(MONITOR_LOCK(threaded_wrapper)) {
			if(wrapper->wops->dir_opener) {
				if(ce == NULL) {
					ce = pthreads_stream_entry;
				}
				threaded_stream = wrapper->wops->dir_opener(threaded_wrapper,
								  path_to_open, "r", options ^ PTHREADS_REPORT_ERRORS, NULL,
								  threaded_context, ce);

				if (threaded_stream) {
					stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
					stream->wrapper = threaded_wrapper;
					stream->flags |= PTHREADS_STREAM_FLAG_NO_BUFFER | PTHREADS_STREAM_FLAG_IS_DIR;
				}
			} else if (threaded_wrapper) {
				pthreads_stream_wrapper_log_error(threaded_wrapper, options ^ PTHREADS_REPORT_ERRORS, "not implemented");
			}
			MONITOR_UNLOCK(threaded_wrapper);
		}
	}
	if (threaded_stream == NULL && (options & PTHREADS_REPORT_ERRORS)) {
		pthreads_stream_display_wrapper_errors(threaded_wrapper, path, "failed to open dir");
	}
	pthreads_stream_tidy_wrapper_error_log(threaded_wrapper);

	return threaded_stream;
}
/* }}} */

/* {{{ _pthreads_stream_readdir */
pthreads_stream_dirent *_pthreads_stream_readdir(pthreads_stream_t *threaded_dirstream, pthreads_stream_dirent *ent) {
	if (sizeof(pthreads_stream_dirent) == pthreads_stream_read(threaded_dirstream, (char*)ent, sizeof(pthreads_stream_dirent))) {
		return ent;
	}
	return NULL;
}
/* }}} */

/* {{{ pthreads_stream_open_wrapper_ex */
pthreads_stream_t *_pthreads_stream_open_wrapper_ex(const char *path, const char *mode, int options,
		zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	pthreads_stream_wrapper_t *threaded_wrapper = NULL;
	pthreads_stream_wrapper *wrapper = NULL;
	const char *path_to_open;
	zend_string *resolved_path = NULL;
	char *copy_of_path = NULL;

	if (opened_path) {
		*opened_path = NULL;
	}

	if (!path || !*path) {
		php_error_docref(NULL, E_WARNING, "Filename cannot be empty");
		return NULL;
	}

	if (options & PTHREADS_USE_PATH) {
		resolved_path = zend_resolve_path(path, (int)strlen(path));
		if (resolved_path) {
			path = ZSTR_VAL(resolved_path);
			/* we've found this file, don't re-check include_path or run realpath */
			options |= PTHREADS_STREAM_ASSUME_REALPATH;
			options &= ~PTHREADS_USE_PATH;
		}
	}

	path_to_open = path;

	threaded_wrapper = pthreads_stream_locate_url_wrapper(path, &path_to_open, options);

	if(threaded_wrapper) {
		wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	}

	if (options & PTHREADS_STREAM_USE_URL && (!wrapper || !wrapper->is_url)) {
		php_error_docref(NULL, E_WARNING, "This function may only be used against URLs");
		if (resolved_path) {
			zend_string_release(resolved_path);
		}
		return NULL;
	}

	if (wrapper) {
		if (!wrapper->wops->stream_opener) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options ^ PTHREADS_REPORT_ERRORS,
					"wrapper does not support stream open");
		} else {
			if(MONITOR_LOCK(threaded_wrapper)) {
				threaded_stream = wrapper->wops->stream_opener(threaded_wrapper,
					path_to_open, mode, options ^ PTHREADS_REPORT_ERRORS,
					opened_path, threaded_context, ce);

				MONITOR_UNLOCK(threaded_wrapper);
			}
		}

		if (threaded_stream) {
			stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
			stream->wrapper = threaded_wrapper;
		}
	}

	if (threaded_stream) {
		if (opened_path && !*opened_path && resolved_path) {
			*opened_path = resolved_path;
			resolved_path = NULL;
		}

		if(stream_lock(threaded_stream)) {
			stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);

			if (stream->orig_path) {
				free(stream->orig_path);
			}
			copy_of_path = strdup(path);
			stream->orig_path = copy_of_path;

			stream_unlock(threaded_stream);
		}

		if (options & PTHREADS_STREAM_MUST_SEEK) {
			pthreads_stream_t *threaded_newstream;
			pthreads_stream *newstream;

			switch(pthreads_stream_make_seekable(threaded_stream, &threaded_newstream,
						(options & PTHREADS_STREAM_WILL_CAST)
							? PTHREADS_STREAM_PREFER_STDIO : PTHREADS_STREAM_NO_PREFERENCE)) {
				case PTHREADS_STREAM_UNCHANGED:
					if (resolved_path) {
						zend_string_release(resolved_path);
					}
					return threaded_stream;
				case PTHREADS_STREAM_RELEASED:
					newstream = PTHREADS_FETCH_STREAMS_STREAM(threaded_newstream);

					if(stream_lock(threaded_newstream)) {
						if (newstream->orig_path) {
							free(newstream->orig_path);
						}
						newstream->orig_path = strdup(path);

						stream_unlock(threaded_newstream);
					}
					if (resolved_path) {
						zend_string_release(resolved_path);
					}
					return threaded_newstream;
				default:
					pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
					threaded_stream = NULL;
					stream = NULL;
					if (options & PTHREADS_REPORT_ERRORS) {
						char *tmp = estrdup(path);
						php_strip_url_passwd(tmp);
						php_error_docref1(NULL, tmp, E_WARNING, "could not make seekable - %s",
								tmp);
						efree(tmp);

						options ^= PTHREADS_REPORT_ERRORS;
					}
			}
		}

		if(stream_lock(threaded_stream)) {
			if(stream->ops->seek && (stream->flags & PTHREADS_STREAM_FLAG_NO_SEEK) == 0 && strchr(mode, 'a') && stream->position == 0)
			{
				zend_off_t newpos = 0;

				/* if opened for append, we need to revise our idea of the initial file position */
				if (0 == stream->ops->seek(threaded_stream, 0, SEEK_CUR, &newpos)) {
					stream->position = newpos;
				}
			}
			stream_unlock(threaded_stream);
		}
	} else if(options & PTHREADS_REPORT_ERRORS) {
		pthreads_stream_display_wrapper_errors(threaded_wrapper, path, "failed to open stream");
		if (opened_path && *opened_path) {
			zend_string_release(*opened_path);
			*opened_path = NULL;
		}
	}
	pthreads_stream_tidy_wrapper_error_log(threaded_wrapper);

#if ZEND_DEBUG
	if (stream == NULL && copy_of_path != NULL) {
		free(copy_of_path);
	}
#endif
	if (resolved_path) {
		zend_string_release(resolved_path);
	}

	return threaded_stream;
}
/* }}} */

#endif
