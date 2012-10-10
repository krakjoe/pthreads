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
#ifndef HAVE_PTHREADS_COMPAT_H
#define HAVE_PTHREADS_COMPAT_H

#ifndef HAVE_PTHREADS_H
#	include <ext/pthreads/src/pthreads.h>
#endif

#if PHP_VERSION_ID > 50399
void pthreads_method_del_ref(zend_function *function);
#endif /* PHP_VERSION_ID > 50399 */

#endif /* HAVE_PTHREADS_COMPAT_H */
