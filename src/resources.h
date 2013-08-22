/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                                		 |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <joe.watkins@live.co.uk>                         |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_RESOURCES_H
#define HAVE_PTHREADS_RESOURCES_H

typedef struct _pthreads_resources *pthreads_resources;

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

struct _pthreads_resources {
	TsHashTable keep;
};

typedef struct {
	void 				***ls;
	zend_class_entry	*scope;
} *pthreads_resource;

pthreads_resources pthreads_resources_alloc(TSRMLS_D);
zend_bool pthreads_resources_keep(pthreads_resources resources, zend_rsrc_list_entry *entry, pthreads_resource data TSRMLS_DC);
zend_bool pthreads_resources_kept(pthreads_resources resources, zend_rsrc_list_entry *entry TSRMLS_DC);
void pthreads_resources_free(pthreads_resources resources TSRMLS_DC);

#endif

