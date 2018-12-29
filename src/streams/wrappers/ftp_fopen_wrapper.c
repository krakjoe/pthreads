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
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FTP_FOPEN_WRAPPER
#define HAVE_PTHREADS_STREAMS_WRAPPERS_FTP_FOPEN_WRAPPER

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_FOPEN_WRAPPER_H
#	include <src/streams/wrappers/fopen_wrapper.h>
#endif

#ifndef HAVE_PTHREADS_NETWORK_H
#	include <src/network.h>
#endif

#ifndef PHP_STRING_H
#	include <ext/standard/php_string.h>
#endif


#define PTHREADS_FTPS_ENCRYPT_DATA 1
#define PTHREADS_GET_FTP_RESULT(threaded_stream)	pthreads_get_ftp_result((threaded_stream), tmp_line, sizeof(tmp_line))

typedef struct _pthreads_ftp_dirstream_data {
	pthreads_stream_t *datastream;
	pthreads_stream_t *controlstream;
	pthreads_stream_t *dirstream;
} pthreads_ftp_dirstream_data;

/* {{{ pthreads_get_ftp_result */
static inline int pthreads_get_ftp_result(pthreads_stream_t *threaded_stream, char *buffer, size_t buffer_size) {
	buffer[0] = '\0'; /* in case read fails to read anything */
	while (pthreads_stream_gets(threaded_stream, buffer, buffer_size-1) &&
		   !(isdigit((int) buffer[0]) && isdigit((int) buffer[1]) &&
			 isdigit((int) buffer[2]) && buffer[3] == ' '));
	return strtol(buffer, NULL, 10);
}
/* }}} */

/* {{{ pthreads_stream_ftp_stream_stat */
static int pthreads_stream_ftp_stream_stat(pthreads_stream_wrapper_t *threaded_wrapper, pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb) {
	/* For now, we return with a failure code to prevent the underlying
	 * file's details from being used instead. */
	return -1;
}
/* }}} */

/* {{{ pthreads_stream_ftp_stream_close */
static int pthreads_stream_ftp_stream_close(pthreads_stream_wrapper_t *threaded_wrapper, pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *controlstream = stream->wrapperthis;
	int ret = 0;

	if (controlstream && stream_lock(threaded_stream)) {
		if (strpbrk(stream->mode, "wa+")) {
			char tmp_line[512];
			int result;

			/* For write modes close data stream first to signal EOF to server */
			result = PTHREADS_GET_FTP_RESULT(controlstream);
			if (result != 226 && result != 250) {
				php_error_docref(NULL, E_WARNING, "FTP server error %d:%s", result, tmp_line);
				ret = EOF;
			}
		}

		pthreads_stream_write_string(controlstream, "QUIT\r\n");
		pthreads_stream_close(controlstream, PTHREADS_STREAM_FREE_CLOSE);
		stream->wrapperthis = NULL;

		stream_unlock(threaded_stream);
	}

	return ret;
}
/* }}} */

/* {{{ pthreads_ftp_fopen_connect */
static pthreads_stream_t *pthreads_ftp_fopen_connect(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
										 zend_string **opened_path, pthreads_stream_context_t *threaded_context, pthreads_stream_t **preuseid,
										 pthreads_url **presource, int *puse_ssl, int *puse_ssl_on_data)
{
	pthreads_stream_t *threaded_stream = NULL, *reuseid = NULL;
	pthreads_url *resource = NULL;
	int result, use_ssl, use_ssl_on_data = 0;
	char tmp_line[512];
	char *transport;
	int transport_len;

	resource = pthreads_url_parse(path);
	if (resource == NULL || resource->path == NULL) {
		if (resource && presource) {
			*presource = resource;
		}
		return NULL;
	}

	use_ssl = resource->scheme && (ZSTR_LEN(resource->scheme) > 3) && ZSTR_VAL(resource->scheme)[3] == 's';

	/* use port 21 if one wasn't specified */
	if (resource->port == 0)
		resource->port = 21;

	transport_len = (int)spprintf(&transport, 0, "tcp://%s:%d", ZSTR_VAL(resource->host), resource->port);
	threaded_stream = pthreads_stream_xport_create(transport, transport_len, PTHREADS_REPORT_ERRORS,
							PTHREADS_STREAM_XPORT_CLIENT | PTHREADS_STREAM_XPORT_CONNECT, NULL, threaded_context, NULL, NULL);
	efree(transport);
	if (threaded_stream == NULL) {
		result = 0; /* silence */
		goto connect_errexit;
	}

	pthreads_stream_context_set(threaded_stream, threaded_context);
	pthreads_stream_notify_info(threaded_context, PTHREADS_STREAM_NOTIFY_CONNECT, NULL, 0);

	/* Start talking to ftp server */
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result > 299 || result < 200) {
		pthreads_stream_notify_error(threaded_context, PTHREADS_STREAM_NOTIFY_FAILURE, tmp_line, result);
		goto connect_errexit;
	}

	if (use_ssl) {
		/* send the AUTH TLS request name */
		pthreads_stream_write_string(threaded_stream, "AUTH TLS\r\n");

		/* get the response */
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);
		if (result != 234) {
			/* AUTH TLS not supported try AUTH SSL */
			pthreads_stream_write_string(threaded_stream, "AUTH SSL\r\n");

			/* get the response */
			result = PTHREADS_GET_FTP_RESULT(threaded_stream);
			if (result != 334) {
				pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Server doesn't support FTPS.");
				goto connect_errexit;
			} else {
				/* we must reuse the old SSL session id */
				/* if we talk to an old ftpd-ssl */
				reuseid = threaded_stream;
			}
		} else {
			/* encrypt data etc */
		}
	}

	if (use_ssl) {
		if (pthreads_stream_xport_crypto_setup(threaded_stream, PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0
				|| pthreads_stream_xport_crypto_enable(threaded_stream, 1) < 0) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Unable to activate SSL mode");
			pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
			threaded_stream = NULL;
			goto connect_errexit;
		}

		/* set PBSZ to 0 */
		pthreads_stream_write_string(threaded_stream, "PBSZ 0\r\n");

		/* ignore the response */
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);

		/* set data connection protection level */
