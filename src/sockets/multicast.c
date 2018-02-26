

#ifndef HAVE_PTHREADS_SOCKET_MULTICAST
#define HAVE_PTHREADS_SOCKET_MULTICAST

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifdef PHP_WIN32
# include "windows_common.h"
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "../socket.h"
#include "multicast.h"
#include "conversions.h"
#include "sockaddr_conv.h"
#include "main/php_network.h"

enum source_op {
	JOIN_SOURCE,
	LEAVE_SOURCE,
	BLOCK_SOURCE,
	UNBLOCK_SOURCE
};

static int _pthreads_mcast_join_leave(pthreads_socket_t *sock, int level, struct sockaddr *group, socklen_t group_len, unsigned int if_index, int join);
#ifdef HAS_MCAST_EXT
static int _pthreads_mcast_source_op(pthreads_socket_t *sock, int level, struct sockaddr *group, socklen_t group_len, struct sockaddr *source, socklen_t source_len, unsigned int if_index, enum source_op sop);
#endif

#ifdef RFC3678_API
static int _pthreads_source_op_to_rfc3678_op(enum source_op sop);
#elif HAS_MCAST_EXT
static const char *_pthreads_source_op_to_string(enum source_op sop);
static int _pthreads_source_op_to_ipv4_op(enum source_op sop);
#endif

static inline int pthreads_socket_string_to_if_index(const char *val, unsigned *out) {
#if HAVE_IF_NAMETOINDEX
	unsigned int ind = if_nametoindex(val);

	if (ind != 0) {
		*out = ind;
		return SUCCESS;
	}

	return FAILURE;
#else
	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
						"this platform does not support looking up an interface by "
						"name, an integer interface index must be supplied instead");
	return FAILURE;
#endif
}

static int pthreads_get_if_index_from_zval(zval *val, unsigned *out)
{
	int ret;

	if (Z_TYPE_P(val) == IS_LONG) {
		if (Z_LVAL_P(val) < 0 || (zend_ulong)Z_LVAL_P(val) > UINT_MAX) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
									"the interface index cannot be negative or larger than %u;"
									" given " ZEND_LONG_FMT, UINT_MAX, Z_LVAL_P(val));
			ret = FAILURE;
		} else {
			*out = Z_LVAL_P(val);
			ret = SUCCESS;
		}
	} else {
		if (Z_REFCOUNTED_P(val)) {
			Z_ADDREF_P(val);
		}
		convert_to_string_ex(val);
		ret = pthreads_socket_string_to_if_index(Z_STRVAL_P(val), out);
		zval_ptr_dtor(val);
	}

	return ret;
}

static int pthreads_socket_get_if_index_from_array(const HashTable *ht, const char *key, unsigned int *if_index)
{
	zval *val;

	if ((val = zend_hash_str_find(ht, key, strlen(key))) == NULL) {
		*if_index = 0; /* default: 0 */
		return SUCCESS;
	}

	return pthreads_get_if_index_from_zval(val, if_index);
}

static int pthreads_socket_get_address_from_array(const HashTable *ht, const char *key, pthreads_socket_t *sock, php_sockaddr_storage *ss, socklen_t *ss_len)
{
	zval *val;

	if ((val = zend_hash_str_find(ht, key, strlen(key))) == NULL) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"no key \"%s\" passed in optval", key);
		return FAILURE;
	}
	if (Z_REFCOUNTED_P(val)) {
		Z_ADDREF_P(val);
	}
	convert_to_string_ex(val);
	if (!pthreads_socket_set_inet46_addr(sock, ss, ss_len, Z_STRVAL_P(val))) {
		zval_ptr_dtor(val);
		return FAILURE;
	}
	zval_ptr_dtor(val);
	return SUCCESS;
}

