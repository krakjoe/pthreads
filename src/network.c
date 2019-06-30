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
#ifndef HAVE_PTHREADS_NETWORK
#define HAVE_PTHREADS_NETWORK

#ifndef HAVE_PTHREADS_NETWORK_H
#	include <src/network.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#	include <src/streams/transports.h>
#endif

pthreads_stream_t *_pthreads_stream_sock_open_from_socket(php_socket_t socket, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream;
	pthreads_stream *stream;
	pthreads_netstream_data_t *sock;

	sock = malloc(sizeof(pthreads_netstream_data_t));
	memset(sock, 0, sizeof(pthreads_netstream_data_t));

	sock->is_blocked = 1;
	sock->timeout.tv_sec = FG(default_socket_timeout);
	sock->timeout.tv_usec = 0;
	sock->socket = socket;

	threaded_stream = PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_generic_socket_ops, sock, "r+", ce);

	if (threaded_stream == NULL) {
		free(sock);
	} else {
		stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
		stream->flags |= PTHREADS_STREAM_FLAG_AVOID_BLOCKING;
	}

	return threaded_stream;
}

pthreads_stream_t *_pthreads_stream_sock_open_host(const char *host, unsigned short port,
		int socktype, struct timeval *timeout) {
	char *res;
	zend_long reslen;
	pthreads_stream_t *threaded_stream;

	reslen = spprintf(&res, 0, "tcp://%s:%d", host, port);

	threaded_stream = pthreads_stream_xport_create(res, reslen, PTHREADS_REPORT_ERRORS,
			PTHREADS_STREAM_XPORT_CLIENT | PTHREADS_STREAM_XPORT_CONNECT, timeout, NULL, NULL, NULL);

	efree(res);

	return threaded_stream;
}

#endif