#if PTHREADS_FTPS_ENCRYPT_DATA
		pthreads_stream_write_string(threaded_stream, "PROT P\r\n");

		/* get the response */
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);
		use_ssl_on_data = (result >= 200 && result<=299) || reuseid;
#else
		pthreads_stream_write_string(threaded_stream, "PROT C\r\n");

		/* get the response */
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);
#endif
	}

#define PHP_FTP_CNTRL_CHK(val, val_len, err_msg) {	\
	unsigned char *s = (unsigned char *) val, *e = (unsigned char *) s + val_len;	\
	while (s < e) {	\
		if (iscntrl(*s)) {	\
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, err_msg, val);	\
			goto connect_errexit;	\
		}	\
		s++;	\
	}	\
}

	/* send the user name */
	if (resource->user != NULL) {
		ZSTR_LEN(resource->user) = php_raw_url_decode(ZSTR_VAL(resource->user), ZSTR_LEN(resource->user));

		PHP_FTP_CNTRL_CHK(ZSTR_VAL(resource->user), ZSTR_LEN(resource->user), "Invalid login %s")

		pthreads_stream_printf(threaded_stream, "USER %s\r\n", ZSTR_VAL(resource->user));
	} else {
		pthreads_stream_write_string(threaded_stream, "USER anonymous\r\n");
	}

	/* get the response */
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);

	/* if a password is required, send it */
	if (result >= 300 && result <= 399) {
		pthreads_stream_notify_info(threaded_context, PTHREADS_STREAM_NOTIFY_AUTH_REQUIRED, tmp_line, 0);

		if (resource->pass != NULL) {
			ZSTR_LEN(resource->pass) = php_raw_url_decode(ZSTR_VAL(resource->pass), ZSTR_LEN(resource->pass));

			PHP_FTP_CNTRL_CHK(ZSTR_VAL(resource->pass), ZSTR_LEN(resource->pass), "Invalid password %s")

			pthreads_stream_printf(threaded_stream, "PASS %s\r\n", ZSTR_VAL(resource->pass));
		} else {
			/* if the user has configured who they are,
			   send that as the password */
			if (FG(from_address)) {
				pthreads_stream_printf(threaded_stream, "PASS %s\r\n", FG(from_address));
			} else {
				pthreads_stream_write_string(threaded_stream, "PASS anonymous\r\n");
			}
		}

		/* read the response */
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);

		if (result > 299 || result < 200) {
			pthreads_stream_notify_error(threaded_context, PTHREADS_STREAM_NOTIFY_AUTH_RESULT, tmp_line, result);
		} else {
			pthreads_stream_notify_info(threaded_context, PTHREADS_STREAM_NOTIFY_AUTH_RESULT, tmp_line, result);
		}
	}
	if (result > 299 || result < 200) {
		goto connect_errexit;
	}

	if (puse_ssl) {
		*puse_ssl = use_ssl;
	}
	if (puse_ssl_on_data) {
		*puse_ssl_on_data = use_ssl_on_data;
	}
	if (preuseid) {
		*preuseid = reuseid;
	}
	if (presource) {
		*presource = resource;
	}

	return threaded_stream;

