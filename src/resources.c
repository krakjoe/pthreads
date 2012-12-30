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
#ifndef HAVE_PTHREADS_RESOURCES
#define HAVE_PTHREADS_RESOURCES

/*
* THIS IS A TEST | THIS IS A TEST | THIS IS A TEST | THIS IS A TEST
* IF IT DOESN'T WORK TOUGH SHIT !! | IF IT DOESN'T WORK TOUGH SHIT !!
*/

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

/* {{{ allocate resource structure */
pthreads_resources pthreads_resources_alloc(TSRMLS_D) {
	pthreads_resources resources = calloc(1, sizeof(*resources));
	if (resources) {
		zend_ts_hash_init(&resources->keep, 3, NULL, NULL, 1);
	}
	return resources;
} /* }}} */

/* {{{ mark a resource for keeping */
zend_bool pthreads_resources_keep(pthreads_resources resources, zend_rsrc_list_entry *entry TSRMLS_DC) {
	if (resources) {
		if (zend_ts_hash_update(
			&resources->keep, entry->ptr, sizeof(entry->ptr), TSRMLS_C, sizeof(TSRMLS_C), NULL
		) == SUCCESS) {
			return 1;
		}
	}
	return 0;
} /* }}} */

/* {{{ 
	For this to work, a "kept" resource has to be kept in the scope of an object in a thread
	I haven't yet worked out a way to do this, so this is incomplete and will cause errors 
}}} */

/* {{{ tells if a resource is being kept */
zend_bool pthreads_resources_kept(pthreads_resources resources, zend_rsrc_list_entry *entry TSRMLS_DC) {
	void **context;
	if (zend_ts_hash_find(&resources->keep, entry->ptr, sizeof(entry->ptr), (void**) &context)==SUCCESS) {
		return 1;
	} else return 0;
} /* }}} */

/* {{{ free resource structure */
void pthreads_resources_free(pthreads_resources resources TSRMLS_DC) {
	if (resources) {
		zend_ts_hash_destroy(
			&resources->keep
		);
		free(resources);
	}
} /* }}} */
#endif

