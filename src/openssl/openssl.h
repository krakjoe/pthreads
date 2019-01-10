#ifndef HAVE_PTHREADS_OPENSSL_H
#define HAVE_PTHREADS_OPENSSL_H
#ifdef HAVE_PTHREADS_OPENSSL_EXT

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#	include <src/streams/transports.h>
#endif

#include "php.h"
#include <openssl/opensslv.h>
#if defined(LIBRESSL_VERSION_NUMBER)
/* LibreSSL version check */
#if LIBRESSL_VERSION_NUMBER < 0x20700000L
#define PTHREADS_OPENSSL_API_VERSION 0x10001
#else
#define PTHREADS_OPENSSL_API_VERSION 0x10100
#endif
#else
/* OpenSSL version check */
#if OPENSSL_VERSION_NUMBER < 0x10002000L
#define PTHREADS_OPENSSL_API_VERSION 0x10001
#elif OPENSSL_VERSION_NUMBER < 0x10100000L
#define PTHREADS_OPENSSL_API_VERSION 0x10002
#else
#define PTHREADS_OPENSSL_API_VERSION 0x10100
#endif
#endif

#define PTHREADS_OPENSSL_RAW_DATA 1
#define PTHREADS_OPENSSL_ZERO_PADDING 2
#define PTHREADS_OPENSSL_DONT_ZERO_PAD_KEY 4

#define PTHREADS_OPENSSL_ERROR_X509_PRIVATE_KEY_VALUES_MISMATCH 0x0B080074

/* Used for client-initiated handshake renegotiation DoS protection*/
#define PTHREADS_OPENSSL_DEFAULT_RENEG_LIMIT 2
#define PTHREADS_OPENSSL_DEFAULT_RENEG_WINDOW 300
#define PTHREADS_OPENSSL_DEFAULT_STREAM_VERIFY_DEPTH 9
#define PTHREADS_OPENSSL_DEFAULT_STREAM_CIPHERS "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:" \
	"ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:" \
	"DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:" \
	"ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:" \
	"ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:" \
	"DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:" \
	"AES256-GCM-SHA384:AES128:AES256:HIGH:!SSLv2:!aNULL:!eNULL:!EXPORT:!DES:!MD5:!RC4:!ADH"

#include <openssl/err.h>


pthreads_stream_transport_factory_func pthreads_openssl_ssl_socket_factory;

/* openssl -> PHP/PTHREADS "bridging" */
/* true global; readonly after module startup */
static char pthreads_default_ssl_conf_filename[MAXPATHLEN];

struct pthreads_openssl_errors {
	int buffer[ERR_NUM_ERRORS];
	int top;
	int bottom;
};

const pthreads_stream_ops pthreads_openssl_socket_ops;

typedef struct _pthreads_object_t pthreads_openssl_x509_t;
typedef struct _pthreads_object_t pthreads_openssl_pkey_t;

#define PTHREADS_FETCH_OPENSSL_X509(object) ((object)->store.openssl->x509)
#define PTHREADS_FETCH_OPENSSL_PKEY(object) ((object)->store.openssl->pkey)

typedef union _pthreads_openssl_t {
	X509 		*x509;
	EVP_PKEY	*pkey;
} pthreads_openssl_t;

void pthreads_init_openssl();
void pthreads_shutdown_openssl();

pthreads_openssl_x509_t *pthreads_openssl_x509_new(X509 *cert);
pthreads_openssl_pkey_t *pthreads_openssl_pkey_new(EVP_PKEY *pkey);

pthreads_stream_t *pthreads_openssl_get_stream_from_ssl_handle(const SSL *ssl);
int pthreads_openssl_get_ssl_stream_data_index();

#ifdef PHP_WIN32
#define PTHREADS_OPENSSL_BIO_MODE_R(flags) (((flags) & PKCS7_BINARY) ? "rb" : "r")
#define PTHREADS_OPENSSL_BIO_MODE_W(flags) (((flags) & PKCS7_BINARY) ? "wb" : "w")
#else
#define PTHREADS_OPENSSL_BIO_MODE_R(flags) "r"
#define PTHREADS_OPENSSL_BIO_MODE_W(flags) "w"
#endif

#endif
#endif
