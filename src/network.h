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
#ifndef HAVE_PTHREADS_NETWORK_H
#define HAVE_PTHREADS_NETWORK_H

#ifndef _PHP_NETWORK_H
#	include <php_network.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

#define PTHREADS_STREAM_SOCKOP_NONE                (1 << 0)
#define PTHREADS_STREAM_SOCKOP_SO_REUSEPORT        (1 << 1)
#define PTHREADS_STREAM_SOCKOP_SO_BROADCAST        (1 << 2)
#define PTHREADS_STREAM_SOCKOP_IPV6_V6ONLY         (1 << 3)
#define PTHREADS_STREAM_SOCKOP_IPV6_V6ONLY_ENABLED (1 << 4)
#define PTHREADS_STREAM_SOCKOP_TCP_NODELAY         (1 << 5)

struct _pthreads_netstream_data_t	{
	php_socket_t socket;
	char is_blocked;
	struct timeval timeout;
	char timeout_event;
	size_t ownsize;
};
typedef struct _pthreads_netstream_data_t pthreads_netstream_data_t;

extern const pthreads_stream_ops pthreads_stream_socket_ops;
extern const pthreads_stream_ops pthreads_stream_generic_socket_ops;

#define PTHREADS_STREAM_IS_SOCKET	(&pthreads_stream_socket_ops)


pthreads_stream_t *_pthreads_stream_sock_open_from_socket(php_socket_t socket, zend_class_entry *ce);
/* open a connection to a host using php_hostconnect and return a stream */
pthreads_stream_t *_pthreads_stream_sock_open_host(const char *host, unsigned short port,
		int socktype, struct timeval *timeout);

#define pthreads_stream_sock_open_from_socket(socket)	_pthreads_stream_sock_open_from_socket((socket), NULL)
#define pthreads_stream_sock_open_host(host, port, socktype, timeout)	_pthreads_stream_sock_open_host((host), (port), (socktype), (timeout))

#endif
