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
#ifndef HAVE_PHP_PTHREADS
#define HAVE_PHP_PTHREADS

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_LOCK_H
#	include <src/lock.h>
#endif

#ifndef HAVE_PHP_PTHREADS_H
#	include <php_pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STATE_H
#	include <src/state.h>
#endif

#ifndef HAVE_PTHREADS_THREAD_H
#	include <src/thread.h>
#endif

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if COMPILE_DL_PTHREADS
	ZEND_GET_MODULE(pthreads)
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

typedef void (*zend_throw_exception_hook_func)(zval * TSRMLS_DC);

zend_throw_exception_hook_func zend_throw_exception_hook_function = NULL;

zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_PTHREADS_EXTNAME,
  NULL,
  PHP_MINIT(pthreads),
  PHP_MSHUTDOWN(pthreads),
  PHP_RINIT(pthreads),
  PHP_RSHUTDOWN(pthreads),
  PHP_MINFO(pthreads),
  PHP_PTHREADS_VERSION,
  STANDARD_MODULE_PROPERTIES
};

zend_class_entry *pthreads_threaded_entry;
zend_class_entry *pthreads_thread_entry;
zend_class_entry *pthreads_worker_entry;
zend_class_entry *pthreads_mutex_entry;
zend_class_entry *pthreads_condition_entry;
zend_class_entry *pthreads_collectable_entry;
zend_class_entry *pthreads_pool_entry;

zend_object_handlers pthreads_handlers;
zend_object_handlers *zend_handlers;
void ***pthreads_instance = NULL;

#ifndef HAVE_SPL
zend_class_entry *spl_ce_InvalidArgumentException;
zend_class_entry *spl_ce_Countable;
zend_class_entry *spl_ce_RuntimeException;
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_MODIFIERS_H
#	include <src/modifiers.h>
#endif

#ifndef PTHREADS_NAME
#	define PTHREADS_NAME Z_OBJCE_P(getThis())->name
#endif

#ifndef PTHREADS_TID
#	define PTHREADS_TID thread->tid
#endif

#ifndef PTHREADS_FRIENDLY_NAME
#	define PTHREADS_FRIENDLY_NAME PTHREADS_NAME, PTHREADS_TID
#endif

ZEND_DECLARE_MODULE_GLOBALS(pthreads)

#ifndef HAVE_PTHREADS_ITERATOR_DEFAULT
# include <iterators/default.h>
#endif

static inline void pthreads_globals_ctor(zend_pthreads_globals *pg TSRMLS_DC) {
	pg->pointer = NULL;
	pg->pid = 0L;
	pg->signal = 0;
	pg->resources = NULL;
	pg->resolve = NULL;
}

void pthreads_throw_exception_hook(zval *ex TSRMLS_DC) {
	if (PTHREADS_ZG(pointer)) {
		if (EG(user_exception_handler)) {
			zend_fcall_info fci;
			zend_fcall_info_cache fcc;
			zval *retval = NULL;
			zval object = *ex;
			zval *saved = &object;
			char *cname = NULL;
		
			zval_copy_ctor(saved);
		
			if (zend_fcall_info_init(EG(user_exception_handler), IS_CALLABLE_CHECK_SILENT, &fci, &fcc, &cname, NULL TSRMLS_CC) == SUCCESS) {
				fci.retval_ptr_ptr = &retval;

				EG(exception) = NULL;
			
				zend_fcall_info_argn(&fci TSRMLS_CC, 1, &saved);
				zend_call_function(&fci, &fcc TSRMLS_CC);
				zend_fcall_info_args_clear(&fci, 1);
			
				if (retval) {
					zval_ptr_dtor(&retval);
				}
			}

			zval_dtor(saved);
		
			if (cname)
				efree(cname);
		}
	}
	
	if (zend_throw_exception_hook_function) {
		zend_throw_exception_hook_function(ex TSRMLS_CC);
	}
}