connect_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}

	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}

	return NULL;
}
/* }}} */

/* {{{ pthreads_fopen_do_pasv */
static unsigned short pthreads_fopen_do_pasv(pthreads_stream_t *threaded_stream, char *ip, size_t ip_size, char **phoststart) {
	char tmp_line[512];
	int result, i;
	unsigned short portno;
	char *tpath, *ttpath, *hoststart=NULL;

#ifdef HAVE_IPV6
	/* We try EPSV first, needed for IPv6 and works on some IPv4 servers */
	pthreads_stream_write_string(threaded_stream, "EPSV\r\n");
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);

	/* check if we got a 229 response */
	if (result != 229) {
#endif
		/* EPSV failed, let's try PASV */
		pthreads_stream_write_string(threaded_stream, "PASV\r\n");
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);

		/* make sure we got a 227 response */
		if (result != 227) {
			return 0;
		}

		/* parse pasv command (129, 80, 95, 25, 13, 221) */
		tpath = tmp_line;
		/* skip over the "227 Some message " part */
		for (tpath += 4; *tpath && !isdigit((int) *tpath); tpath++);
		if (!*tpath) {
			return 0;
		}
		/* skip over the host ip, to get the port */
		hoststart = tpath;
		for (i = 0; i < 4; i++) {
			for (; isdigit((int) *tpath); tpath++);
			if (*tpath != ',') {
				return 0;
			}
			*tpath='.';
			tpath++;
		}
		tpath[-1] = '\0';
		memcpy(ip, hoststart, ip_size);
		ip[ip_size-1] = '\0';
		hoststart = ip;

		/* pull out the MSB of the port */
		portno = (unsigned short) strtoul(tpath, &ttpath, 10) * 256;
		if (ttpath == NULL) {
			/* didn't get correct response from PASV */
			return 0;
		}
		tpath = ttpath;
		if (*tpath != ',') {
			return 0;
		}
		tpath++;
		/* pull out the LSB of the port */
		portno += (unsigned short) strtoul(tpath, &ttpath, 10);
#ifdef HAVE_IPV6
	} else {
		/* parse epsv command (|||6446|) */
		for (i = 0, tpath = tmp_line + 4; *tpath; tpath++) {
			if (*tpath == '|') {
				i++;
				if (i == 3)
					break;
			}
		}
		if (i < 3) {
			return 0;
		}
		/* pull out the port */
		portno = (unsigned short) strtoul(tpath + 1, &ttpath, 10);
	}
#endif
	if (ttpath == NULL) {
		/* didn't get correct response from EPSV/PASV */
		return 0;
	}

	if (phoststart) {
		*phoststart = hoststart;
	}

	return portno;
}
/* }}} */

