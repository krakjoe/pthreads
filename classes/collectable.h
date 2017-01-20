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
#ifndef HAVE_PTHREADS_CLASS_COLLECTABLE_H
#define HAVE_PTHREADS_CLASS_COLLECTABLE_H
#if PHP_VERSION_ID >= 70200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Collectable_isGarbage, 0, 0, _IS_BOOL, 0)
#else
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(Collectable_isGarbage, 0, 0, _IS_BOOL, NULL, 0)
#endif
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_collectable_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_COLLECTABLE
#	define HAVE_PTHREADS_CLASS_COLLECTABLE
zend_function_entry pthreads_collectable_methods[] = {
	PHP_ABSTRACT_ME(Collectable, isGarbage, 	Collectable_isGarbage)
	{NULL, NULL, NULL}
};
#	endif
#endif
