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

#ifndef HAVE_PHP_PTHREADS_H
#	include <php_pthreads.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef ZTS
#	error "pthreads requires that Thread Safety is enabled, add --enable-maintainer-zts to your PHP build configuration"
#endif

#if COMPILE_DL_PTHREADS
	ZEND_TSRMLS_CACHE_DEFINE();
	ZEND_GET_MODULE(pthreads)
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_COPY_H
#	include <src/copy.h>
#endif

ZEND_BEGIN_ARG_INFO_EX(pthreads_no_sleeping_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

PHP_FUNCTION(pthreads_no_sleeping) {
	zend_throw_exception_ex(spl_ce_RuntimeException, 0,
		"%s is not suitable for use in multi threaded applications, use synchronized Threaded::wait",
		ZSTR_VAL(EX(func)->common.function_name));
}

static zend_function_entry pthreads_functions[] = {
	PHP_FE(pthreads_no_sleeping, pthreads_no_sleeping_arginfo)
	PHP_FE_END
};

zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_PTHREADS_EXTNAME,
  pthreads_functions,
  PHP_MINIT(pthreads),
  PHP_MSHUTDOWN(pthreads),
  PHP_RINIT(pthreads),
  PHP_RSHUTDOWN(pthreads),
  PHP_MINFO(pthreads),
  PHP_PTHREADS_VERSION,
  NO_MODULE_GLOBALS,
  ZEND_MODULE_POST_ZEND_DEACTIVATE_N(pthreads),
  STANDARD_MODULE_PROPERTIES_EX
};

zend_class_entry *pthreads_threaded_entry;
zend_class_entry *pthreads_volatile_entry;
zend_class_entry *pthreads_thread_entry;
zend_class_entry *pthreads_worker_entry;
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

ZEND_DECLARE_MODULE_GLOBALS(pthreads)

typedef struct _pthreads_supported_sapi_t {
	const char *name;
	size_t      nlen;
} pthreads_supported_sapi_t;

const static pthreads_supported_sapi_t whitelist[] = {
	{ZEND_STRL("cli")},
	{ZEND_STRL("phpdbg")}, /* not really supported, needs work */
	{NULL, 0}
};

static inline zend_bool pthreads_is_supported_sapi(char *name) {
	const pthreads_supported_sapi_t *sapi = whitelist;
	zend_long nlen = strlen(name);

	while (sapi->name) {
		if (nlen == sapi->nlen && 
			memcmp(sapi->name, name, nlen) == SUCCESS) {
			return 1;
		}
		sapi++;
	}

	return 0;
}

typedef void (*zend_execute_ex_function)(zend_execute_data *);

zend_execute_ex_function zend_execute_ex_hook = NULL;

static inline void pthreads_globals_ctor(zend_pthreads_globals *pg) {
	ZVAL_UNDEF(&pg->this);
	pg->pid = 0L;
	pg->signal = 0;
	pg->resources = NULL;
}

static inline void pthreads_replace_internal_function(char *oname, size_t olen, char *rname, size_t rlen) {
	zend_function *functions[2];

	functions[0] = zend_hash_str_find_ptr(EG(function_table), oname, olen);
	functions[1] = zend_hash_str_find_ptr(EG(function_table), rname, rlen);

	if (functions[0] && functions[1]) {
		zend_string *rename = zend_string_copy(functions[0]->common.function_name);
		if ((functions[1] = zend_hash_str_update_mem(
				EG(function_table), oname, olen, functions[1], sizeof(zend_internal_function)))) {
			functions[1]->common.function_name = rename;
		} else zend_string_release(rename);
	}
}

/* {{{ */
static inline void pthreads_execute_ex(zend_execute_data *data) {
	if (zend_execute_ex_hook) {
		zend_execute_ex_hook(data);
	} else execute_ex(data);
	
	if (EG(exception))
		zend_try_exception_handler();
} /* }}} */

