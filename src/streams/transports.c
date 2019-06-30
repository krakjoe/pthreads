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
#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS
#define HAVE_PTHREADS_STREAMS_TRANSPORTS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_HASH_H
#	include <src/hash.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#	include "src/streams/transports.h"
#endif

int pthreads_init_stream_transports() {
	return (pthreads_stream_xport_register("tcp", pthreads_stream_generic_socket_factory) == SUCCESS
			&&
			pthreads_stream_xport_register("udp", pthreads_stream_generic_socket_factory) == SUCCESS
#if defined(AF_UNIX) && !(defined(PHP_WIN32) || defined(__riscos__))
			&&
			pthreads_stream_xport_register("unix", pthreads_stream_generic_socket_factory) == SUCCESS
			&&
			pthreads_stream_xport_register("udg", pthreads_stream_generic_socket_factory) == SUCCESS
#endif
		) ? SUCCESS : FAILURE;
}

pthreads_hashtable *pthreads_stream_xport_get_hash(void)
{
	return &PTHREADS_STREAMG(xport_hash);
}

int pthreads_stream_xport_register(const char *protocol, pthreads_stream_transport_factory factory)
{
	pthreads_hashtable *xport_hash = &PTHREADS_STREAMG(xport_hash);
	zend_string *str = zend_string_init_interned(protocol, strlen(protocol), 1);

	if(MONITOR_LOCK(xport_hash)) {
		zend_hash_update_ptr(&xport_hash->ht, str, factory);
		MONITOR_UNLOCK(xport_hash);
	}
	return SUCCESS;
}

int pthreads_stream_xport_unregister(const char *protocol)
{
	pthreads_hashtable *xport_hash = &PTHREADS_STREAMG(xport_hash);
	int result = FAILURE;

	if(MONITOR_LOCK(xport_hash)) {
		result = zend_hash_str_del(&xport_hash->ht, protocol, strlen(protocol));
		MONITOR_UNLOCK(xport_hash);
	}
	return result;
}

#define PTHREADS_ERR_REPORT(out_err, fmt, arg) \
	if (out_err) { *out_err = strpprintf(0, fmt, arg); } \
	else { php_error_docref(NULL, E_WARNING, fmt, arg); }

#define PTHREADS_ERR_RETURN(out_err, local_err, fmt) \
	if (out_err) { *out_err = local_err; } \
	else { php_error_docref(NULL, E_WARNING, fmt, local_err ? ZSTR_VAL(local_err) : "Unspecified error"); \
		if (local_err) { zend_string_release(local_err); local_err = NULL; } \
	}

