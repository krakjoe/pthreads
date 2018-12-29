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
#ifndef HAVE_PTHREADS_STREAMS_STANDARD_FILTERS
#define HAVE_PTHREADS_STREAMS_STANDARD_FILTERS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_STANDARD_FILTERS_H
#	include <src/streams/standard_filters.h>
#endif

#ifndef HAVE_PTHREADS_STREAMS_STRING_FILTERS
#	include <src/streams/string_filters.c>
#endif

static const struct {
	const pthreads_stream_filter_ops *ops;
	const pthreads_stream_filter_factory *factory;
} pthreads_standard_filters[] = {
	{ &pthreads_strfilter_rot13_ops, &pthreads_strfilter_rot13_factory },
	{ &pthreads_strfilter_toupper_ops, &pthreads_strfilter_toupper_factory },
	{ &pthreads_strfilter_tolower_ops, &pthreads_strfilter_tolower_factory },
	/* additional filters to go here */
	{ NULL, NULL }
};

/* {{{ filter init and shutdown */
int standard_filters_init() {
	int i;

	for (i = 0; pthreads_standard_filters[i].ops; i++) {
		if (FAILURE == pthreads_stream_filter_register_factory(
				pthreads_standard_filters[i].ops->label,
				pthreads_standard_filters[i].factory
				)) {
			return FAILURE;
		}
	}
	return SUCCESS;
}

int standard_filters_shutdown() {
	int i;

	for (i = 0; pthreads_standard_filters[i].ops; i++) {
		pthreads_stream_filter_unregister_factory(pthreads_standard_filters[i].ops->label);
	}
	return SUCCESS;
}
/* }}} */

#endif
