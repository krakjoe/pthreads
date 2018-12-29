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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER
#define HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_MEMORY_H
#	include <src/streams/memory.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER_H
#	include <src/streams/wrappers/fopen_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_NETWORK_H
#	include <src/network.h>
#endif

/* {{{ */
static size_t pthreads_stream_output_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	PHPWRITE(buf, count);
	return count;
}
/* }}} */

/* {{{ */
static size_t pthreads_stream_output_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	stream->eof = 1;
	return 0;
}
/* }}} */

/* {{{ */
static int pthreads_stream_output_close(pthreads_stream_t *threaded_stream, int close_handle) {
	return 0;
}
/* }}} */

/* {{{ */
static void pthreads_stream_output_free(pthreads_stream_t *threaded_stream, int close_handle) { }
/* }}} */

const pthreads_stream_ops pthreads_stream_output_ops = {
	pthreads_stream_output_write,
	pthreads_stream_output_read,
	pthreads_stream_output_close,
	pthreads_stream_output_free,
	NULL, /* flush */
	"Output",
	NULL, /* seek */
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

/* {{{ */
static void pthreads_stream_apply_filter_list(pthreads_stream_t *threaded_stream, char *filterlist, int read_chain, int write_chain) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	char *p, *token = NULL;
	pthreads_stream_filter_t *temp_filter;

	p = php_strtok_r(filterlist, "|", &token);
	while (p) {
		php_url_decode(p, strlen(p));
		if (read_chain) {
			if ((temp_filter = pthreads_stream_filter_create(p, NULL))) {
				pthreads_stream_filter_append(pthreads_stream_get_readfilters(threaded_stream), temp_filter);
			} else {
				php_error_docref(NULL, E_WARNING, "Unable to create filter (%s)", p);
			}
		}
		if (write_chain) {
			if ((temp_filter = pthreads_stream_filter_create(p, NULL))) {
				pthreads_stream_filter_append(pthreads_stream_get_writefilters(threaded_stream), temp_filter);
			} else {
				php_error_docref(NULL, E_WARNING, "Unable to create filter (%s)", p);
			}
		}
		p = php_strtok_r(NULL, "|", &token);
	}
}
/* }}} */

/* {{{ */
pthreads_stream_t * pthreads_stream_url_wrap_php(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
									 zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	int fd = -1;
	int mode_rw = 0;
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	char *p, *token, *pathdup;
	zend_long max_memory;
	FILE *file = NULL;
#ifdef PHP_WIN32
	int pipe_requested = 0;
#endif

	if (!strncasecmp(path, "php://", 6)) {
		path += 6;
	}

	if (!strncasecmp(path, "temp", 4)) {
		path += 4;
		max_memory = PTHREADS_STREAM_MAX_MEM;
		if (!strncasecmp(path, "/maxmemory:", 11)) {
			path += 11;
			max_memory = ZEND_STRTOL(path, NULL, 10);
			if (max_memory < 0) {
				zend_throw_error(NULL, "Max memory must be >= 0");
				return NULL;
			}
		}
		mode_rw = pthreads_stream_mode_from_str(mode);
		return pthreads_stream_temp_create(mode_rw, max_memory, ce);
	}

	if (!strcasecmp(path, "memory")) {
		mode_rw = pthreads_stream_mode_from_str(mode);
		return pthreads_stream_memory_create(mode_rw, ce);
	}

	if (!strcasecmp(path, "output")) {
		return PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_output_ops, NULL, "wb", ce);
	}

	if (!strcasecmp(path, "stdin")) {
		if ((options & PTHREADS_STREAM_OPEN_FOR_INCLUDE) && !PG(allow_url_include) ) {
			if (options & PTHREADS_REPORT_ERRORS) {
				php_error_docref(NULL, E_WARNING, "URL file-access is disabled in the server configuration");
			}
			return NULL;
		}
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_in = 0;
			fd = STDIN_FILENO;
			if (cli_in) {
				fd = dup(fd);
			} else {
				cli_in = 1;
				file = stdin;
			}
		} else {
			fd = dup(STDIN_FILENO);
		}
#ifdef PHP_WIN32
		pipe_requested = 1;
#endif
	} else if (!strcasecmp(path, "stdout")) {
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_out = 0;
			fd = STDOUT_FILENO;
			if (cli_out) {
				fd = dup(fd);
			} else {
				cli_out = 1;
				file = stdout;
			}
		} else {
			fd = dup(STDOUT_FILENO);
		}
#ifdef PHP_WIN32
		pipe_requested = 1;