/* {{{ pthreads_fopen_url_wrap_ftp */
pthreads_stream_t * pthreads_stream_url_wrap_ftp(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode,
									 int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream = NULL, *datastream = NULL;
	pthreads_url *resource = NULL;
	char tmp_line[512];
	char ip[sizeof("123.123.123.123")];
	unsigned short portno;
	char *hoststart = NULL;
	int result = 0, use_ssl, use_ssl_on_data=0;
	pthreads_stream_t *reuseid=NULL;
	size_t file_size = 0;
	zval *tmpzval;
	zend_bool allow_overwrite = 0;
	int8_t read_write = 0;
	char *transport;
	int transport_len;
	zend_string *error_message = NULL;

	tmp_line[0] = '\0';

	if (strpbrk(mode, "r+")) {
		read_write = 1; /* Open for reading */
	}
	if (strpbrk(mode, "wa+")) {
		if (read_write) {
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "FTP does not support simultaneous read/write connections");
			return NULL;
		}
		if (strchr(mode, 'a')) {
			read_write = 3; /* Open for Appending */
		} else {
			read_write = 2; /* Open for writing */
		}
	}
	if (!read_write) {
		/* No mode specified? */
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Unknown file open mode");
		return NULL;
	}

	if (threaded_context &&
		(tmpzval = pthreads_stream_context_get_option(threaded_context, "ftp", "proxy")) != NULL) {
		if (read_write == 1) {
			/* Use http wrapper to proxy ftp request */
			return pthreads_stream_url_wrap_http(threaded_wrapper, path, mode, options, opened_path, threaded_context, ce);
		} else {
			/* ftp proxy is read-only */
			pthreads_stream_wrapper_log_error(threaded_wrapper, options, "FTP proxy may only be used in read mode");
			return NULL;
		}
	}

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, path, mode, options, opened_path, threaded_context, &reuseid, &resource, &use_ssl, &use_ssl_on_data);
	if (!threaded_stream) {
		goto errexit;
	}

	/* set the connection to be binary */
	pthreads_stream_write_string(threaded_stream, "TYPE I\r\n");
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result > 299 || result < 200)
		goto errexit;

	/* find out the size of the file (verifying it exists) */
	pthreads_stream_printf(threaded_stream, "SIZE %s\r\n", ZSTR_VAL(resource->path));

	/* read the response */
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (read_write == 1) {
		/* Read Mode */
		char *sizestr;

		/* when reading file, it must exist */
		if (result > 299 || result < 200) {
			errno = ENOENT;
			goto errexit;
		}

		sizestr = strchr(tmp_line, ' ');
		if (sizestr) {
			sizestr++;
			file_size = atoi(sizestr);
			pthreads_stream_notify_file_size(threaded_context, file_size, tmp_line, result);
		}
	} else if (read_write == 2) {
		/* when writing file (but not appending), it must NOT exist, unless a context option exists which allows it */
		if (threaded_context && (tmpzval = pthreads_stream_context_get_option(threaded_context, "ftp", "overwrite")) != NULL) {
			allow_overwrite = Z_LVAL_P(tmpzval) ? 1 : 0;
		}
		if (result <= 299 && result >= 200) {
			if (allow_overwrite) {
				/* Context permits overwriting file,
				   so we just delete whatever's there in preparation */
				pthreads_stream_printf(threaded_stream, "DELE %s\r\n", ZSTR_VAL(resource->path));
				result = PTHREADS_GET_FTP_RESULT(threaded_stream);
				if (result >= 300 || result <= 199) {
					goto errexit;
				}
			} else {
				pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Remote file already exists and overwrite context option not specified");
				errno = EEXIST;
				goto errexit;
			}
		}
	}

	/* set up the passive connection */
	portno = pthreads_fopen_do_pasv(threaded_stream, ip, sizeof(ip), &hoststart);

	if (!portno) {
		goto errexit;
	}

	/* Send RETR/STOR command */
	if (read_write == 1) {
		/* set resume position if applicable */
		if (threaded_context &&
			(tmpzval = pthreads_stream_context_get_option(threaded_context, "ftp", "resume_pos")) != NULL &&
			Z_TYPE_P(tmpzval) == IS_LONG &&
			Z_LVAL_P(tmpzval) > 0) {
			pthreads_stream_printf(threaded_stream, "REST " ZEND_LONG_FMT "\r\n", Z_LVAL_P(tmpzval));
			result = PTHREADS_GET_FTP_RESULT(threaded_stream);
			if (result < 300 || result > 399) {
				pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Unable to resume from offset " ZEND_LONG_FMT, Z_LVAL_P(tmpzval));
				goto errexit;
			}
		}

		/* retrieve file */
		memcpy(tmp_line, "RETR", sizeof("RETR"));
	} else if (read_write == 2) {
		/* Write new file */
		memcpy(tmp_line, "STOR", sizeof("STOR"));
	} else {
		/* Append */
		memcpy(tmp_line, "APPE", sizeof("APPE"));
	}
	pthreads_stream_printf(threaded_stream, "%s %s\r\n", tmp_line, (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));

	/* open the data channel */
	if (hoststart == NULL) {
		hoststart = ZSTR_VAL(resource->host);
	}
	transport_len = (int)spprintf(&transport, 0, "tcp://%s:%d", hoststart, portno);
	datastream = pthreads_stream_xport_create(transport, transport_len, PTHREADS_REPORT_ERRORS,
			PTHREADS_STREAM_XPORT_CLIENT | PTHREADS_STREAM_XPORT_CONNECT, NULL, threaded_context, &error_message, NULL);
	efree(transport);
	if (datastream == NULL) {
		tmp_line[0]='\0';
		goto errexit;
	}

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result != 150 && result != 125) {
		/* Could not retrieve or send the file
		 * this data will only be sent to us after connection on the data port was initiated.
		 */
		pthreads_stream_close(datastream, PTHREADS_STREAM_FREE_CLOSE);
		datastream = NULL;
		goto errexit;
	}

	pthreads_stream_context_set(datastream, threaded_context);
	pthreads_stream_notify_progress_init(threaded_context, 0, file_size);

	if (use_ssl_on_data && (pthreads_stream_xport_crypto_setup(datastream,
			PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0 ||
			pthreads_stream_xport_crypto_enable(datastream, 1) < 0)) {

		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Unable to activate SSL mode");
		pthreads_stream_close(datastream, PTHREADS_STREAM_FREE_CLOSE);
		datastream = NULL;
		tmp_line[0]='\0';
		goto errexit;
	}

	/* remember control stream */
	PTHREADS_FETCH_STREAMS_STREAM(datastream)->wrapperthis = threaded_stream;

	pthreads_url_free(resource);
	return datastream;

errexit:
	if (resource) {
		pthreads_url_free(resource);
	}
	if (threaded_stream) {
		pthreads_stream_notify_error(threaded_context, PTHREADS_STREAM_NOTIFY_FAILURE, tmp_line, result);
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	if (tmp_line[0] != '\0')
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "FTP server reports %s", tmp_line);

	if (error_message) {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Failed to set up data channel: %s", ZSTR_VAL(error_message));
		zend_string_release(error_message);
	}
	return NULL;
}
/* }}} */