PHP_MINIT_FUNCTION(pthreads)
{
	zend_class_entry ce;

	if (!pthreads_is_supported_sapi(sapi_module.name)) {
		zend_error(E_ERROR, "The %s SAPI is not supported by pthreads",
			sapi_module.name);
		return FAILURE;
	}

	zend_execute_ex_hook = zend_execute_ex;
	zend_execute_ex = pthreads_execute_ex;	

	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_ALL", PTHREADS_INHERIT_ALL, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_NONE", PTHREADS_INHERIT_NONE, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INI", PTHREADS_INHERIT_INI, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CONSTANTS", PTHREADS_INHERIT_CONSTANTS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_CLASSES", PTHREADS_INHERIT_CLASSES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_FUNCTIONS", PTHREADS_INHERIT_FUNCTIONS, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_INCLUDES", PTHREADS_INHERIT_INCLUDES, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("PTHREADS_INHERIT_COMMENTS", PTHREADS_INHERIT_COMMENTS, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("PTHREADS_ALLOW_HEADERS", PTHREADS_ALLOW_HEADERS, CONST_CS | CONST_PERSISTENT);

#ifdef PTHREADS_KILL_SIGNAL
	REGISTER_LONG_CONSTANT("PTHREADS_KILL_THREAD", PTHREADS_KILL_SIGNAL, CONST_CS | CONST_PERSISTENT);
#endif

	INIT_CLASS_ENTRY(ce, "Threaded", pthreads_threaded_methods);
	pthreads_threaded_entry=zend_register_internal_class(&ce);
	pthreads_threaded_entry->get_iterator = pthreads_object_iterator_create;
	pthreads_threaded_entry->create_object = pthreads_threaded_ctor;
	pthreads_threaded_entry->serialize = pthreads_threaded_serialize;
	pthreads_threaded_entry->unserialize = pthreads_threaded_unserialize;
	zend_class_implements(pthreads_threaded_entry, 1, zend_ce_traversable);

	INIT_CLASS_ENTRY(ce, "Volatile", NULL);
	pthreads_volatile_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	
	INIT_CLASS_ENTRY(ce, "Thread", pthreads_thread_methods);
	pthreads_thread_entry=zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	pthreads_thread_entry->create_object = pthreads_thread_ctor;

	INIT_CLASS_ENTRY(ce, "Collectable", pthreads_collectable_methods);
	pthreads_collectable_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	zend_declare_property_bool(pthreads_collectable_entry, ZEND_STRL("garbage"), 0, ZEND_ACC_PROTECTED);	

	INIT_CLASS_ENTRY(ce, "Worker", pthreads_worker_methods);
	pthreads_worker_entry=zend_register_internal_class_ex(&ce, pthreads_thread_entry);
	pthreads_worker_entry->create_object = pthreads_worker_ctor;	

	INIT_CLASS_ENTRY(ce, "Pool", pthreads_pool_methods);
	pthreads_pool_entry=zend_register_internal_class(&ce);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("size"), 1, ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("class"),   ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("workers"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(pthreads_pool_entry, ZEND_STRL("ctor"),    ZEND_ACC_PROTECTED);
	zend_declare_property_long(pthreads_pool_entry, ZEND_STRL("last"), 0, ZEND_ACC_PROTECTED);

	/*
	* Setup standard and threaded object handlers
	*/
	zend_handlers = zend_get_std_object_handlers();
	
	memcpy(&pthreads_handlers, zend_handlers, sizeof(zend_object_handlers));

	pthreads_handlers.offset = XtOffsetOf(pthreads_object_t, std);

	pthreads_handlers.free_obj = pthreads_base_free;
	pthreads_handlers.cast_object = pthreads_cast_object;
	pthreads_handlers.count_elements = pthreads_count_properties;

	pthreads_handlers.get_debug_info = pthreads_read_debug;	
	pthreads_handlers.get_properties = pthreads_read_properties;
	
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
	pthreads_handlers.get_gc = pthreads_base_gc;

	pthreads_handlers.clone_obj = pthreads_base_clone; 
	pthreads_handlers.compare_objects = pthreads_compare_objects;

	ZEND_INIT_MODULE_GLOBALS(pthreads, pthreads_globals_ctor, NULL);	

	if (pthreads_globals_init()) {
		TSRMLS_CACHE_UPDATE();
		
		/*
		* Global Init
		*/
		pthreads_instance = TSRMLS_CACHE;
	}

#ifndef HAVE_SPL
	spl_ce_InvalidArgumentException = zend_exception_get_default();
	spl_ce_Countable                = zend_exception_get_default();
	spl_ce_RuntimeException		= zend_exception_get_default();
#endif

	return SUCCESS;
}

static inline int sapi_cli_deactivate(void) 
{
	fflush(stdout);
	if (SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}
	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(pthreads)
{
	if (pthreads_instance == TSRMLS_CACHE) {
		pthreads_globals_shutdown();

		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = sapi_cli_deactivate;
		}
	}

	zend_execute_ex = zend_execute_ex_hook;

	return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(pthreads)
{
	if (PTHREADS_ZG(resources)) {
		zend_hash_destroy(PTHREADS_ZG(resources));
		FREE_HASHTABLE(PTHREADS_ZG(resources));
		PTHREADS_ZG(resources) = NULL;
	}

	return SUCCESS;
}

PHP_RINIT_FUNCTION(pthreads) {
	ZEND_TSRMLS_CACHE_UPDATE();

	zend_hash_init(&PTHREADS_ZG(resolve), 15, NULL, NULL, 0);

	if (pthreads_instance != TSRMLS_CACHE) {
		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = NULL;
		}
	}

	pthreads_replace_internal_function(ZEND_STRL("usleep"), ZEND_STRL("pthreads_no_sleeping"));
	pthreads_replace_internal_function(ZEND_STRL("sleep"),  ZEND_STRL("pthreads_no_sleeping"));

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pthreads) {
	zend_hash_destroy(&PTHREADS_ZG(resolve));

	return SUCCESS;
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

#ifndef HAVE_PTHREADS_CLASS_COLLECTABLE
#	include <classes/collectable.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_WORKER
#	include <classes/worker.h>
#endif

#ifndef HAVE_PTHREADS_CLASS_POOL
#	include <classes/pool.h>
#endif

#endif
