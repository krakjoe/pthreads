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
#ifndef HAVE_PTHREADS_MODIFIERS_H
#define HAVE_PTHREADS_MODIFIERS_H

#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

typedef struct _pthreads_modifiers {
	HashTable modified;
	HashTable protection;
} *pthreads_modifiers;

/* {{{ allocate modifiers object */
pthreads_modifiers pthreads_modifiers_alloc(); /* }}} */

/* {{{ initialize modifiers using the referenced class entry */
void pthreads_modifiers_init(pthreads_modifiers modifiers, zend_class_entry *entry); /* }}} */

/* {{{ set modifiers for a method */
int pthreads_modifiers_set(pthreads_modifiers modifiers, zend_string *method, int32_t modify); /* }}} */

/* {{{ get modifiers for a method */
int32_t pthreads_modifiers_get(pthreads_modifiers modifiers, zend_string *method); /* }}} */

/* {{{ protect a method call */
zend_bool pthreads_modifiers_protect(pthreads_modifiers modifiers, zend_string *method, zend_bool *unprotect); /* }}} */

/* {{{ unprotect a method call */
zend_bool pthreads_modifiers_unprotect(pthreads_modifiers modifiers, zend_string *method, zend_bool unprotect); /* }}} */

/* {{{ free modifiers object */
void pthreads_modifiers_free(pthreads_modifiers modifiers); /* }}} */

#endif