static int pthreads_do_mcast_opt(pthreads_socket_t *sock, int level, int optname, zval *arg4)
{
	HashTable		 		*opt_ht;
	unsigned int			if_index;
	int						retval;
	int (*mcast_req_fun)(pthreads_socket_t *, int, struct sockaddr *, socklen_t, unsigned);
#ifdef HAS_MCAST_EXT
	int (*mcast_sreq_fun)(pthreads_socket_t *, int, struct sockaddr *, socklen_t, struct sockaddr *, socklen_t, unsigned);
#endif

	switch (optname) {
	case PTHREADS_MCAST_JOIN_GROUP:
		mcast_req_fun = &pthreads_mcast_join;
		goto mcast_req_fun;
	case PTHREADS_MCAST_LEAVE_GROUP:
		{
			php_sockaddr_storage	group = {0};
			socklen_t				glen;

			mcast_req_fun = &pthreads_mcast_leave;
mcast_req_fun:
			convert_to_array_ex(arg4);
			opt_ht = Z_ARRVAL_P(arg4);

			if (pthreads_socket_get_address_from_array(opt_ht, "group", sock, &group, &glen) == FAILURE) {
					return FAILURE;
			}

			if (pthreads_socket_get_if_index_from_array(opt_ht, "interface", &if_index) == FAILURE) {
					return FAILURE;
			}

			retval = mcast_req_fun(sock, level, (struct sockaddr*)&group,
				glen, if_index);
			break;
		}

#ifdef HAS_MCAST_EXT
	case PTHREADS_MCAST_BLOCK_SOURCE:
		mcast_sreq_fun = &pthreads_mcast_block_source;
		goto mcast_sreq_fun;
	case PTHREADS_MCAST_UNBLOCK_SOURCE:
		mcast_sreq_fun = &pthreads_mcast_unblock_source;
		goto mcast_sreq_fun;
	case PTHREADS_MCAST_JOIN_SOURCE_GROUP:
		mcast_sreq_fun = &pthreads_mcast_join_source;
		goto mcast_sreq_fun;
	case PTHREADS_MCAST_LEAVE_SOURCE_GROUP:
		{
			php_sockaddr_storage	group = {0},
									source = {0};
			socklen_t				glen,
									slen;

			mcast_sreq_fun = &pthreads_mcast_leave_source;
		mcast_sreq_fun:
			convert_to_array_ex(arg4);
			opt_ht = Z_ARRVAL_P(arg4);

			if (pthreads_socket_get_address_from_array(opt_ht, "group", sock, &group, &glen) == FAILURE) {
				return FAILURE;
			}

			if (pthreads_socket_get_address_from_array(opt_ht, "source", sock, &source, &slen) == FAILURE) {
				return FAILURE;
			}

			if (pthreads_socket_get_if_index_from_array(opt_ht, "interface", &if_index) == FAILURE) {
				return FAILURE;
			}

			retval = mcast_sreq_fun(sock, level, (struct sockaddr*)&group,
					glen, (struct sockaddr*)&source, slen, if_index);
			break;
		}
#endif
	default:
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"unexpected option in pthreads_do_mcast_opt (level %d, option %d). "
				"This is a bug.", level, optname);
		return FAILURE;
	}

	if (retval != 0) {
		if (retval != -2) { /* error, but message already emitted */
			PTHREADS_SOCKET_ERROR(sock, "Unable to set socket option", errno);
		}
		return FAILURE;
	}
	return SUCCESS;
}