pthreads_stream_t *_pthreads_stream_xport_create(const char *name, size_t namelen, int options,
		int flags,
		struct timeval *timeout,
		pthreads_stream_context_t *threaded_context,
		zend_string **error_string,
		int *error_code)
{
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	pthreads_stream_transport_factory factory = NULL;
	const char *p, *protocol = NULL;
	size_t n = 0;
	int failed = 0;
	zend_string *error_text = NULL;
	struct timeval default_timeout = { 0, 0 };

	default_timeout.tv_sec = FG(default_socket_timeout);

	if (timeout == NULL) {
		timeout = &default_timeout;
	}

	for (p = name; isalnum((int)*p) || *p == '+' || *p == '-' || *p == '.'; p++) {
		n++;
	}

	if ((*p == ':') && (n > 1) && !strncmp("://", p, 3)) {
		protocol = name;
		name = p + 3;
		namelen -= n + 3;
	} else {
		protocol = "tcp";
		n = 3;
	}

	if (protocol) {
		pthreads_hashtable *xport_hash = &PTHREADS_STREAMG(xport_hash);

		if(MONITOR_LOCK(xport_hash)) {
			factory = zend_hash_str_find_ptr(&xport_hash->ht, protocol, n);
			MONITOR_UNLOCK(xport_hash);
		}

		if (NULL == factory) {
			char wrapper_name[32];

			if (n >= sizeof(wrapper_name))
				n = sizeof(wrapper_name) - 1;
			PHP_STRLCPY(wrapper_name, protocol, sizeof(wrapper_name), n);

			ERR_REPORT(error_string, "Unable to find the socket transport \"%s\" - did you forget to enable it when you configured PHP?",
					wrapper_name);

			return NULL;
		}
	}

	if (factory == NULL) {
		/* should never happen */
		php_error_docref(NULL, E_WARNING, "Could not find a factory !?");
		return NULL;
	}
	threaded_stream = (factory)(protocol, n, (char*)name, namelen, options, flags, timeout, threaded_context);

	if (threaded_stream) {
		pthreads_stream_context_set(threaded_stream, threaded_context);

		if ((flags & PTHREADS_STREAM_XPORT_SERVER) == 0) {
			/* client */

			if (flags & (PTHREADS_STREAM_XPORT_CONNECT|PTHREADS_STREAM_XPORT_CONNECT_ASYNC)) {
				if (-1 == pthreads_stream_xport_connect(threaded_stream, name, namelen,
							flags & PTHREADS_STREAM_XPORT_CONNECT_ASYNC ? 1 : 0,
							timeout, &error_text, error_code)) {

					PTHREADS_ERR_RETURN(error_string, error_text, "connect() failed: %s");

					failed = 1;
				}
			}

		} else {
			/* server */
			if (flags & PTHREADS_STREAM_XPORT_BIND) {
				if (0 != pthreads_stream_xport_bind(threaded_stream, name, namelen, &error_text)) {
					PTHREADS_ERR_RETURN(error_string, error_text, "bind() failed: %s");
					failed = 1;
				} else if (flags & PTHREADS_STREAM_XPORT_LISTEN) {
					zval *zbacklog = NULL;
					int backlog = 32;
					stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
					threaded_context = pthreads_stream_get_context(threaded_stream);

					if (threaded_context && (zbacklog = pthreads_stream_context_get_option(threaded_context, "socket", "backlog")) != NULL) {
						backlog = zval_get_long(zbacklog);
					}

					if (0 != pthreads_stream_xport_listen(threaded_stream, backlog, &error_text)) {
						PTHREADS_ERR_RETURN(error_string, error_text, "listen() failed: %s");
						failed = 1;
					}
				}
			}
		}
	}

	if (failed) {
		/* failure means that they don't get a stream to play with */
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
		threaded_stream = NULL;
	}

	return threaded_stream;
}

/* Bind the stream to a local address */
int pthreads_stream_xport_bind(pthreads_stream_t *threaded_stream,
		const char *name, size_t namelen,
		zend_string **error_text
		)
{
	pthreads_stream_xport_param param;
	int ret;

	memset(&param, 0, sizeof(param));
	param.op = PTHREADS_STREAM_XPORT_OP_BIND;
	param.inputs.name = (char*)name;
	param.inputs.namelen = namelen;
	param.want_errortext = error_text ? 1 : 0;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		if (error_text) {
			*error_text = param.outputs.error_text;
		}
		return param.outputs.returncode;
	}

	return ret;
}

/* Connect to a remote address */
int pthreads_stream_xport_connect(pthreads_stream_t *threaded_stream,
		const char *name, size_t namelen,
		int asynchronous,
		struct timeval *timeout,
		zend_string **error_text,
		int *error_code
		)
{
	pthreads_stream_xport_param param;
	int ret;

	memset(&param, 0, sizeof(param));
	param.op = asynchronous ? PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC : PTHREADS_STREAM_XPORT_OP_CONNECT;
	param.inputs.name = (char*)name;
	param.inputs.namelen = namelen;
	param.inputs.timeout = timeout;

	param.want_errortext = error_text ? 1 : 0;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		if (error_text) {
			*error_text = param.outputs.error_text;
		}
		if (error_code) {
			*error_code = param.outputs.error_code;
		}
		return param.outputs.returncode;
	}

	return ret;
}

