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
#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#define HAVE_PTHREADS_STREAMS_TRANSPORTS_H

#ifdef PHP_WIN32
#include "config.w32.h"
#include <Ws2tcpip.h>
#endif

#if HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifndef HAVE_PTHREADS_HASH_H
#	include "src/hash.h"
#endif

#ifndef HAVE_PTHREADS_STREAM_GLOBALS_H
#	include "src/streams/streamglobals.h"
#endif

#define PTHREADS_STREAM_XPORT_CLIENT		0
#define PTHREADS_STREAM_XPORT_SERVER		1

#define PTHREADS_STREAM_XPORT_CONNECT		2
#define PTHREADS_STREAM_XPORT_BIND			4
#define PTHREADS_STREAM_XPORT_LISTEN		8
#define PTHREADS_STREAM_XPORT_CONNECT_ASYNC	16

typedef pthreads_stream_t *(pthreads_stream_transport_factory_func)(const char *proto, size_t protolen,
		const char *resourcename, size_t resourcenamelen,
		int options, int flags, struct timeval *timeout,
		pthreads_stream_context_t *threaded_context);
typedef pthreads_stream_transport_factory_func *pthreads_stream_transport_factory;

int pthreads_stream_xport_register(const char *protocol, pthreads_stream_transport_factory factory);
int pthreads_stream_xport_unregister(const char *protocol);

/* Open a client or server socket connection */
pthreads_stream_t *_pthreads_stream_xport_create(const char *name, size_t namelen, int options,
		int flags, struct timeval *timeout,
		pthreads_stream_context_t *threaded_context,
		zend_string **error_string,
		int *error_code);

#define pthreads_stream_xport_create(name, namelen, options, flags, timeout, context, estr, ecode) \
	_pthreads_stream_xport_create(name, namelen, options, flags, timeout, context, estr, ecode)

/* Bind the stream to a local address */
int pthreads_stream_xport_bind(pthreads_stream_t *threaded_stream,
		const char *name, size_t namelen,
		zend_string **error_text
		);

/* Connect to a remote address */
int pthreads_stream_xport_connect(pthreads_stream_t *threaded_stream,
		const char *name, size_t namelen,
		int asynchronous,
		struct timeval *timeout,
		zend_string **error_text,
		int *error_code
		);

/* Prepare to listen */
int pthreads_stream_xport_listen(pthreads_stream_t *threaded_stream,
		int backlog,
		zend_string **error_text
		);

/* Get the next client and their address as a string, or the underlying address
 * structure.  You must efree either of these if you request them */
int pthreads_stream_xport_accept(pthreads_stream_t *threaded_stream, pthreads_stream_t **threaded_client,
		zend_string **textaddr,
		void **addr, socklen_t *addrlen,
		struct timeval *timeout,
		zend_string **error_text
		);

/* Get the name of either the socket or it's peer */
int pthreads_stream_xport_get_name(pthreads_stream_t *threaded_stream, int want_peer,
		zend_string **textaddr,
		void **addr, socklen_t *addrlen
		);

enum pthreads_stream_xport_send_recv_flags {
	PTHREADS_STREAM_OOB = 1,
	PTHREADS_STREAM_PEEK = 2
};

/* Similar to recv() system call; read data from the stream, optionally
 * peeking, optionally retrieving OOB data */
int pthreads_stream_xport_recvfrom(pthreads_stream_t *threaded_stream, char *buf, size_t buflen,
		int flags, void **addr, socklen_t *addrlen,
		zend_string **textaddr);

/* Similar to send() system call; send data to the stream, optionally
 * sending it as OOB data */
int pthreads_stream_xport_sendto(pthreads_stream_t *threaded_stream, const char *buf, size_t buflen,
		int flags, void *addr, socklen_t addrlen);

typedef enum {
	PTHREADS_STREAM_SHUT_RD,
	PTHREADS_STREAM_SHUT_WR,
	PTHREADS_STREAM_SHUT_RDWR
} pthreads_stream_shutdown_t;

/* Similar to shutdown() system call; shut down part of a full-duplex connection */
int pthreads_stream_xport_shutdown(pthreads_stream_t *threaded_stream, pthreads_stream_shutdown_t how);


/* Structure definition for the set_option interface that the above functions wrap */