int pthreads_do_setsockopt_ip_mcast(pthreads_socket_t *sock,
									int level,
									int optname,
									zval *arg4)
{
	unsigned int	if_index;
	struct in_addr	if_addr;
	void 			*opt_ptr;
	socklen_t		optlen;
	unsigned char	ipv4_mcast_ttl_lback;
	int				retval;

	switch (optname) {
	case PTHREADS_MCAST_JOIN_GROUP:
	case PTHREADS_MCAST_LEAVE_GROUP:
#ifdef HAS_MCAST_EXT
	case PTHREADS_MCAST_BLOCK_SOURCE:
	case PTHREADS_MCAST_UNBLOCK_SOURCE:
	case PTHREADS_MCAST_JOIN_SOURCE_GROUP:
	case PTHREADS_MCAST_LEAVE_SOURCE_GROUP:
#endif
		if (pthreads_do_mcast_opt(sock, level, optname, arg4) == FAILURE) {
			return FAILURE;
		} else {
			return SUCCESS;
		}

	case IP_MULTICAST_IF:
		if (pthreads_get_if_index_from_zval(arg4, &if_index) == FAILURE) {
			return FAILURE;
		}

		if (pthreads_if_index_to_addr4(if_index, sock, &if_addr) == FAILURE) {
			return FAILURE;
		}
		opt_ptr = &if_addr;
		optlen	= sizeof(if_addr);
		goto dosockopt;

	case IP_MULTICAST_LOOP:
		convert_to_boolean_ex(arg4);
		ipv4_mcast_ttl_lback = (unsigned char) (Z_TYPE_P(arg4) == IS_TRUE);
		goto ipv4_loop_ttl;

	case IP_MULTICAST_TTL:
		convert_to_long_ex(arg4);
		if (Z_LVAL_P(arg4) < 0L || Z_LVAL_P(arg4) > 255L) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Expected a value between 0 and 255");
			return FAILURE;
		}
		ipv4_mcast_ttl_lback = (unsigned char) Z_LVAL_P(arg4);
ipv4_loop_ttl:
		opt_ptr = &ipv4_mcast_ttl_lback;
		optlen	= sizeof(ipv4_mcast_ttl_lback);
		goto dosockopt;
	}

	return 1;

dosockopt:
	retval = setsockopt(sock->fd, level, optname, opt_ptr, optlen);
	if (retval != 0) {
		PTHREADS_SOCKET_ERROR(sock, "Unable to set socket option", errno);
		return FAILURE;
	}

	return SUCCESS;
}

int pthreads_do_setsockopt_ipv6_mcast(	pthreads_socket_t *sock,
										int level,
										int optname,
										zval *arg4)
{
	unsigned int	if_index;
	void			*opt_ptr;
	socklen_t		optlen;
	int				ov;
	int				retval;

	switch (optname) {
	case PTHREADS_MCAST_JOIN_GROUP:
	case PTHREADS_MCAST_LEAVE_GROUP:
#ifdef HAS_MCAST_EXT
	case PTHREADS_MCAST_BLOCK_SOURCE:
	case PTHREADS_MCAST_UNBLOCK_SOURCE:
	case PTHREADS_MCAST_JOIN_SOURCE_GROUP:
	case PTHREADS_MCAST_LEAVE_SOURCE_GROUP:
#endif
		if (pthreads_do_mcast_opt(sock, level, optname, arg4) == FAILURE) {
			return FAILURE;
		} else {
			return SUCCESS;
		}

	case IPV6_MULTICAST_IF:
		if (pthreads_get_if_index_from_zval(arg4, &if_index) == FAILURE) {
			return FAILURE;
		}

		opt_ptr = &if_index;
		optlen	= sizeof(if_index);
		goto dosockopt;

	case IPV6_MULTICAST_LOOP:
		convert_to_boolean_ex(arg4);
		ov = (int) Z_TYPE_P(arg4) == IS_TRUE;
		goto ipv6_loop_hops;
	case IPV6_MULTICAST_HOPS:
		convert_to_long_ex(arg4);
		if (Z_LVAL_P(arg4) < -1L || Z_LVAL_P(arg4) > 255L) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Expected a value between -1 and 255");
			return FAILURE;
		}
		ov = (int) Z_LVAL_P(arg4);
ipv6_loop_hops:
		opt_ptr = &ov;
		optlen	= sizeof(ov);
		goto dosockopt;
	}

	return 1; /* not handled */

dosockopt:
	retval = setsockopt(sock->fd, level, optname, opt_ptr, optlen);
	if (retval != 0) {
		PTHREADS_SOCKET_ERROR(sock, "Unable to set socket option", errno);
		return FAILURE;
	}

	return SUCCESS;
}

int pthreads_mcast_join(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index)
{
	return _pthreads_mcast_join_leave(sock, level, group, group_len, if_index, 1);
}

int pthreads_mcast_leave(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index)
{
	return _pthreads_mcast_join_leave(sock, level, group, group_len, if_index, 0);
}