/* Prepare to listen */
int pthreads_stream_xport_listen(pthreads_stream_t *threaded_stream, int backlog, zend_string **error_text)
{
	pthreads_stream_xport_param param;
	int ret;

	memset(&param, 0, sizeof(param));
	param.op = PTHREADS_STREAM_XPORT_OP_LISTEN;
	param.inputs.backlog = backlog;
	param.want_errortext = error_text ? 1 : 0;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		if (error_text) {
			*error_text = param.outputs.error_text;
		}

		return param.outputs.returncode;
	}

	return ret;
}

/* Get the next client and their address (as a string) */
int pthreads_stream_xport_accept(pthreads_stream_t *threaded_stream, pthreads_stream_t **threaded_client,
		zend_string **textaddr,
		void **addr, socklen_t *addrlen,
		struct timeval *timeout,
		zend_string **error_text
		)
{
	pthreads_stream_xport_param param;
	int ret;

	memset(&param, 0, sizeof(param));

	param.op = PTHREADS_STREAM_XPORT_OP_ACCEPT;
	param.inputs.timeout = timeout;
	param.want_addr = addr ? 1 : 0;
	param.want_textaddr = textaddr ? 1 : 0;
	param.want_errortext = error_text ? 1 : 0;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		*threaded_client = param.outputs.client;
		if (addr) {
			*addr = param.outputs.addr;
			*addrlen = param.outputs.addrlen;
		}
		if (textaddr) {
			*textaddr = param.outputs.textaddr;
		}
		if (error_text) {
			*error_text = param.outputs.error_text;
		}

		return param.outputs.returncode;
	}
	return ret;
}

int pthreads_stream_xport_get_name(pthreads_stream_t *threaded_stream, int want_peer, zend_string **textaddr, void **addr, socklen_t *addrlen ) {
	pthreads_stream_xport_param param;
	int ret;

	memset(&param, 0, sizeof(param));

	param.op = want_peer ? PTHREADS_STREAM_XPORT_OP_GET_PEER_NAME : PTHREADS_STREAM_XPORT_OP_GET_NAME;
	param.want_addr = addr ? 1 : 0;
	param.want_textaddr = textaddr ? 1 : 0;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		if (addr) {
			*addr = param.outputs.addr;
			*addrlen = param.outputs.addrlen;
		}
		if (textaddr) {
			*textaddr = param.outputs.textaddr;
		}

		return param.outputs.returncode;
	}
	return ret;
}

int pthreads_stream_xport_crypto_setup(pthreads_stream_t *threaded_stream, pthreads_stream_xport_crypt_method_t crypto_method, pthreads_stream_t *threaded_session_stream) {
	pthreads_stream_xport_crypto_param param;
	int ret;

	memset(&param, 0, sizeof(param));
	param.op = PTHREADS_STREAM_XPORT_CRYPTO_OP_SETUP;
	param.inputs.method = crypto_method;
	param.inputs.session = threaded_session_stream;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_CRYPTO_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		return param.outputs.returncode;
	}

	php_error_docref("streams.crypto", E_WARNING, "this stream does not support SSL/crypto");

	return ret;
}

int pthreads_stream_xport_crypto_enable(pthreads_stream_t *threaded_stream, int activate) {
	pthreads_stream_xport_crypto_param param;
	int ret;

	memset(&param, 0, sizeof(param));
	param.op = PTHREADS_STREAM_XPORT_CRYPTO_OP_ENABLE;
	param.inputs.activate = activate;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_CRYPTO_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		return param.outputs.returncode;
	}

	php_error_docref("streams.crypto", E_WARNING, "this stream does not support SSL/crypto");

	return ret;
}

/* Similar to recv() system call; read data from the stream, optionally
 * peeking, optionally retrieving OOB data */