/* {{{ pthreads_ftp_dirsteam_read */
static size_t pthreads_ftp_dirstream_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream_dirent *ent = (pthreads_stream_dirent *)buf;
	pthreads_stream_t *innerstream;
	size_t tmp_len;
	zend_string *basename;

	if(stream_lock(threaded_stream)) {
		innerstream = ((pthreads_ftp_dirstream_data *)PTHREADS_FETCH_STREAMS_STREAM(threaded_stream)->abstract)->datastream;

		if (count != sizeof(pthreads_stream_dirent)) {
			stream_unlock(threaded_stream);
			return 0;
		}

		if (pthreads_stream_eof(innerstream)) {
			stream_unlock(threaded_stream);
			return 0;
		}

		if (!pthreads_stream_get_line(innerstream, ent->d_name, sizeof(ent->d_name), &tmp_len)) {
			stream_unlock(threaded_stream);
			return 0;
		}

		basename = php_basename(ent->d_name, tmp_len, NULL, 0);

		tmp_len = MIN(sizeof(ent->d_name), ZSTR_LEN(basename) - 1);
		memcpy(ent->d_name, ZSTR_VAL(basename), tmp_len);
		ent->d_name[tmp_len - 1] = '\0';
		zend_string_release(basename);

		/* Trim off trailing whitespace characters */
		while (tmp_len > 0 &&
				(ent->d_name[tmp_len - 1] == '\n' || ent->d_name[tmp_len - 1] == '\r' ||
				 ent->d_name[tmp_len - 1] == '\t' || ent->d_name[tmp_len - 1] == ' ')) {
			ent->d_name[--tmp_len] = '\0';
		}
		stream_unlock(threaded_stream);
	}
	return sizeof(pthreads_stream_dirent);
}
/* }}} */

/* {{{ pthreads_ftp_dirstream_close */
static int pthreads_ftp_dirstream_close(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_ftp_dirstream_data *data = stream->abstract;

	/* close control connection */
	if (data->controlstream) {
		pthreads_stream_close(data->controlstream, PTHREADS_STREAM_FREE_CLOSE);
		data->controlstream = NULL;
	}
	/* close data connection */
	pthreads_stream_close(data->datastream, PTHREADS_STREAM_FREE_CLOSE);
	data->datastream = NULL;

	return 0;
}
/* }}} */

/* {{{ pthreads_ftp_dirstream_close */
static void pthreads_ftp_dirstream_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_ftp_dirstream_data *data = stream->abstract;

	efree(data);
	stream->abstract = NULL;
}
/* }}} */
/* ftp dirstreams only need to support read and close operations,
   They can't be rewound because the underlying ftp stream can't be rewound. */
static const pthreads_stream_ops pthreads_ftp_dirstream_ops = {
	NULL, /* write */
	pthreads_ftp_dirstream_read, /* read */
	pthreads_ftp_dirstream_close, /* close */
	pthreads_ftp_dirstream_free, /* free */
	NULL, /* flush */
	"ftpdir",
	NULL, /* rewind */
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set option */
};