#ifdef HAS_MCAST_EXT
int pthreads_mcast_join_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index)
{
	return _pthreads_mcast_source_op(sock, level, group, group_len, source, source_len, if_index, JOIN_SOURCE);
}

int pthreads_mcast_leave_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index)
{
	return _pthreads_mcast_source_op(sock, level, group, group_len, source, source_len, if_index, LEAVE_SOURCE);
}

int pthreads_mcast_block_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index)
{
	return _pthreads_mcast_source_op(sock, level, group, group_len, source, source_len, if_index, BLOCK_SOURCE);
}

int pthreads_mcast_unblock_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index)
{
	return _pthreads_mcast_source_op(sock, level, group, group_len, source, source_len, if_index, UNBLOCK_SOURCE);
}
#endif /* HAS_MCAST_EXT */


static int _pthreads_mcast_join_leave(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group, /* struct sockaddr_in/sockaddr_in6 */
	socklen_t group_len,
	unsigned int if_index,
	int join)
{
#ifdef RFC3678_API
	struct group_req greq = {0};

	memcpy(&greq.gr_group, group, group_len);
	assert(greq.gr_group.ss_family != 0); /* the caller has set this */
	greq.gr_interface = if_index;

	return setsockopt(sock->fd, level,
			join ? MCAST_JOIN_GROUP : MCAST_LEAVE_GROUP, (char*)&greq,
			sizeof(greq));
#else
	if (sock->domain == AF_INET) {
		struct ip_mreq mreq = {0};
		struct in_addr addr;

		assert(group_len == sizeof(struct sockaddr_in));

		if (if_index != 0) {
			if (pthreads_if_index_to_addr4(if_index, sock, &addr) == FAILURE)
				return -2; /* failure, but notice already emitted */
			mreq.imr_interface = addr;
		} else {
			mreq.imr_interface.s_addr = htonl(INADDR_ANY);
		}
		mreq.imr_multiaddr = ((struct sockaddr_in*)group)->sin_addr;
		return setsockopt(sock->fd, level,
				join ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP, (char*)&mreq,
				sizeof(mreq));
	}
#if HAVE_IPV6
	else if (sock->domain == AF_INET6) {
		struct ipv6_mreq mreq = {0};

		assert(group_len == sizeof(struct sockaddr_in6));

		mreq.ipv6mr_multiaddr = ((struct sockaddr_in6*)group)->sin6_addr;
		mreq.ipv6mr_interface = if_index;

		return setsockopt(sock->fd, level,
				join ? IPV6_JOIN_GROUP : IPV6_LEAVE_GROUP, (char*)&mreq,
				sizeof(mreq));
	}
#endif
	else {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Option %s is inapplicable to this socket type",
				join ? "MCAST_JOIN_GROUP" : "MCAST_LEAVE_GROUP");
		return -2;
	}
#endif
}

#ifdef HAS_MCAST_EXT
static int _pthreads_mcast_source_op(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index,
	enum source_op sop)
{
#ifdef RFC3678_API
	struct group_source_req gsreq = {0};

	memcpy(&gsreq.gsr_group, group, group_len);
	assert(gsreq.gsr_group.ss_family != 0);
	memcpy(&gsreq.gsr_source, source, source_len);
	assert(gsreq.gsr_source.ss_family != 0);
	gsreq.gsr_interface = if_index;

	return setsockopt(sock->fd, level,
			_pthreads_source_op_to_rfc3678_op(sop), (char*)&gsreq, sizeof(gsreq));
#else
	if (sock->domain == AF_INET) {
		struct ip_mreq_source mreqs = {0};
		struct in_addr addr;

		mreqs.imr_multiaddr = ((struct sockaddr_in*)group)->sin_addr;
		mreqs.imr_sourceaddr =  ((struct sockaddr_in*)source)->sin_addr;

		assert(group_len == sizeof(struct sockaddr_in));
		assert(source_len == sizeof(struct sockaddr_in));

		if (if_index != 0) {
			if (pthreads_if_index_to_addr4(if_index, sock, &addr) == FAILURE)
				return -2; /* failure, but notice already emitted */
			mreqs.imr_interface = addr;
		} else {
			mreqs.imr_interface.s_addr = htonl(INADDR_ANY);
		}

		return setsockopt(sock->fd, level,
				_pthreads_source_op_to_ipv4_op(sop), (char*)&mreqs, sizeof(mreqs));
	}
#if HAVE_IPV6
	else if (sock->domain == AF_INET6) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"This platform does not support %s for IPv6 sockets",
				_pthreads_source_op_to_string(sop));
		return -2;
	}
