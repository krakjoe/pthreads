#if defined(MCAST_JOIN_GROUP) && !defined(__APPLE__)
# define RFC3678_API 1
/* has block/unblock and source membership, in this case for both IPv4 and IPv6 */
# define HAS_MCAST_EXT 1
#elif defined(IP_ADD_SOURCE_MEMBERSHIP) && !defined(__APPLE__)
/* has block/unblock and source membership, but only for IPv4 */
# define HAS_MCAST_EXT 1
#endif

#ifndef RFC3678_API
# define PTHREADS_MCAST_JOIN_GROUP			IP_ADD_MEMBERSHIP
# define PTHREADS_MCAST_LEAVE_GROUP			IP_DROP_MEMBERSHIP
# ifdef HAS_MCAST_EXT
#  define PTHREADS_MCAST_BLOCK_SOURCE		IP_BLOCK_SOURCE
#  define PTHREADS_MCAST_UNBLOCK_SOURCE		IP_UNBLOCK_SOURCE
#  define PTHREADS_MCAST_JOIN_SOURCE_GROUP	IP_ADD_SOURCE_MEMBERSHIP
#  define PTHREADS_MCAST_LEAVE_SOURCE_GROUP	IP_DROP_SOURCE_MEMBERSHIP
# endif
#else
# define PTHREADS_MCAST_JOIN_GROUP			MCAST_JOIN_GROUP
# define PTHREADS_MCAST_LEAVE_GROUP			MCAST_LEAVE_GROUP
# define PTHREADS_MCAST_BLOCK_SOURCE		MCAST_BLOCK_SOURCE
# define PTHREADS_MCAST_UNBLOCK_SOURCE		MCAST_UNBLOCK_SOURCE
# define PTHREADS_MCAST_JOIN_SOURCE_GROUP	MCAST_JOIN_SOURCE_GROUP
# define PTHREADS_MCAST_LEAVE_SOURCE_GROUP	MCAST_LEAVE_SOURCE_GROUP
#endif

int pthreads_do_setsockopt_ip_mcast(pthreads_socket_t *sock,
									int level,
									int optname,
									zval *arg4);

int pthreads_do_setsockopt_ipv6_mcast(	pthreads_socket_t *sock,
										int level,
										int optname,
										zval *arg4);

int pthreads_do_setsockopt_ipv6_rfc3542(pthreads_socket_t *sock, int level, int optname, zval *arg4);
int pthreads_do_getsockopt_ipv6_rfc3542(pthreads_socket_t *sock, int level, int optname, zval *result);

int pthreads_if_index_to_addr4(
        unsigned if_index,
        pthreads_socket_t *sock,
        struct in_addr *out_addr);

int pthreads_add4_to_if_index(
        struct in_addr *addr,
        pthreads_socket_t *sock,
        unsigned *if_index);


int pthreads_mcast_join(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index);

int pthreads_mcast_leave(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	unsigned int if_index);

#ifdef HAS_MCAST_EXT
int pthreads_mcast_join_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int pthreads_mcast_leave_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int pthreads_mcast_block_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);

int pthreads_mcast_unblock_source(
	pthreads_socket_t *sock,
	int level,
	struct sockaddr *group,
	socklen_t group_len,
	struct sockaddr *source,
	socklen_t source_len,
	unsigned int if_index);
#endif
