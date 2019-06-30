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
#ifndef HAVE_PTHREADS_STREAMS_CONTEXT_H
#define HAVE_PTHREADS_STREAMS_CONTEXT_H

#ifndef HAVE_PTHREADS_STORE_H
#	include "src/store.h"
#endif

/* Stream context and status notification related definitions */

/* callback for status notifications */
typedef void (*pthreads_stream_notification_func)(pthreads_stream_context *context,
		int notifycode, int severity,
		char *xmsg, int xcode,
		size_t bytes_sofar, size_t bytes_max,
		void * ptr);

#define PTHREADS_STREAM_NOTIFIER_PROGRESS	1

/* Attempt to fetch context from the zval passed,
   If no context was passed, use the default context
   The default context has not yet been created, do it now. */
#define pthreads_stream_context_from_zval(zcontext, nocontext) ( \
		(zcontext) && !Z_ISNULL_P(zcontext) ? PTHREADS_FETCH_FROM(Z_OBJ_P(zcontext)) : \
		(nocontext) ? NULL : PTHREADS_GET_DEF_CONTEXT )

#define pthreads_stream_context_to_zval(context, zval) { ZVAL_OBJ(zval, context); Z_ADDREF_P((zval)); }

typedef struct _pthreads_stream_notifier pthreads_stream_notifier;

struct _pthreads_stream_notifier {
	pthreads_stream_notification_func func;
	void (*dtor)(pthreads_stream_notifier *notifier);
	pthreads_storage *ptr;
	int mask;
	size_t progress, progress_max; /* position for progress notification */
};

struct _pthreads_stream_context {
	pthreads_stream_notifier *notifier;
	pthreads_object_t *options;	/* hash keyed by wrapper family or specific wrapper */
};

pthreads_stream_context *pthreads_stream_context_alloc(void);
void pthreads_stream_context_free(pthreads_stream_context *context);
zval *pthreads_stream_context_get_option(pthreads_stream_context_t *threaded_context, const char *wrappername, const char *optionname);
int pthreads_stream_context_set_option(pthreads_stream_context_t *threaded_context, const char *wrappername, const char *optionname, zval *optionvalue);
pthreads_stream_notifier *pthreads_stream_notification_alloc(void);
void pthreads_stream_notification_free(pthreads_stream_notifier *notifier);

/* not all notification codes are implemented */
#define PTHREADS_STREAM_NOTIFY_RESOLVE			1
#define PTHREADS_STREAM_NOTIFY_CONNECT			2
#define PTHREADS_STREAM_NOTIFY_AUTH_REQUIRED	3
#define PTHREADS_STREAM_NOTIFY_MIME_TYPE_IS		4
#define PTHREADS_STREAM_NOTIFY_FILE_SIZE_IS		5
#define PTHREADS_STREAM_NOTIFY_REDIRECTED		6
#define PTHREADS_STREAM_NOTIFY_PROGRESS			7
#define PTHREADS_STREAM_NOTIFY_COMPLETED		8
#define PTHREADS_STREAM_NOTIFY_FAILURE			9
#define PTHREADS_STREAM_NOTIFY_AUTH_RESULT		10

#define PTHREADS_STREAM_NOTIFY_SEVERITY_INFO	0
#define PTHREADS_STREAM_NOTIFY_SEVERITY_WARN	1
#define PTHREADS_STREAM_NOTIFY_SEVERITY_ERR		2

void pthreads_stream_notification_notify(pthreads_stream_context_t *threaded_context, int notifycode, int severity,
		char *xmsg, int xcode, size_t bytes_sofar, size_t bytes_max, void * ptr);

pthreads_stream_context_t *pthreads_stream_context_set(pthreads_stream_t *threaded_stream, pthreads_stream_context_t *threaded_context);

void pthreads_stream_notify_info(pthreads_stream_context_t *threaded_context, int code, char *xmsg, int xcode);
void pthreads_stream_notify_progress(pthreads_stream_context_t *threaded_context, size_t bsofar, size_t bmax);
void pthreads_stream_notify_progress_init(pthreads_stream_context_t *threaded_context, size_t sofar, size_t bmax);
void pthreads_stream_notify_progress_increment(pthreads_stream_context_t *threaded_context, size_t sofar, size_t max);
void pthreads_stream_notify_file_size(pthreads_stream_context_t *threaded_context, size_t file_size, char *xmsg, int xcode);
void pthreads_stream_notify_error(pthreads_stream_context_t *threaded_context, int code, char *xmsg, int xcode);

#endif