#endif
	else {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Option %s is inapplicable to this socket type",
				_pthreads_source_op_to_string(sop));
		return -2;
	}
#endif
}

#if RFC3678_API
static int _pthreads_source_op_to_rfc3678_op(enum source_op sop)
{
	switch (sop) {
	case JOIN_SOURCE:
		return MCAST_JOIN_SOURCE_GROUP;
	case LEAVE_SOURCE:
		return MCAST_LEAVE_SOURCE_GROUP;
	case BLOCK_SOURCE:
		return MCAST_BLOCK_SOURCE;
	case UNBLOCK_SOURCE:
		return MCAST_UNBLOCK_SOURCE;
	}

	assert(0);
	return 0;
}
#else
static const char *_pthreads_source_op_to_string(enum source_op sop)
{
	switch (sop) {
	case JOIN_SOURCE:
		return "MCAST_JOIN_SOURCE_GROUP";
	case LEAVE_SOURCE:
		return "MCAST_LEAVE_SOURCE_GROUP";
	case BLOCK_SOURCE:
		return "MCAST_BLOCK_SOURCE";
	case UNBLOCK_SOURCE:
		return "MCAST_UNBLOCK_SOURCE";
	}

	assert(0);
	return "";
}

static int _pthreads_source_op_to_ipv4_op(enum source_op sop)
{
	switch (sop) {
	case JOIN_SOURCE:
		return IP_ADD_SOURCE_MEMBERSHIP;
	case LEAVE_SOURCE:
		return IP_DROP_SOURCE_MEMBERSHIP;
	case BLOCK_SOURCE:
		return IP_BLOCK_SOURCE;
	case UNBLOCK_SOURCE:
		return IP_UNBLOCK_SOURCE;
	}

	assert(0);
	return 0;
}
#endif

#endif /* HAS_MCAST_EXT */

#ifdef PHP_WIN32
int pthreads_if_index_to_addr4(unsigned if_index, pthreads_socket_t *sock, struct in_addr *out_addr)
{
	MIB_IPADDRTABLE *addr_table;
    ULONG size;
    DWORD retval;
	DWORD i;

	if (if_index == 0) {
		out_addr->s_addr = INADDR_ANY;
		return SUCCESS;
	}

	size = 4 * (sizeof *addr_table);
	addr_table = emalloc(size);
retry:
	retval = GetIpAddrTable(addr_table, &size, 0);
	if (retval == ERROR_INSUFFICIENT_BUFFER) {
		efree(addr_table);
		addr_table = emalloc(size);
		goto retry;
	}
	if (retval != NO_ERROR) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"GetIpAddrTable failed with error %lu", retval);

		return FAILURE;
	}
	for (i = 0; i < addr_table->dwNumEntries; i++) {
		MIB_IPADDRROW r = addr_table->table[i];
		if (r.dwIndex == if_index) {
			out_addr->s_addr = r.dwAddr;
			return SUCCESS;
		}
	}
	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
			"No interface with index %u was found", if_index);

	return FAILURE;
}

int pthreads_add4_to_if_index(struct in_addr *addr, pthreads_socket_t *sock, unsigned *if_index)
{
	MIB_IPADDRTABLE *addr_table;
    ULONG size;
    DWORD retval;
	DWORD i;

	if (addr->s_addr == INADDR_ANY) {
		*if_index = 0;
		return SUCCESS;
	}

	size = 4 * (sizeof *addr_table);
	addr_table = emalloc(size);
retry:
	retval = GetIpAddrTable(addr_table, &size, 0);
	if (retval == ERROR_INSUFFICIENT_BUFFER) {
		efree(addr_table);
		addr_table = emalloc(size);
		goto retry;
	}
	if (retval != NO_ERROR) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"GetIpAddrTable failed with error %lu", retval);
		return FAILURE;
	}
	for (i = 0; i < addr_table->dwNumEntries; i++) {
		MIB_IPADDRROW r = addr_table->table[i];
		if (r.dwAddr == addr->s_addr) {
			*if_index = r.dwIndex;
			return SUCCESS;
		}
	}

	{
		char addr_str[17] = {0};
		inet_ntop(AF_INET, addr, addr_str, sizeof(addr_str));

		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"The interface with IP address %s was not found", addr_str);
	}
	return FAILURE;
}