/* {{{ pthreads_stream_ftp_opendir */
pthreads_stream_t * pthreads_stream_ftp_opendir(pthreads_stream_wrapper_t *threaded_wrapper, const char *path, const char *mode, int options,
									zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_t *threaded_stream, *reuseid, *datastream = NULL;
	pthreads_ftp_dirstream_data *dirsdata;
	pthreads_url *resource = NULL;
	int result = 0, use_ssl, use_ssl_on_data = 0;
	char *hoststart = NULL, tmp_line[512];
	char ip[sizeof("123.123.123.123")];
	unsigned short portno;

	tmp_line[0] = '\0';

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, path, mode, options, opened_path, threaded_context, &reuseid, &resource, &use_ssl, &use_ssl_on_data);
	if (!threaded_stream) {
		goto opendir_errexit;
	}

	/* set the connection to be ascii */
	pthreads_stream_write_string(threaded_stream, "TYPE A\r\n");
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result > 299 || result < 200)
		goto opendir_errexit;

	// tmp_line isn't relevant after the php_fopen_do_pasv().
	tmp_line[0] = '\0';

	/* set up the passive connection */
	portno = pthreads_fopen_do_pasv(threaded_stream, ip, sizeof(ip), &hoststart);

	if (!portno) {
		goto opendir_errexit;
	}

	/* open the data channel */
	if (hoststart == NULL) {
		hoststart = ZSTR_VAL(resource->host);
	}

	datastream = pthreads_stream_sock_open_host(hoststart, portno, SOCK_STREAM, 0);
	if (datastream == NULL) {
		goto opendir_errexit;
	}

	pthreads_stream_printf(threaded_stream, "NLST %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result != 150 && result != 125) {
		/* Could not retrieve or send the file
		 * this data will only be sent to us after connection on the data port was initiated.
		 */
		pthreads_stream_close(datastream, PTHREADS_STREAM_FREE_CLOSE);
		datastream = NULL;
		goto opendir_errexit;
	}

	pthreads_stream_context_set(datastream, threaded_context);
	if (use_ssl_on_data && (pthreads_stream_xport_crypto_setup(datastream,
			PTHREADS_STREAM_CRYPTO_METHOD_SSLv23_CLIENT, NULL) < 0 ||
			pthreads_stream_xport_crypto_enable(datastream, 1) < 0)) {

		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "Unable to activate SSL mode");
		pthreads_stream_close(datastream, PTHREADS_STREAM_FREE_CLOSE);
		datastream = NULL;
		goto opendir_errexit;
	}

	pthreads_url_free(resource);

	dirsdata = emalloc(sizeof *dirsdata);
	dirsdata->datastream = datastream;
	dirsdata->controlstream = threaded_stream;
	dirsdata->dirstream = PTHREADS_STREAM_CLASS_NEW(&pthreads_ftp_dirstream_ops, dirsdata, mode, ce);

	return dirsdata->dirstream;

opendir_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}
	if (threaded_stream) {
		pthreads_stream_notify_error(threaded_context, PTHREADS_STREAM_NOTIFY_FAILURE, tmp_line, result);
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	if (tmp_line[0] != '\0') {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "FTP server reports %s", tmp_line);
	}
	return NULL;
}
/* }}} */

/* {{{ pthreads_stream_ftp_url_stat */
static int pthreads_stream_ftp_url_stat(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int flags, pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_url *resource = NULL;
	int result;
	char tmp_line[512];

	/* If ssb is NULL then someone is misbehaving */
	if (!ssb) return -1;

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, url, "r", 0, NULL, threaded_context, NULL, &resource, NULL, NULL);
	if (!threaded_stream) {
		goto stat_errexit;
	}

	ssb->sb.st_mode = 0644;									/* FTP won't give us a valid mode, so approximate one based on being readable */
	pthreads_stream_printf(threaded_stream, "CWD %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/")); /* If we can CWD to it, it's a directory (maybe a link, but we can't tell) */
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result < 200 || result > 299) {
		ssb->sb.st_mode |= S_IFREG;
	} else {
		ssb->sb.st_mode |= S_IFDIR;
	}

	pthreads_stream_write_string(threaded_stream, "TYPE I\r\n"); /* we need this since some servers refuse to accept SIZE command in ASCII mode */

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);

	if(result < 200 || result > 299) {
		goto stat_errexit;
	}

	pthreads_stream_printf(threaded_stream, "SIZE %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result < 200 || result > 299) {
		/* Failure either means it doesn't exist
		   or it's a directory and this server
		   fails on listing directory sizes */
		if (ssb->sb.st_mode & S_IFDIR) {
			ssb->sb.st_size = 0;
		} else {
			goto stat_errexit;
		}
	} else {
		ssb->sb.st_size = atoi(tmp_line + 4);
	}

	pthreads_stream_printf(threaded_stream, "MDTM %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result == 213) {
		char *p = tmp_line + 4;
		int n;
		struct tm tm, tmbuf, *gmt;
		time_t stamp;

		while ((size_t)(p - tmp_line) < sizeof(tmp_line) && !isdigit(*p)) {
			p++;
		}

		if ((size_t)(p - tmp_line) > sizeof(tmp_line)) {
			goto mdtm_error;
		}

		n = sscanf(p, "%4u%2u%2u%2u%2u%2u", &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec);
		if (n != 6) {
			goto mdtm_error;
		}

		tm.tm_year -= 1900;
		tm.tm_mon--;
		tm.tm_isdst = -1;

		/* figure out the GMT offset */
		stamp = time(NULL);
		gmt = php_gmtime_r(&stamp, &tmbuf);
		if (!gmt) {
			goto mdtm_error;
		}
		gmt->tm_isdst = -1;

		/* apply the GMT offset */
		tm.tm_sec += (long)(stamp - mktime(gmt));
		tm.tm_isdst = gmt->tm_isdst;

		ssb->sb.st_mtime = mktime(&tm);
	} else {
		/* error or unsupported command */
mdtm_error:
		ssb->sb.st_mtime = -1;
	}

	ssb->sb.st_ino = 0;						/* Unknown values */
	ssb->sb.st_dev = 0;
	ssb->sb.st_uid = 0;
	ssb->sb.st_gid = 0;
	ssb->sb.st_atime = -1;
	ssb->sb.st_ctime = -1;

	ssb->sb.st_nlink = 1;
	ssb->sb.st_rdev = -1;
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	ssb->sb.st_blksize = 4096;				/* Guess since FTP won't expose this information */
#ifdef HAVE_ST_BLOCKS
	ssb->sb.st_blocks = (int)((4095 + ssb->sb.st_size) / ssb->sb.st_blksize); /* emulate ceil */
