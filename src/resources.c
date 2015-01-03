/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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

/* {{{ mark a resource for keeping */
zend_bool pthreads_resources_keep(pthreads_resource data TSRMLS_DC) {
	if (!PTHREADS_ZG(resources)) {
		ALLOC_HASHTABLE(PTHREADS_ZG(resources));
		zend_hash_init(PTHREADS_ZG(resources), 15, NULL, NULL, 1);
	}
	
	if (zend_hash_update(PTHREADS_ZG(resources),
			(char*) data->copy, sizeof(void*),
			(void**) &data, sizeof(void*), NULL) == SUCCESS) {
		return 1;
	}
	return 0;
} /* }}} */

/* {{{ tells if a resource is being kept */
zend_bool pthreads_resources_kept(zend_rsrc_list_entry *entry TSRMLS_DC) {
	if (entry) {
		pthreads_resource *data = NULL;
		if (PTHREADS_ZG(resources) && zend_hash_find(PTHREADS_ZG(resources), 
			(char*) entry, sizeof(void*), (void**) &data)==SUCCESS) {	
			if (data && (*data)->ls != TSRMLS_C) {
				return 1;
			}
		}
	}
	return 0;
} /* }}} */

#endif