#else

int pthreads_if_index_to_addr4(unsigned if_index, pthreads_socket_t *sock, struct in_addr *out_addr)
{
	struct ifreq if_req;

	if (if_index == 0) {
		out_addr->s_addr = INADDR_ANY;
		return SUCCESS;
	}

#if !defined(ifr_ifindex) && defined(ifr_index)
#define ifr_ifindex ifr_index
#endif

#if defined(SIOCGIFNAME)
	if_req.ifr_ifindex = if_index;
	if (ioctl(sock->fd, SIOCGIFNAME, &if_req) == -1) {
#elif defined(HAVE_IF_INDEXTONAME)
	if (if_indextoname(if_index, if_req.ifr_name) == NULL) {
#else
#error Neither SIOCGIFNAME nor if_indextoname are available
#endif
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Failed obtaining address for interface %u: error %d", if_index, errno);
		return FAILURE;
	}

	if (ioctl(sock->fd, SIOCGIFADDR, &if_req) == -1) {
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"Failed obtaining address for interface %u: error %d", if_index, errno);
		return FAILURE;
	}

	memcpy(out_addr, &((struct sockaddr_in *) &if_req.ifr_addr)->sin_addr, sizeof *out_addr);

	return SUCCESS;
}

int pthreads_add4_to_if_index(struct in_addr *addr, pthreads_socket_t *sock, unsigned *if_index)
{
	struct ifconf	if_conf = {0};
	char			*buf = NULL,
					*p;
	int				size = 0,
					lastsize = 0;
	size_t			entry_len;

	if (addr->s_addr == INADDR_ANY) {
		*if_index = 0;
		return SUCCESS;
	}

	for(;;) {
		size += 5 * sizeof(struct ifreq);
		buf = ecalloc(size, 1);
		if_conf.ifc_len = size;
		if_conf.ifc_buf = buf;

		if (ioctl(sock->fd, SIOCGIFCONF, (char*)&if_conf) == -1 &&
				(errno != EINVAL || lastsize != 0)) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Failed obtaining interfaces list: error %d", errno);
			goto err;
		}

		if (if_conf.ifc_len == lastsize)
			/* not increasing anymore */
			break;
		else {
			lastsize = if_conf.ifc_len;
			efree(buf);
			buf = NULL;
		}
	}

	for (p = if_conf.ifc_buf;
		 p < if_conf.ifc_buf + if_conf.ifc_len;
		 p += entry_len) {
		struct ifreq *cur_req;

		/* let's hope the pointer is aligned */
		cur_req = (struct ifreq*) p;

#ifdef HAVE_SOCKADDR_SA_LEN
		entry_len = cur_req->ifr_addr.sa_len + sizeof(cur_req->ifr_name);
#else
		/* if there's no sa_len, assume the ifr_addr field is a sockaddr */
		entry_len = sizeof(struct sockaddr) + sizeof(cur_req->ifr_name);
#endif
		entry_len = MAX(entry_len, sizeof(*cur_req));

		if ((((struct sockaddr*)&cur_req->ifr_addr)->sa_family == AF_INET) &&
				(((struct sockaddr_in*)&cur_req->ifr_addr)->sin_addr.s_addr ==
					addr->s_addr)) {
#if defined(SIOCGIFINDEX)
			if (ioctl(sock->fd, SIOCGIFINDEX, (char*)cur_req)
					== -1) {
#elif defined(HAVE_IF_NAMETOINDEX)
			unsigned index_tmp;
			if ((index_tmp = if_nametoindex(cur_req->ifr_name)) == 0) {
#else
#error Neither SIOCGIFINDEX nor if_nametoindex are available
#endif
				zend_throw_exception_ex(spl_ce_RuntimeException, 0,
						"Error converting interface name to index: error %d", errno);
				goto err;
			} else {
#if defined(SIOCGIFINDEX)
				*if_index = cur_req->ifr_ifindex;
#else
				*if_index = index_tmp;
#endif
				efree(buf);
				return SUCCESS;
			}
		}
	}

	{
		char addr_str[17] = {0};
		inet_ntop(AF_INET, addr, addr_str, sizeof(addr_str));
		zend_throw_exception_ex(spl_ce_RuntimeException, 0,
				"The interface with IP address %s was not found", addr_str);
	}

err:
	if (buf != NULL)
		efree(buf);
	return FAILURE;
}