#endif
#endif
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	pthreads_url_free(resource);
	return 0;

stat_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}
	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return -1;
}
/* }}} */

/* {{{ pthreads_stream_ftp_unlink */
static int pthreads_stream_ftp_unlink(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_url *resource = NULL;
	int result;
	char tmp_line[512];

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, url, "r", 0, NULL, threaded_context, NULL, &resource, NULL, NULL);
	if (!threaded_stream) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto unlink_errexit;
	}

	if (resource->path == NULL) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto unlink_errexit;
	}

	/* Attempt to delete the file */
	pthreads_stream_printf(threaded_stream, "DELE %s\r\n", (resource->path != NULL ? ZSTR_VAL(resource->path) : "/"));

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result < 200 || result > 299) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Error Deleting file: %s", tmp_line);
		}
		goto unlink_errexit;
	}

	pthreads_url_free(resource);
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	return 1;

unlink_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}
	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return 0;
}
/* }}} */

/* {{{ pthreads_stream_ftp_rename */
static int pthreads_stream_ftp_rename(pthreads_stream_wrapper_t *threaded_wrapper, const char *url_from, const char *url_to, int options, pthreads_stream_context_t *threaded_context)
{
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_url *resource_from = NULL, *resource_to = NULL;
	int result;
	char tmp_line[512];

	resource_from = pthreads_url_parse(url_from);
	resource_to = pthreads_url_parse(url_to);
	/* Must be same scheme (ftp/ftp or ftps/ftps), same host, and same port
		(or a 21/0 0/21 combination which is also "same")
	   Also require paths to/from */
	if (!resource_from ||
		!resource_to ||
		!resource_from->scheme ||
		!resource_to->scheme ||
		!zend_string_equals(resource_from->scheme, resource_to->scheme) ||
		!resource_from->host ||
		!resource_to->host ||
		!zend_string_equals(resource_from->host, resource_to->host) ||
		(resource_from->port != resource_to->port &&
		 resource_from->port * resource_to->port != 0 &&
		 resource_from->port + resource_to->port != 21) ||
		!resource_from->path ||
		!resource_to->path) {
		goto rename_errexit;
	}

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, url_from, "r", 0, NULL, threaded_context, NULL, NULL, NULL, NULL);
	if (!threaded_stream) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Unable to connect to %s", ZSTR_VAL(resource_from->host));
		}
		goto rename_errexit;
	}

	/* Rename FROM */
	pthreads_stream_printf(threaded_stream, "RNFR %s\r\n", (resource_from->path != NULL ? ZSTR_VAL(resource_from->path) : "/"));

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result < 300 || result > 399) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Error Renaming file: %s", tmp_line);
		}
		goto rename_errexit;
	}

	/* Rename TO */
	pthreads_stream_printf(threaded_stream, "RNTO %s\r\n", (resource_to->path != NULL ? ZSTR_VAL(resource_to->path) : "/"));

	result = PTHREADS_GET_FTP_RESULT(threaded_stream);
	if (result < 200 || result > 299) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Error Renaming file: %s", tmp_line);
		}
		goto rename_errexit;
	}

	pthreads_url_free(resource_from);
	pthreads_url_free(resource_to);
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	return 1;

rename_errexit:
	if (resource_from) {
		pthreads_url_free(resource_from);
	}
	if (resource_to) {
		pthreads_url_free(resource_to);
	}
	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return 0;
}
/* }}} */