typedef struct _pthreads_stream_xport_param {
	enum {
		PTHREADS_STREAM_XPORT_OP_BIND, PTHREADS_STREAM_XPORT_OP_CONNECT,
		PTHREADS_STREAM_XPORT_OP_LISTEN, PTHREADS_STREAM_XPORT_OP_ACCEPT,
		PTHREADS_STREAM_XPORT_OP_CONNECT_ASYNC,
		PTHREADS_STREAM_XPORT_OP_GET_NAME,
		PTHREADS_STREAM_XPORT_OP_GET_PEER_NAME,
		PTHREADS_STREAM_XPORT_OP_RECV,
		PTHREADS_STREAM_XPORT_OP_SEND,
		PTHREADS_STREAM_XPORT_OP_SHUTDOWN
	} op;
	unsigned int want_addr:1;
	unsigned int want_textaddr:1;
	unsigned int want_errortext:1;
	unsigned int how:2;

	struct {
		char *name;
		size_t namelen;
		struct timeval *timeout;
		struct sockaddr *addr;
		char *buf;
		size_t buflen;
		socklen_t addrlen;
		int backlog;
		int flags;
	} inputs;
	struct {
		pthreads_stream_t *client;
		struct sockaddr *addr;
		socklen_t addrlen;
		zend_string *textaddr;
		zend_string *error_text;
		int returncode;
		int error_code;
	} outputs;
} pthreads_stream_xport_param;

/* Because both client and server streams use the same mechanisms
   for encryption we use the LSB to denote clients.
*/
typedef enum {
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv2_CLIENT = (1 << 1 | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv3_CLIENT = (1 << 2 | 1),
	/* v23 no longer negotiates SSL2 or SSL3 */
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_0_CLIENT = (1 << 3 | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_1_CLIENT = (1 << 4 | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_2_CLIENT = (1 << 5 | 1),
	/* TLS equates to TLS_ANY as of PHP 7.2 */
	PTHREADS_STREAM_CRYPTO_METHOD_TLS_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_TLS_ANY_CLIENT = ((1 << 3) | (1 << 4) | (1 << 5) | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_ANY_CLIENT = ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | 1),
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv2_SERVER = (1 << 1),
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv3_SERVER = (1 << 2),
	/* v23 no longer negotiates SSL2 or SSL3 */
	PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_SERVER = ((1 << 3) | (1 << 4) | (1 << 5)),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_0_SERVER = (1 << 3),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_1_SERVER = (1 << 4),
	PTHREADS_STREAM_CRYPTO_METHOD_TLSv1_2_SERVER = (1 << 5),
	/* TLS equates to TLS_ANY as of PHP 7.2 */
	PTHREADS_STREAM_CRYPTO_METHOD_TLS_SERVER = ((1 << 3) | (1 << 4) | (1 << 5)),
	PTHREADS_STREAM_CRYPTO_METHOD_TLS_ANY_SERVER = ((1 << 3) | (1 << 4) | (1 << 5)),
	PTHREADS_STREAM_CRYPTO_METHOD_ANY_SERVER = ((1 << 1) | (1 << 2) | (1 << 3) | (1 << 4) | (1 << 5))
} pthreads_stream_xport_crypt_method_t;

/* These functions provide crypto support on the underlying transport */

int pthreads_stream_xport_crypto_setup(pthreads_stream_t *threaded_stream, pthreads_stream_xport_crypt_method_t crypto_method, pthreads_stream_t *session_stream);
int pthreads_stream_xport_crypto_enable(pthreads_stream_t *threaded_stream, int activate);

typedef struct _pthreads_stream_xport_crypto_param {
	struct {
		pthreads_stream_t *session;
		int activate;
		pthreads_stream_xport_crypt_method_t method;
	} inputs;
	struct {
		int returncode;
	} outputs;
	enum {
		PTHREADS_STREAM_XPORT_CRYPTO_OP_SETUP,
		PTHREADS_STREAM_XPORT_CRYPTO_OP_ENABLE
	} op;
} pthreads_stream_xport_crypto_param;

pthreads_hashtable *pthreads_stream_xport_get_hash(void);
pthreads_stream_transport_factory_func pthreads_stream_generic_socket_factory;

#endif
