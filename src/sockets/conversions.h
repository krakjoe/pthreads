#ifndef HAVE_PTHREADS_SOCKET_CONVERSIONS_H
#define HAVE_PTHREADS_SOCKET_CONVERSIONS_H 1

#include <php.h>

#ifndef PHP_WIN32
# include <netinet/in.h>
# include <sys/socket.h>
#else
# include <Ws2tcpip.h>
#endif

#include "../socket.h"

/* TYPE DEFINITIONS */
struct pthreads_err_s {
	int		has_error;
	char	*msg;
	int		level;
	int		should_free;
};

struct pthreads_key_value {
	const char	*key;
	unsigned	key_size;
	void		*value;
};

/* the complete types of these two are not relevant to the outside */
typedef struct _pthreads_ser_context pthreads_ser_context;
typedef struct _pthreads_res_context pthreads_res_context;

#define PTHREADS_KEY_RECVMSG_RET 	"recvmsg_ret"
#define PTHREADS_KEY_FILL_SOCKADDR 	"fill_sockaddr"
#define PTHREADS_KEY_RECVMSG_RET   	"recvmsg_ret"
#define PTHREADS_KEY_CMSG_LEN	  	"cmsg_len"

typedef void (pthreads_from_zval_write_field)(const zval *arr_value, char *field, pthreads_ser_context *ctx);
typedef void (pthreads_to_zval_read_field)(const char *data, zval *zv, pthreads_res_context *ctx);

/* VARIABLE DECLARATIONS */
extern const struct pthreads_key_value empty_key_value_list[];

/* AUX FUNCTIONS */
void pthreads_err_msg_dispose(struct pthreads_err_s *err);
void pthreads_allocations_dispose(zend_llist **allocations);

/* CONVERSION FUNCTIONS */
void pthreads_from_zval_write_int(const zval *arr_value, char *field, pthreads_ser_context *ctx);
void pthreads_to_zval_read_int(const char *data, zval *zv, pthreads_res_context *ctx);

#ifdef IPV6_PKTINFO
void pthreads_from_zval_write_in6_pktinfo(const zval *container, char *in6_pktinfo_c, pthreads_ser_context *ctx);
void pthreads_to_zval_read_in6_pktinfo(const char *data, zval *zv, pthreads_res_context *ctx);
#endif

#ifdef SO_PASSCRED
void pthreads_from_zval_write_ucred(const zval *container, char *ucred_c, pthreads_ser_context *ctx);
void pthreads_to_zval_read_ucred(const char *data, zval *zv, pthreads_res_context *ctx);
#endif

void pthreads_from_zval_write_msghdr_send(const zval *container, char *msghdr_c, pthreads_ser_context *ctx);
void pthreads_from_zval_write_msghdr_recv(const zval *container, char *msghdr_c, pthreads_ser_context *ctx);
void pthreads_to_zval_read_msghdr(const char *msghdr_c, zval *zv, pthreads_res_context *ctx);

/* ENTRY POINTS FOR CONVERSIONS */
void *pthreads_from_zval_run_conversions(	const zval						*container,
											pthreads_socket_t				*sock,
											pthreads_from_zval_write_field	*writer,
											size_t							struct_size,
											const char						*top_name,
											zend_llist						**allocations /* out */,
											struct pthreads_err_s			*err /* in/out */);

zval *pthreads_to_zval_run_conversions(		const char						*structure,
											pthreads_to_zval_read_field		*reader,
											const char						*top_name,
											const struct pthreads_key_value	*key_value_pairs,
											struct pthreads_err_s			*err, zval *zv);

#endif