/* {{{ pthreads_stream_ftp_mkdir */
static int pthreads_stream_ftp_mkdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int mode, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_url *resource = NULL;
	int result, recursive = options & PTHREADS_STREAM_MKDIR_RECURSIVE;
	char tmp_line[512];

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, url, "r", 0, NULL, threaded_context, NULL, &resource, NULL, NULL);
	if (!threaded_stream) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto mkdir_errexit;
	}

	if (resource->path == NULL) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto mkdir_errexit;
	}

	if (!recursive) {
		pthreads_stream_printf(threaded_stream, "MKD %s\r\n", ZSTR_VAL(resource->path));
		result = PTHREADS_GET_FTP_RESULT(threaded_stream);
    } else {
        /* we look for directory separator from the end of string, thus hopefuly reducing our work load */
        char *p, *e, *buf;

        buf = estrndup(ZSTR_VAL(resource->path), ZSTR_LEN(resource->path));
        e = buf + ZSTR_LEN(resource->path);

        /* find a top level directory we need to create */
        while ((p = strrchr(buf, '/'))) {
            *p = '\0';
			pthreads_stream_printf(threaded_stream, "CWD %s\r\n", buf);
			result = PTHREADS_GET_FTP_RESULT(threaded_stream);
			if (result >= 200 && result <= 299) {
				*p = '/';
				break;
			}
        }
        if (p == buf) {
			pthreads_stream_printf(threaded_stream, "MKD %s\r\n", ZSTR_VAL(resource->path));
			result = PTHREADS_GET_FTP_RESULT(threaded_stream);
        } else {
			pthreads_stream_printf(threaded_stream, "MKD %s\r\n", buf);
			result = PTHREADS_GET_FTP_RESULT(threaded_stream);
			if (result >= 200 && result <= 299) {
				if (!p) {
					p = buf;
				}
				/* create any needed directories if the creation of the 1st directory worked */
				while (++p != e) {
					if (*p == '\0' && *(p + 1) != '\0') {
						*p = '/';
						pthreads_stream_printf(threaded_stream, "MKD %s\r\n", buf);
						result = PTHREADS_GET_FTP_RESULT(threaded_stream);
						if (result < 200 || result > 299) {
							if (options & PTHREADS_REPORT_ERRORS) {
								php_error_docref(NULL, E_WARNING, "%s", tmp_line);
							}
							break;
						}
					}
				}
			}
		}
        efree(buf);
    }

	pthreads_url_free(resource);
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);

	if (result < 200 || result > 299) {
		/* Failure */
		return 0;
	}

	return 1;

mkdir_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}
	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return 0;
}
/* }}} */

/* {{{ pthreads_stream_ftp_rmdir */
static int pthreads_stream_ftp_rmdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_url *resource = NULL;
	int result;
	char tmp_line[512];

	threaded_stream = pthreads_ftp_fopen_connect(threaded_wrapper, url, "r", 0, NULL, threaded_context, NULL, &resource, NULL, NULL);
	if (!threaded_stream) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Unable to connect to %s", url);
		}
		goto rmdir_errexit;
	}

	if (resource->path == NULL) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "Invalid path provided in %s", url);
		}
		goto rmdir_errexit;
	}

	pthreads_stream_printf(threaded_stream, "RMD %s\r\n", ZSTR_VAL(resource->path));
	result = PTHREADS_GET_FTP_RESULT(threaded_stream);

	if (result < 200 || result > 299) {
		if (options & PTHREADS_REPORT_ERRORS) {
			php_error_docref(NULL, E_WARNING, "%s", tmp_line);
		}
		goto rmdir_errexit;
	}

	pthreads_url_free(resource);
	pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);

	return 1;

rmdir_errexit:
	if (resource) {
		pthreads_url_free(resource);
	}

	if (threaded_stream) {
		pthreads_stream_close(threaded_stream, PTHREADS_STREAM_FREE_CLOSE);
	}
	return 0;
}
/* }}} */

const pthreads_stream_wrapper_ops pthreads_ftp_stream_wops = {
	pthreads_stream_url_wrap_ftp,
	pthreads_stream_ftp_stream_close, /* stream_close */
	pthreads_stream_ftp_stream_stat,
	pthreads_stream_ftp_url_stat, /* stat_url */
	pthreads_stream_ftp_opendir, /* opendir */
	"ftp",
	pthreads_stream_ftp_unlink, /* unlink */
	pthreads_stream_ftp_rename, /* rename */
	pthreads_stream_ftp_mkdir,  /* mkdir */
	pthreads_stream_ftp_rmdir,  /* rmdir */
	NULL
};

#endif
