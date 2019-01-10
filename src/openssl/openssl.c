#ifndef HAVE_PTHREADS_OPENSSL
#define HAVE_PTHREADS_OPENSSL

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_PTHREADS_OPENSSL_EXT

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_OPENSSL_H
#	include <src/openssl/openssl.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_TRANSPORTS_H
#	include <src/streams/transports.h>
#endif

/* OpenSSL includes */
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/dh.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/crypto.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/pkcs12.h>


static int pthreads_ssl_stream_data_index;

void pthreads_init_openssl() {
	char * config_filename;

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
	OPENSSL_config(NULL);
	SSL_library_init();
	OpenSSL_add_all_ciphers();
	OpenSSL_add_all_digests();
	OpenSSL_add_all_algorithms();

#if !defined(OPENSSL_NO_AES) && defined(EVP_CIPH_CCM_MODE) && OPENSSL_VERSION_NUMBER < 0x100020000
	EVP_add_cipher(EVP_aes_128_ccm());
	EVP_add_cipher(EVP_aes_192_ccm());
	EVP_add_cipher(EVP_aes_256_ccm());
#endif

	SSL_load_error_strings();
#else
	OPENSSL_init_ssl(OPENSSL_INIT_LOAD_CONFIG, NULL);
#endif

	/* register a resource id number with OpenSSL so that we can map SSL -> stream structures in
	 * OpenSSL callbacks */
	pthreads_ssl_stream_data_index = SSL_get_ex_new_index(0, "PHP stream index", NULL, NULL, NULL);

	//REGISTER_STRING_CONSTANT("OPENSSL_VERSION_TEXT", OPENSSL_VERSION_TEXT, CONST_CS|CONST_PERSISTENT);
	//REGISTER_LONG_CONSTANT("OPENSSL_VERSION_NUMBER", OPENSSL_VERSION_NUMBER, CONST_CS|CONST_PERSISTENT);

	/* Determine default SSL configuration file */
	config_filename = getenv("OPENSSL_CONF");
	if (config_filename == NULL) {
		config_filename = getenv("SSLEAY_CONF");
	}

	/* default to 'openssl.cnf' if no environment variable is set */
	if (config_filename == NULL) {
		snprintf(pthreads_default_ssl_conf_filename, sizeof(pthreads_default_ssl_conf_filename), "%s/%s",
				X509_get_default_cert_area(),
				"openssl.cnf");
	} else {
		strlcpy(pthreads_default_ssl_conf_filename, config_filename, sizeof(pthreads_default_ssl_conf_filename));
	}

	pthreads_stream_xport_register("ssl", pthreads_openssl_ssl_socket_factory);
#ifndef OPENSSL_NO_SSL3
	pthreads_stream_xport_register("sslv3", pthreads_openssl_ssl_socket_factory);
#endif
	pthreads_stream_xport_register("tls", pthreads_openssl_ssl_socket_factory);
	pthreads_stream_xport_register("tlsv1.0", pthreads_openssl_ssl_socket_factory);
	pthreads_stream_xport_register("tlsv1.1", pthreads_openssl_ssl_socket_factory);
	pthreads_stream_xport_register("tlsv1.2", pthreads_openssl_ssl_socket_factory);

	/* override the default tcp socket provider */
	pthreads_stream_xport_register("tcp", pthreads_openssl_ssl_socket_factory);

	pthreads_register_url_stream_wrapper("https", PTHREADS_STREAMG(stream_http_wrapper));
	pthreads_register_url_stream_wrapper("ftps", PTHREADS_STREAMG(stream_ftp_wrapper));

	//REGISTER_INI_ENTRIES();
}

void pthreads_shutdown_openssl() {
#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined (LIBRESSL_VERSION_NUMBER)
	EVP_cleanup();

	/* prevent accessing locking callback from unloaded extension */
	CRYPTO_set_locking_callback(NULL);
	/* free allocated error strings */
	ERR_free_strings();
	CONF_modules_free();
#endif

	pthreads_unregister_url_stream_wrapper("https");
	pthreads_unregister_url_stream_wrapper("ftps");

	pthreads_stream_xport_unregister("ssl");
#ifndef OPENSSL_NO_SSL3
	pthreads_stream_xport_unregister("sslv3");
#endif
	pthreads_stream_xport_unregister("tls");
	pthreads_stream_xport_unregister("tlsv1.0");
	pthreads_stream_xport_unregister("tlsv1.1");
	pthreads_stream_xport_unregister("tlsv1.2");

	/* reinstate the default tcp handler */
	pthreads_stream_xport_register("tcp", pthreads_stream_generic_socket_factory);

	//UNREGISTER_INI_ENTRIES();
}

pthreads_openssl_x509_t *pthreads_openssl_x509_new(X509 *cert) {
	pthreads_object_t *threaded_x509 = pthreads_object_init(pthreads_openssl_x509_entry);
	PTHREADS_FETCH_OPENSSL_X509(threaded_x509) = cert;

	return threaded_x509;
}

pthreads_openssl_pkey_t *pthreads_openssl_pkey_new(EVP_PKEY *pkey) {
	pthreads_object_t *threaded_pkey = pthreads_object_init(pthreads_openssl_pkey_entry);
	PTHREADS_FETCH_OPENSSL_PKEY(threaded_pkey) = pkey;

	return threaded_pkey;
}

pthreads_stream_t *pthreads_openssl_get_stream_from_ssl_handle(const SSL *ssl)
{
	return (pthreads_stream_t*)SSL_get_ex_data(ssl, pthreads_ssl_stream_data_index);
}

int pthreads_openssl_get_ssl_stream_data_index()
{
	return pthreads_ssl_stream_data_index;
}

#endif
#endif