PHP_MINIT_FUNCTION(pthreads)
{
	zend_class_entry ce;
	
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_ALL", PTHREADS_INHERIT_ALL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_NONE", PTHREADS_INHERIT_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INI", PTHREADS_INHERIT_INI, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CONSTANTS", PTHREADS_INHERIT_CONSTANTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CLASSES", PTHREADS_INHERIT_CLASSES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_FUNCTIONS", PTHREADS_INHERIT_FUNCTIONS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INCLUDES", PTHREADS_INHERIT_INCLUDES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_COMMENTS", PTHREADS_INHERIT_COMMENTS, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("PTHREADS_ALLOW_HEADERS", PTHREADS_ALLOW_HEADERS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_ALLOW_GLOBALS", PTHREADS_ALLOW_GLOBALS, CONST_CS | CONST_PERSISTENT);
	
	INIT_CLASS_ENTRY(ce, "Threaded", pthreads_threaded_methods);
	ce.serialize = pthreads_internal_serialize;
	ce.unserialize = pthreads_internal_unserialize;
	pthreads_threaded_entry=zend_register_internal_class(&ce TSRMLS_CC);
	pthreads_threaded_entry->get_iterator = pthreads_object_iterator_ctor;
	pthreads_threaded_entry->create_object = pthreads_threaded_ctor;
	zend_class_implements(pthreads_threaded_entry TSRMLS_CC, 1, zend_ce_traversable);

	{
		/* NOTE: SPL SUCKS ASS */
		zend_class_entry **spl = NULL;

		if (zend_hash_find(CG(class_table), "countable", sizeof("countable"), (void**) &spl) == SUCCESS) {
			spl_ce_Countable = *spl;

			zend_class_implements(
				pthreads_threaded_entry TSRMLS_CC, 1, spl_ce_Countable);
		}
	}
	
	/* for BC with <= 1* */
	zend_register_class_alias_ex(
		ZEND_STRL("Stackable"), pthreads_threaded_entry TSRMLS_CC);

	INIT_CLASS_ENTRY(ce, "Thread", pthreads_thread_methods);
	ce.serialize = pthreads_internal_serialize;
	ce.unserialize = pthreads_internal_unserialize;
	pthreads_thread_entry=zend_register_internal_class_ex(&ce, pthreads_threaded_entry, NULL TSRMLS_CC);
	pthreads_thread_entry->get_iterator = pthreads_object_iterator_ctor;
	pthreads_thread_entry->create_object = pthreads_thread_ctor;
	
	INIT_CLASS_ENTRY(ce, "Worker", pthreads_worker_methods);
	ce.serialize = pthreads_internal_serialize;
	ce.unserialize = pthreads_internal_unserialize;
	pthreads_worker_entry=zend_register_internal_class_ex(&ce, pthreads_thread_entry, NULL TSRMLS_CC);
	pthreads_worker_entry->get_iterator = pthreads_object_iterator_ctor;
	pthreads_worker_entry->create_object = pthreads_worker_ctor;

	INIT_CLASS_ENTRY(ce, "Mutex", pthreads_mutex_methods);
	ce.serialize = zend_class_serialize_deny;
	ce.unserialize = zend_class_unserialize_deny;
	pthreads_mutex_entry=zend_register_internal_class(&ce TSRMLS_CC);
	pthreads_mutex_entry->ce_flags |= ZEND_ACC_FINAL;
	
	INIT_CLASS_ENTRY(ce, "Cond", pthreads_condition_methods);
	ce.serialize = zend_class_serialize_deny;
	ce.unserialize = zend_class_unserialize_deny;
	pthreads_condition_entry=zend_register_internal_class(&ce TSRMLS_CC);
	pthreads_condition_entry->ce_flags |= ZEND_ACC_FINAL;

	INIT_CLASS_ENTRY(ce, "Collectable", pthreads_collectable_methods);
	pthreads_collectable_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_entry, NULL TSRMLS_CC);
	zend_declare_property_bool(pthreads_collectable_entry, ZEND_STRL("garbage"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);	

	INIT_CLASS_ENTRY(ce, "Pool", pthreads_pool_methods);
	pthreads_pool_entry=zend_register_internal_class(&ce TSRMLS_CC);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("size"), 1, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("class"),   ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("workers"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("work"),    ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("ctor"),    ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("options"),    ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("last"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);

	/*
	* Setup standard and pthreads object handlers
	*/
	zend_handlers = zend_get_std_object_handlers();
	
	memcpy(&pthreads_handlers, zend_handlers, sizeof(zend_object_handlers));

    pthreads_handlers.cast_object = pthreads_cast_object;
    pthreads_handlers.count_elements = pthreads_count_properties;
    
	pthreads_handlers.get_debug_info = pthreads_read_debug;	
	pthreads_handlers.get_properties = pthreads_read_properties;
	
	pthreads_handlers.get_method = pthreads_get_method;
	pthreads_handlers.call_method = pthreads_call_method;
	
	pthreads_handlers.read_property = pthreads_read_property;
	pthreads_handlers.write_property = pthreads_write_property;
	pthreads_handlers.has_property = pthreads_has_property;
	pthreads_handlers.unset_property = pthreads_unset_property;
	
	pthreads_handlers.read_dimension = pthreads_read_dimension;
	pthreads_handlers.write_dimension = pthreads_write_dimension;
	pthreads_handlers.has_dimension = pthreads_has_dimension;
	pthreads_handlers.unset_dimension = pthreads_unset_dimension;
	
	pthreads_handlers.get_property_ptr_ptr = NULL;
	pthreads_handlers.get = NULL;
	pthreads_handlers.set = NULL;
	
#if PHP_VERSION_ID > 50399
    /* when the gc runs, it will fetch properties, every time */
    /* so we pass in a dummy function to control memory usage */
    /* properties copied will be destroyed with the object */
    pthreads_handlers.get_gc = NULL;
#endif

	pthreads_handlers.clone_obj = pthreads_clone_object; 

	ZEND_INIT_MODULE_GLOBALS(pthreads, pthreads_globals_ctor, NULL);	

	if (pthreads_globals_init(TSRMLS_C)) {
		/*
		* Global Init
		*/
		pthreads_instance = TSRMLS_C;
	}

#ifndef HAVE_SPL
	spl_ce_InvalidArgumentException = zend_exception_get_default(TSRMLS_C);
	spl_ce_Countable                = zend_exception_get_default(TSRMLS_C);
	spl_ce_RuntimeException			= zend_exception_get_default(TSRMLS_C);
#endif
	
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pthreads)
{
	if (pthreads_instance == TSRMLS_C) {
		pthreads_globals_shutdown(TSRMLS_C);
	}
	
	return SUCCESS;
}

static inline void pthreads_registered_dtor(void *ptr) {
    efree(*((void**)ptr));
}

PHP_RINIT_FUNCTION(pthreads) {
	ALLOC_HASHTABLE(PTHREADS_ZG(resolve));
	zend_hash_init(PTHREADS_ZG(resolve), 15, NULL, NULL, 0);
	
	ALLOC_HASHTABLE(PTHREADS_ZG(cache));
	zend_hash_init(PTHREADS_ZG(cache), 15, NULL, NULL, 0);
}

PHP_RSHUTDOWN_FUNCTION(pthreads) {
	zend_hash_destroy(PTHREADS_ZG(resolve));
	FREE_HASHTABLE(PTHREADS_ZG(resolve));

	if (PTHREADS_ZG(resources)) {
		zend_hash_destroy(PTHREADS_ZG(resources));	
		FREE_HASHTABLE(PTHREADS_ZG(resources));
		PTHREADS_ZG(resources) = NULL;
	}
	
	zend_hash_destroy(PTHREADS_ZG(cache));
	FREE_HASHTABLE(PTHREADS_ZG(cache));
}

PHP_MINFO_FUNCTION(pthreads)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_PTHREADS_VERSION);
	php_info_print_table_end();
}

#ifndef HAVE_PTHREADS_CLASS_THREADED
#	include <classes/threaded.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_THREAD
#	include <classes/thread.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_WORKER
#	include <classes/worker.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_MUTEX
#	include <classes/mutex.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_COND
#	include <classes/cond.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_COLLECTABLE
#	include <classes/collectable.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_POOL
#	include <classes/pool.h>
#endif

#endif