#endif
	} else if (!strcasecmp(path, "stderr")) {
		if (!strcmp(sapi_module.name, "cli")) {
			static int cli_err = 0;
			fd = STDERR_FILENO;
			if (cli_err) {
				fd = dup(fd);
			} else {
				cli_err = 1;
				file = stderr;
			}
		} else {
			fd = dup(STDERR_FILENO);
		}
#ifdef PHP_WIN32
		pipe_requested = 1;
#endif
	} else if (!strncasecmp(path, "fd/", 3)) {
		const char *start;
		char       *end;
		zend_long  fildes_ori;
		int		   dtablesize;

		if (strcmp(sapi_module.name, "cli")) {
			if (options & PTHREADS_REPORT_ERRORS) {
				php_error_docref(NULL, E_WARNING, "Direct access to file descriptors is only available from command-line PHP");
			}
			return NULL;
		}

		if ((options & PTHREADS_STREAM_OPEN_FOR_INCLUDE) && !PG(allow_url_include)) {
			if (options & PTHREADS_REPORT_ERRORS) {
				php_error_docref(NULL, E_WARNING, "URL file-access is disabled in the server configuration");
			}
			return NULL;
		}

		start = &path[3];
		fildes_ori = ZEND_STRTOL(start, &end, 10);
		if (end == start || *end != '\0') {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options,
				"php://fd/ stream must be specified in the form php://fd/<orig fd>");
			return NULL;
		}

#if HAVE_UNISTD_H
		dtablesize = getdtablesize();
#else
		dtablesize = INT_MAX;
#endif

		if (fildes_ori < 0 || fildes_ori >= dtablesize) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options,
				"The file descriptors must be non-negative numbers smaller than %d", dtablesize);
			return NULL;
		}

		fd = dup((int)fildes_ori);
		if (fd == -1) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options,
				"Error duping file descriptor " ZEND_LONG_FMT "; possibly it doesn't exist: "
				"[%d]: %s", fildes_ori, errno, strerror(errno));
			return NULL;
		}
	} else if (!strncasecmp(path, "filter/", 7)) {
		/* Save time/memory when chain isn't specified */
		if (strchr(mode, 'r') || strchr(mode, '+')) {
			mode_rw |= PTHREADS_STREAM_FILTER_READ;
		}
		if (strchr(mode, 'w') || strchr(mode, '+') || strchr(mode, 'a')) {
			mode_rw |= PTHREADS_STREAM_FILTER_WRITE;
		}
		pathdup = estrndup(path + 6, strlen(path + 6));
		p = strstr(pathdup, "/resource=");
		if (!p) {
			zend_throw_error(NULL, "No URL resource specified");
			efree(pathdup);
			return NULL;
		}

		if (!(threaded_stream = pthreads_stream_open_wrapper(p + 10, mode, options, opened_path))) {
			efree(pathdup);
			return NULL;
		}

		*p = '\0';

		p = php_strtok_r(pathdup + 1, "/", &token);
		while (p) {
			if (!strncasecmp(p, "read=", 5)) {
				pthreads_stream_apply_filter_list(threaded_stream, p + 5, 1, 0);
			} else if (!strncasecmp(p, "write=", 6)) {
				pthreads_stream_apply_filter_list(threaded_stream, p + 6, 0, 1);
			} else {
				pthreads_stream_apply_filter_list(threaded_stream, p, mode_rw & PTHREADS_STREAM_FILTER_READ, mode_rw & PTHREADS_STREAM_FILTER_WRITE);
			}
			p = php_strtok_r(NULL, "/", &token);
		}
		efree(pathdup);

		return threaded_stream;
	} else {
		/* invalid php://thingy */
		php_error_docref(NULL, E_WARNING, "Invalid php:// URL specified");
		return NULL;
	}

	/* must be stdin, stderr or stdout */
	if (fd == -1)	{
		/* failed to dup */
		return NULL;
	}

#if defined(S_IFSOCK) && !defined(PHP_WIN32)
	do {
		zend_stat_t st;
		memset(&st, 0, sizeof(st));
		if (zend_fstat(fd, &st) == 0 && (st.st_mode & S_IFMT) == S_IFSOCK) {
			threaded_stream = _pthreads_stream_sock_open_from_socket(fd, pthreads_socket_stream_entry);
			if (threaded_stream) {
				stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
				stream->ops = &pthreads_stream_socket_ops;
				return threaded_stream;
			}
		}
	} while (0);
#endif

	if (file) {
		threaded_stream = pthreads_stream_fopen_from_file(file, mode);
	} else {
		threaded_stream = pthreads_stream_fopen_from_fd(fd, mode);
		if (threaded_stream == NULL) {
			close(fd);
		}
	}

#ifdef PHP_WIN32
	if (pipe_requested && threaded_stream && threaded_context) {
		zval *blocking_pipes = pthreads_stream_context_get_option(threaded_context, "pipe", "blocking");
		if (blocking_pipes) {
			pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_PIPE_BLOCKING, zval_get_long(blocking_pipes), NULL);
		}
	}
#endif
	return threaded_stream;
}
/* }}} */

const pthreads_stream_wrapper_ops pthreads_stdio_wops = {
	pthreads_stream_url_wrap_php,
	NULL, /* close */ NULL, /* fstat */
	NULL, /* stat */ NULL, /* opendir */
	"PHP",
	NULL, /* unlink */ NULL, /* rename */
	NULL, /* mkdir */ NULL, /* rmdir */
	NULL
};

#endif
