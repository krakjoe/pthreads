#ifndef HAVE_PTHREADS_SOCKET_SOCKADR_CONV_H
#define HAVE_PTHREADS_SOCKET_SOCKADR_CONV_H

#include <php_network.h>
#include "../socket.h" /* pthreads_socket_t */

#ifndef PHP_WIN32
# include <netinet/in.h>
#else
# include <Winsock2.h>
#endif


/*
 * Convert an IPv6 literal or a hostname info a sockaddr_in6.
 * The IPv6 literal can be a IPv4 mapped address (like ::ffff:127.0.0.1).
 * If the hostname yields no IPv6 addresses, a mapped IPv4 address may be returned (AI_V4MAPPED)
 */
int pthreads_socket_set_inet6_addr(pthreads_socket_t *sock, struct sockaddr_in6 *sin, zend_string *address);

/*
 * Convert an IPv4 literal or a hostname into a sockaddr_in.
 */
int pthreads_socket_set_inet_addr(pthreads_socket_t *sock, struct sockaddr_in *sin, zend_string *address);

/*
 * Calls either php_set_inet6_addr() or php_set_inet_addr(), depending on the type of the socket.
 */
int pthreads_socket_set_inet46_addr(pthreads_socket_t *sock, php_sockaddr_storage *ss, socklen_t *ss_len, char *string);

#endif
