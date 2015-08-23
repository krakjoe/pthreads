/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_RESOURCES_H
#define HAVE_PTHREADS_RESOURCES_H

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct {
	zend_resource *original;
	void ***ls;
} *pthreads_resource;

zend_bool pthreads_resources_keep(pthreads_resource data);
zend_bool pthreads_resources_kept(zend_resource *entry);
#endif