#if HAVE_IPV6
int pthreads_do_setsockopt_ipv6_rfc3542(pthreads_socket_t *sock, int level, int optname, zval *arg4)
{
	struct pthreads_err_s	err = {0};
	zend_llist				*allocations = NULL;
	void					*opt_ptr;
	socklen_t				optlen;
	int						retval;

	assert(level == IPPROTO_IPV6);

	switch (optname) {
#ifdef IPV6_PKTINFO
	case IPV6_PKTINFO:
#ifdef PHP_WIN32
		if (Z_TYPE_P(arg4) == IS_ARRAY) {
			zend_throw_exception_ex(spl_ce_RuntimeException, 0,
					"Windows does not support sticky IPV6_PKTINFO");
			return FAILURE;
		} else {
			/* windows has no IPV6_RECVPKTINFO, and uses IPV6_PKTINFO
			 * for the same effect. We define IPV6_RECVPKTINFO to be
			 * IPV6_PKTINFO, so assume the assume user used IPV6_RECVPKTINFO */
			return 1;
		}
#endif
		opt_ptr = pthreads_from_zval_run_conversions(arg4, sock, pthreads_from_zval_write_in6_pktinfo,
				sizeof(struct in6_pktinfo),	"in6_pktinfo", &allocations, &err);
		if (err.has_error) {
			err_msg_dispose(&err);
			return FAILURE;
		}

		optlen = sizeof(struct in6_pktinfo);
		goto dosockopt;
#endif
	}

	/* we also support IPV6_TCLASS, but that can be handled by the default
	 * integer optval handling in the caller */
	return 1;

dosockopt:
	retval = setsockopt(sock->fd, level, optname, opt_ptr, optlen);
	if (retval != 0) {
		PTHREADS_SOCKET_ERROR(sock, "Unable to set socket option", errno);
	}
	allocations_dispose(&allocations);

	return retval != 0 ? FAILURE : SUCCESS;
}

int pthreads_do_getsockopt_ipv6_rfc3542(pthreads_socket_t *sock, int level, int optname, zval *result)
{
	struct err_s		err = {0};
	void				*buffer;
	socklen_t			size;
	int					res;
	to_zval_read_field	*reader;

	assert(level == IPPROTO_IPV6);

	switch (optname) {
#ifdef IPV6_PKTINFO
	case IPV6_PKTINFO:
		size = sizeof(struct in6_pktinfo);
		reader = &to_zval_read_in6_pktinfo;
		break;
#endif
	default:
		return 1;
	}

	buffer = ecalloc(1, size);
	res = getsockopt(sock->fd, level, optname, buffer, &size);
	if (res != 0) {
		PTHREADS_SOCKET_ERROR(sock, "Unable to get socket option", errno);
	} else {
		zval tmp;
		zval *zv = pthreads_to_zval_run_conversions(buffer, reader, "in6_pktinfo",
				empty_key_value_list, &err, &tmp);
		if (err.has_error) {
			err_msg_dispose(&err);
			res = -1;
		} else {
			ZVAL_COPY_VALUE(result, zv);
		}
	}
	efree(buffer);

	return res == 0 ? SUCCESS : FAILURE;
}
#endif /* HAVE_IPV6 */

#endif

#endif
