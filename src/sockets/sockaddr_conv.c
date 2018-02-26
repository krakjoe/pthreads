#include <php.h>
#include <php_network.h>
#include "../socket.h"

#ifdef PHP_WIN32
#include "windows_common.h"
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

extern int pthreads_socket_string_to_if_index(const char *val, unsigned *out);


#if HAVE_IPV6
static inline zend_bool pthreads_socket_set_inet6_addr(pthreads_socket_t *sock, struct sockaddr_in6 *sin, zend_string *address) {
	struct in6_addr tmp;
#if HAVE_GETADDRINFO
	struct addrinfo hints;
	struct addrinfo *addrinfo = NULL;
#endif
	char *scope = strchr(ZSTR_VAL(address), '%');

	if (inet_pton(AF_INET6, ZSTR_VAL(address), &tmp)) {
		memcpy(&(sin->sin6_addr.s6_addr), &(tmp.s6_addr), sizeof(struct in6_addr));
	} else {
#if HAVE_GETADDRINFO
		memset(&hints, 0, sizeof(struct addrinfo));

		hints.ai_family = AF_INET6;
#if HAVE_AI_V4MAPPED
		hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
#else
		hints.ai_flags = AI_ADDRCONFIG;
#endif
		getaddrinfo(ZSTR_VAL(address), NULL, &hints, &addrinfo);
		if (!addrinfo) {
			PTHREADS_SOCKET_ERROR(sock, "Host lookup failed", errno);
			return 0;
		}

		if (addrinfo->ai_family != PF_INET6 || addrinfo->ai_addrlen != sizeof(struct sockaddr_in6)) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Host lookup failed: Non AF_INET6 domain returned on AF_INET6 socket");

			freeaddrinfo(addrinfo);
			return 0;
		}

		memcpy(&(sin->sin6_addr.s6_addr), ((struct sockaddr_in6*)(addrinfo->ai_addr))->sin6_addr.s6_addr, sizeof(struct in6_addr));
		freeaddrinfo(addrinfo);
#else
		/* No IPv6 specific hostname resolution is available on this system? */
		zend_throw_exception_ex(spl_ce_RuntimeException, 0, "Host lookup failed: getaddrinfo() not available on this system");

		return 0;
#endif

		if (scope++) {
			zend_long lval = 0;
			double dval = 0;
			unsigned scope_id = 0;

			if (IS_LONG == is_numeric_string(scope, strlen(scope), &lval, &dval, 0)) {
				if (lval > 0 && lval <= UINT_MAX) {
					scope_id = lval;
				} else {
					pthreads_socket_string_to_if_index(scope, &scope_id);
				}
				sin->sin6_scope_id = scope_id;
			}
		}
	}

	return 1;
}
#endif

static inline zend_bool pthreads_socket_set_inet_addr(pthreads_socket_t *sock, struct sockaddr_in *sin, zend_string *address) {
	struct in_addr tmp;
	struct hostent *hentry;

	if (inet_aton(ZSTR_VAL(address), &tmp)) {
		sin->sin_addr.s_addr = tmp.s_addr;
	} else {
		if (ZSTR_LEN(address) > MAXFQDNLEN || !(hentry = php_network_gethostbyname(ZSTR_VAL(address)))) {
			PTHREADS_SOCKET_ERROR(sock, "Host lookup failed", errno);

			return 0;
		}

		if (hentry->h_addrtype != AF_INET) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Host lookup failed: Non AF_INET domain returned on AF_INET socket");
			return 0;
		}

		memcpy(&(sin->sin_addr.s_addr), hentry->h_addr_list[0], hentry->h_length);
	}

	return 1;
}

/* Sets addr by hostname or by ip in string form (AF_INET or AF_INET6,
 * depending on the socket) */
int pthreads_socket_set_inet46_addr(pthreads_socket_t *sock, php_sockaddr_storage *ss, socklen_t *ss_len, char *string) /* {{{ */
{
	if (sock->domain == AF_INET) {
		struct sockaddr_in t = {0};
		if (pthreads_socket_set_inet_addr(&t, string, sock)) {
			memcpy(ss, &t, sizeof t);
			ss->ss_family = AF_INET;
			*ss_len = sizeof(t);
			return 1;
		}
	}
#if HAVE_IPV6
	else if (sock->domain == AF_INET6) {
		struct sockaddr_in6 t = {0};
		if (pthreads_socket_set_inet6_addr(&t, string, sock)) {
			memcpy(ss, &t, sizeof t);
			ss->ss_family = AF_INET6;
			*ss_len = sizeof(t);
			return 1;
		}
	}
#endif
	else {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
							"IP address used in the context of an unexpected type of socket");
	}
	return 0;
}