int pthreads_stream_xport_recvfrom(pthreads_stream_t *threaded_stream, char *buf, size_t buflen, int flags, void **addr, socklen_t *addrlen, zend_string **textaddr)
{
	pthreads_stream_xport_param param;
	int ret = 0;
	int recvd_len = 0;
#if 0
	int oob;

	if (flags == 0 && addr == NULL) {
		return pthreads_stream_read(threaded_stream, buf, buflen);
	}

	if (pthreads_chain_has_head(pthreads_stream_get_readfilters(threaded_stream))) {
		php_error_docref(NULL, E_WARNING, "cannot peek or fetch OOB data from a filtered stream");
		return -1;
	}

	oob = (flags & PTHREADS_STREAM_OOB) == PTHREADS_STREAM_OOB;

	if (!oob && addr == NULL) {
		/* must be peeking at regular data; copy content from the buffer
		 * first, then adjust the pointer/len before handing off to the
		 * stream */
		recvd_len = threaded_stream->writepos - threaded_stream->readpos;
		if (recvd_len > buflen) {
			recvd_len = buflen;
		}
		if (recvd_len) {
			memcpy(buf, threaded_stream->readbuf, recvd_len);
			buf += recvd_len;
			buflen -= recvd_len;
		}
		/* if we filled their buffer, return */
		if (buflen == 0) {
			return recvd_len;
		}
	}
#endif

	/* otherwise, we are going to bypass the buffer */

	memset(&param, 0, sizeof(param));

	param.op = PTHREADS_STREAM_XPORT_OP_RECV;
	param.want_addr = addr ? 1 : 0;
	param.want_textaddr = textaddr ? 1 : 0;
	param.inputs.buf = buf;
	param.inputs.buflen = buflen;
	param.inputs.flags = flags;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		if (addr) {
			*addr = param.outputs.addr;
			*addrlen = param.outputs.addrlen;
		}
		if (textaddr) {
			*textaddr = param.outputs.textaddr;
		}
		return recvd_len + param.outputs.returncode;
	}
	return recvd_len ? recvd_len : -1;
}

/* Similar to send() system call; send data to the stream, optionally
 * sending it as OOB data */
int pthreads_stream_xport_sendto(pthreads_stream_t *threaded_stream, const char *buf, size_t buflen,
		int flags, void *addr, socklen_t addrlen) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_xport_param param;
	int ret = 0;
	int oob;

#if 0
	if (flags == 0 && addr == NULL) {
		return pthreads_stream_write(threaded_stream, buf, buflen);
	}
#endif

	if(stream_lock(threaded_stream)) {
		oob = (flags & PTHREADS_STREAM_OOB) == PTHREADS_STREAM_OOB;

		if ((oob || addr) && pthreads_chain_has_head(pthreads_stream_get_writefilters(threaded_stream))) {
			stream_unlock(threaded_stream);
			php_error_docref(NULL, E_WARNING, "cannot write OOB data, or data to a targeted address on a filtered stream");
			return -1;
		}

		memset(&param, 0, sizeof(param));

		param.op = PTHREADS_STREAM_XPORT_OP_SEND;
		param.want_addr = addr ? 1 : 0;
		param.inputs.buf = (char*)buf;
		param.inputs.buflen = buflen;
		param.inputs.flags = flags;
		param.inputs.addr = addr;
		param.inputs.addrlen = addrlen;

		ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

		if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
			stream_unlock(threaded_stream);
			return param.outputs.returncode;
		}
		stream_unlock(threaded_stream);
	}
	return -1;
}

/* Similar to shutdown() system call; shut down part of a full-duplex connection */
int pthreads_stream_xport_shutdown(pthreads_stream_t *threaded_stream, pthreads_stream_shutdown_t how)
{
	pthreads_stream_xport_param param;
	int ret = 0;

	memset(&param, 0, sizeof(param));

	param.op = PTHREADS_STREAM_XPORT_OP_SHUTDOWN;
	param.how = how;

	ret = pthreads_stream_set_option(threaded_stream, PTHREADS_STREAM_OPTION_XPORT_API, 0, &param);

	if (ret == PTHREADS_STREAM_OPTION_RETURN_OK) {
		return param.outputs.returncode;
	}
	return -1;
}

#endif
