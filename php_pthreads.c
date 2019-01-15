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

#if PHP_VERSION_ID < 70200
#	error "pthreads requires PHP 7.2, ZTS in versions 7.0 and 7.1 is broken"
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

static zend_internal_function zend_putenv_function;
static zend_internal_function zend_getenv_function;

PHP_FUNCTION(pthreads_putenv)
{
	pthreads_globals_lock();

	zend_try {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;

		fci.size = sizeof(zend_fcall_info);
		fci.params = ZEND_CALL_ARG(execute_data, 1);
		fci.param_count = ZEND_NUM_ARGS();
		fci.retval = return_value;
		fcc.function_handler = (zend_function*) &zend_putenv_function;
#if PHP_VERSION_ID <= 70300
		fcc.initialized = 1;
#endif

		zend_call_function(&fci, &fcc);
	} zend_end_try();

	pthreads_globals_unlock();
}

PHP_FUNCTION(pthreads_getenv)
{
	pthreads_globals_lock();

	zend_try {
		zend_fcall_info fci = empty_fcall_info;
		zend_fcall_info_cache fcc = empty_fcall_info_cache;

		fci.size = sizeof(zend_fcall_info);
		fci.params = ZEND_CALL_ARG(execute_data, 1);
		fci.param_count = ZEND_NUM_ARGS();
		fci.retval = return_value;
		fcc.function_handler = (zend_function*) &zend_getenv_function;
#if PHP_VERSION_ID <= 70300
		fcc.initialized = 1;
#endif

		zend_call_function(&fci, &fcc);
	} zend_end_try();

	pthreads_globals_unlock();
}

zend_function_entry php_pthreads_functions[] = {
	PHP_FE(pthreads_putenv, NULL)
	PHP_FE(pthreads_getenv, NULL)
	PHP_FE_END
};

zend_module_entry pthreads_module_entry = {
  STANDARD_MODULE_HEADER,
  PHP_PTHREADS_EXTNAME,
  php_pthreads_functions,
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
zend_class_entry *pthreads_socket_entry;

zend_object_handlers pthreads_handlers;
zend_object_handlers pthreads_socket_handlers;
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
	{ZEND_STRL("homegear")},
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

/* {{{ */
static inline void pthreads_execute_ex(zend_execute_data *data) {
	if (zend_execute_ex_hook) {
		zend_execute_ex_hook(data);
	} else execute_ex(data);
	
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		if (EG(exception) && 
			(!EG(current_execute_data) || !EG(current_execute_data)->prev_execute_data))
			zend_try_exception_handler();
	}
} /* }}} */ 

/* {{{ */
static inline zend_bool pthreads_verify_type(zend_execute_data *execute_data, zval *var, zend_arg_info *info) {
	if (!ZEND_TYPE_IS_SET(info->type)) {
		return 1;
	}

	if (ZEND_TYPE_IS_CLASS(info->type)) {
		pthreads_object_t *threaded;

		if (!var || 
			Z_TYPE_P(var) != IS_OBJECT || 
			!instanceof_function(Z_OBJCE_P(var), pthreads_threaded_entry)) {
			return 0;
		}

		threaded = PTHREADS_FETCH_FROM(Z_OBJ_P(var));

		if (!PTHREADS_IN_CREATOR(threaded)) {
			zend_class_entry *ce;
			void **cache = CACHE_ADDR(EX(opline)->op2.num);

			if (*cache) {
				ce = *cache;
			} else {
				ce = zend_fetch_class(ZEND_TYPE_NAME(info->type), 
					(ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD));
			
				if (!ce) {
					return Z_TYPE_P(var) == IS_NULL && ZEND_TYPE_ALLOW_NULL(info->type);
				}

				*cache = (void*) ce;
			}

			if (Z_TYPE_P(var) == IS_OBJECT) {
				zend_class_entry *instance = zend_fetch_class(
					threaded->std.ce->name, (ZEND_FETCH_CLASS_AUTO | ZEND_FETCH_CLASS_NO_AUTOLOAD));

				if (!instance) {
					return 0;
				}

				return instanceof_function(instance, ce);
			}
		}
	}

	return 0;
} /* }}} */

/* {{{ */
static inline int php_pthreads_recv(zend_execute_data *execute_data) {
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		uint32_t arg_num = EX(opline)->op1.num;	
		zval *var = NULL;

		if (UNEXPECTED(arg_num > EX_NUM_ARGS())) {
			return ZEND_USER_OPCODE_DISPATCH;	
		}

#if ZEND_USE_ABS_CONST_ADDR
		if (EX(opline)->result_type == IS_CONST) {
				var = (zval*) EX(opline)->result.var;	
		} else var = EX_VAR(EX(opline)->result.num);
#else
		var = EX_VAR(EX(opline)->result.num);
#endif

		if (UNEXPECTED((EX(func)->op_array.fn_flags & ZEND_ACC_HAS_TYPE_HINTS) != 0)) {
			if (pthreads_verify_type(execute_data, 
				var, 
				&EX(func)->common.arg_info[arg_num-1])) {
				EX(opline)++;
				return ZEND_USER_OPCODE_CONTINUE;
			}
		}
	}
	return ZEND_USER_OPCODE_DISPATCH;
} /* }}} */

/* {{{ */
static inline int php_pthreads_verify_return_type(zend_execute_data *execute_data) {
	if (Z_TYPE(PTHREADS_ZG(this)) != IS_UNDEF) {
		zval *var = NULL;
		
		if (EX(opline)->op1_type == IS_UNUSED) {	
			return ZEND_USER_OPCODE_DISPATCH;
		}

#if ZEND_USE_ABS_CONST_ADDR
		if (EX(opline)->op1_type & IS_CONST) {
			var = (zval*) EX(opline)->op1.var;
		} else var = EX_VAR(EX(opline)->op1.num);
#else
		var = EX_VAR(EX(opline)->op1.num);
#endif

		if (pthreads_verify_type(execute_data, 
			var,
			EX(func)->common.arg_info - 1)) {
			EX(opline)++;
			return ZEND_USER_OPCODE_CONTINUE;
		}
	}

	return ZEND_USER_OPCODE_DISPATCH;
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

	INIT_CLASS_ENTRY(ce, "Collectable", pthreads_collectable_methods);
	pthreads_collectable_entry = zend_register_internal_interface(&ce);

	INIT_CLASS_ENTRY(ce, "Threaded", pthreads_threaded_methods);
	pthreads_threaded_entry=zend_register_internal_class(&ce);
	pthreads_threaded_entry->get_iterator = pthreads_object_iterator_create;
	pthreads_threaded_entry->create_object = pthreads_threaded_ctor;
	pthreads_threaded_entry->serialize = pthreads_threaded_serialize;
	pthreads_threaded_entry->unserialize = pthreads_threaded_unserialize;
	zend_class_implements(pthreads_threaded_entry, 2, zend_ce_traversable, pthreads_collectable_entry);
	
	INIT_CLASS_ENTRY(ce, "Volatile", NULL);
	pthreads_volatile_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	
	INIT_CLASS_ENTRY(ce, "Thread", pthreads_thread_methods);
	pthreads_thread_entry=zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	pthreads_thread_entry->create_object = pthreads_thread_ctor;

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

	INIT_CLASS_ENTRY(ce, "Socket", pthreads_socket_methods);
	pthreads_socket_entry = zend_register_internal_class_ex(&ce, pthreads_threaded_entry);
	pthreads_socket_entry->create_object = pthreads_socket_ctor;

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_UNIX"), AF_UNIX);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_INET"), AF_INET);
#ifdef HAVE_IPV6
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("AF_INET6"), AF_INET6);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_STREAM"), SOCK_STREAM);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_DGRAM"), SOCK_DGRAM);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_RAW"), SOCK_RAW);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_SEQPACKET"), SOCK_SEQPACKET);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOCK_RDM"), SOCK_RDM);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_DEBUG"), SO_DEBUG);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_REUSEADDR"), SO_REUSEADDR);
#ifdef SO_REUSEPORT
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_REUSEPORT"), SO_REUSEPORT);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_KEEPALIVE"), SO_KEEPALIVE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_DONTROUTE"), SO_DONTROUTE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_LINGER"), SO_LINGER);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_BROADCAST"), SO_BROADCAST);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_OOBINLINE"), SO_OOBINLINE);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDBUF"), SO_SNDBUF);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVBUF"), SO_RCVBUF);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDLOWAT"), SO_SNDLOWAT);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVLOWAT"), SO_RCVLOWAT);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_SNDTIMEO"), SO_SNDTIMEO);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_RCVTIMEO"), SO_RCVTIMEO);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_TYPE"), SO_TYPE);
#ifdef SO_FAMILY
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_FAMILY"), SO_FAMILY);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_ERROR"), SO_ERROR);
#ifdef SO_BINDTODEVICE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SO_BINDTODEVICE"), SO_BINDTODEVICE);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOMAXCONN"), SOMAXCONN);
#ifdef TCP_NODELAY
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("TCP_NODELAY"), TCP_NODELAY);
#endif
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("NORMAL_READ"), PTHREADS_NORMAL_READ);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("BINARY_READ"), PTHREADS_BINARY_READ);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_SOCKET"), SOL_SOCKET);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_TCP"), IPPROTO_TCP);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("SOL_UDP"), IPPROTO_UDP);

	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_OOB"), MSG_OOB);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_WAITALL"), MSG_WAITALL);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CTRUNC"), MSG_CTRUNC);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_TRUNC"), MSG_TRUNC);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_PEEK"), MSG_PEEK);
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_DONTROUTE"), MSG_DONTROUTE);
#ifdef MSG_EOR
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_EOR"), MSG_EOR);
#endif
#ifdef MSG_EOF
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_EOF"), MSG_EOF);
#endif
#ifdef MSG_CONFIRM
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CONFIRM"), MSG_CONFIRM);
#endif
#ifdef MSG_ERRQUEUE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_ERRQUEUE"), MSG_ERRQUEUE);
#endif
#ifdef MSG_NOSIGNAL
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_NOSIGNAL"), MSG_NOSIGNAL);
#endif
#ifdef MSG_MORE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_MORE"), MSG_MORE);
#endif
#ifdef MSG_WAITFORONE
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_WAITFORONE"), MSG_WAITFORONE);
#endif
#ifdef MSG_CMSG_CLOEXEC
	zend_declare_class_constant_long(pthreads_socket_entry, ZEND_STRL("MSG_CMSG_CLOEXEC"), MSG_CMSG_CLOEXEC);
#endif

#ifndef _WIN32
#ifdef EPERM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPERM"), EPERM);
#endif
#ifdef ENOENT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOENT"), ENOENT);
#endif
#ifdef EINTR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINTR"), EINTR);
#endif
#ifdef EIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EIO"), EIO);
#endif
#ifdef ENXIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENXIO"), ENXIO);
#endif
#ifdef E2BIG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("E2BIG"), E2BIG);
#endif
#ifdef EBADF
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADF"), EBADF);
#endif
#ifdef EAGAIN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAGAIN"), EAGAIN);
#endif
#ifdef ENOMEM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMEM"), ENOMEM);
#endif
#ifdef EACCES
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EACCES"), EACCES);
#endif
#ifdef EFAULT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EFAULT"), EFAULT);
#endif
#ifdef ENOTBLK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTBLK"), ENOTBLK);
#endif
#ifdef EBUSY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBUSY"), EBUSY);
#endif
#ifdef EEXIST
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EEXIST"), EEXIST);
#endif
#ifdef EXDEV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EXDEV"), EXDEV);
#endif
#ifdef ENODEV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENODEV"), ENODEV);
#endif
#ifdef ENOTDIR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTDIR"), ENOTDIR);
#endif
#ifdef EISDIR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISDIR"), EISDIR);
#endif
#ifdef EINVAL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINVAL"), EINVAL);
#endif
#ifdef ENFILE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENFILE"), ENFILE);
#endif
#ifdef EMFILE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMFILE"), EMFILE);
#endif
#ifdef ENOTTY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTTY"), ENOTTY);
#endif
#ifdef ENOSPC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSPC"), ENOSPC);
#endif
#ifdef ESPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESPIPE"), ESPIPE);
#endif
#ifdef EROFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EROFS"), EROFS);
#endif
#ifdef EMLINK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMLINK"), EMLINK);
#endif
#ifdef EPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPIPE"), EPIPE);
#endif
#ifdef ENAMETOOLONG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENAMETOOLONG"), ENAMETOOLONG);
#endif
#ifdef ENOLCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOLCK"), ENOLCK);
#endif
#ifdef ENOSYS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSYS"), ENOSYS);
#endif
#ifdef ENOTEMPTY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTEMPTY"), ENOTEMPTY);
#endif
#ifdef ELOOP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELOOP"), ELOOP);
#endif
#ifdef EWOULDBLOCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EWOULDBLOCK"), EWOULDBLOCK);
#endif
#ifdef ENOMSG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMSG"), ENOMSG);
#endif
#ifdef EIDRM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EIDRM"), EIDRM);
#endif
#ifdef ECHRNG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECHRNG"), ECHRNG);
#endif
#ifdef EL2NSYNC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL2NSYNC"), EL2NSYNC);
#endif
#ifdef EL3HLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL3HLT"), EL3HLT);
#endif
#ifdef EL3RST
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL3RST"), EL3RST);
#endif
#ifdef ELNRNG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELNRNG"), ELNRNG);
#endif
#ifdef EUNATCH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUNATCH"), EUNATCH);
#endif
#ifdef ENOCSI
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOCSI"), ENOCSI);
#endif
#ifdef EL2HLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EL2HLT"), EL2HLT);
#endif
#ifdef EBADE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADE"), EBADE);
#endif
#ifdef EBADR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADR"), EBADR);
#endif
#ifdef EXFULL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EXFULL"), EXFULL);
#endif
#ifdef ENOANO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOANO"), ENOANO);
#endif
#ifdef EBADRQC
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADRQC"), EBADRQC);
#endif
#ifdef EBADSLT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADSLT"), EBADSLT);
#endif
#ifdef ENOSTR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSTR"), ENOSTR);
#endif
#ifdef ENODATA
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENODATA"), ENODATA);
#endif
#ifdef ETIME
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIME"), ETIME);
#endif
#ifdef ENOSR
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOSR"), ENOSR);
#endif
#ifdef ENONET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENONET"), ENONET);
#endif
#ifdef EREMOTE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTE"), EREMOTE);
#endif
#ifdef ENOLINK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOLINK"), ENOLINK);
#endif
#ifdef EADV
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADV"), EADV);
#endif
#ifdef ESRMNT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESRMNT"), ESRMNT);
#endif
#ifdef ECOMM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECOMM"), ECOMM);
#endif
#ifdef EPROTO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTO"), EPROTO);
#endif
#ifdef EMULTIHOP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMULTIHOP"), EMULTIHOP);
#endif
#ifdef EBADMSG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADMSG"), EBADMSG);
#endif
#ifdef ENOTUNIQ
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTUNIQ"), ENOTUNIQ);
#endif
#ifdef EBADFD
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADFD"), EBADFD);
#endif
#ifdef EREMCHG
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMCHG"), EREMCHG);
#endif
#ifdef ERESTART
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ERESTART"), ERESTART);
#endif
#ifdef ESTRPIPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESTRPIPE"), ESTRPIPE);
#endif
#ifdef EUSERS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUSERS"), EUSERS);
#endif
#ifdef ENOTSOCK
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTSOCK"), ENOTSOCK);
#endif
#ifdef EDESTADDRREQ
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDESTADDRREQ"), EDESTADDRREQ);
#endif
#ifdef EMSGSIZE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMSGSIZE"), EMSGSIZE);
#endif
#ifdef EPROTOTYPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTOTYPE"), EPROTOTYPE);
#endif
#ifdef ENOPROTOOPT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOPROTOOPT"), ENOPROTOOPT);
#endif
#ifdef EPROTONOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTONOSUPPORT"), EPROTONOSUPPORT);
#endif
#ifdef ESOCKTNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESOCKTNOSUPPORT"), ESOCKTNOSUPPORT);
#endif
#ifdef EOPNOTSUPP
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EOPNOTSUPP"), EOPNOTSUPP);
#endif
#ifdef EPFNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPFNOSUPPORT"), EPFNOSUPPORT);
#endif
#ifdef EAFNOSUPPORT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAFNOSUPPORT"), EAFNOSUPPORT);
#endif
#ifdef EADDRINUSE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRINUSE"), EADDRINUSE);
#endif
#ifdef EADDRNOTAVAIL
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRNOTAVAIL"), EADDRNOTAVAIL);
#endif
#ifdef ENETDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETDOWN"), ENETDOWN);
#endif
#ifdef ENETUNREACH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETUNREACH"), ENETUNREACH);
#endif
#ifdef ENETRESET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETRESET"), ENETRESET);
#endif
#ifdef ECONNABORTED
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNABORTED"), ECONNABORTED);
#endif
#ifdef ECONNRESET
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNRESET"), ECONNRESET);
#endif
#ifdef ENOBUFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOBUFS"), ENOBUFS);
#endif
#ifdef EISCONN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISCONN"), EISCONN);
#endif
#ifdef ENOTCONN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTCONN"), ENOTCONN);
#endif
#ifdef ESHUTDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESHUTDOWN"), ESHUTDOWN);
#endif
#ifdef ETOOMANYREFS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETOOMANYREFS"), ETOOMANYREFS);
#endif
#ifdef ETIMEDOUT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIMEDOUT"), ETIMEDOUT);
#endif
#ifdef ECONNREFUSED
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNREFUSED"), ECONNREFUSED);
#endif
#ifdef EHOSTDOWN
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTDOWN"), EHOSTDOWN);
#endif
#ifdef EHOSTUNREACH
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTUNREACH"), EHOSTUNREACH);
#endif
#ifdef EALREADY
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EALREADY"), EALREADY);
#endif
#ifdef EINPROGRESS
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINPROGRESS"), EINPROGRESS);
#endif
#ifdef EISNAM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISNAM"), EISNAM);
#endif
#ifdef EREMOTEIO
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTEIO"), EREMOTEIO);
#endif
#ifdef EDQUOT
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDQUOT"), EDQUOT);
#endif
#ifdef ENOMEDIUM
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOMEDIUM"), ENOMEDIUM);
#endif
#ifdef EMEDIUMTYPE
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMEDIUMTYPE"), EMEDIUMTYPE);
#endif
#else
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINTR"), WSAEINTR);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EBADF"), WSAEBADF);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EACCES"), WSAEACCES);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EFAULT"), WSAEFAULT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINVAL"), WSAEINVAL);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMFILE"), WSAEMFILE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EWOULDBLOCK"), WSAEWOULDBLOCK);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EINPROGRESS"), WSAEINPROGRESS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EALREADY"), WSAEALREADY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTSOCK"), WSAENOTSOCK);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDESTADDRREQ"), WSAEDESTADDRREQ);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EMSGSIZE"), WSAEMSGSIZE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTOTYPE"), WSAEPROTOTYPE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOPROTOOPT"), WSAENOPROTOOPT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROTONOSUPPORT"), WSAEPROTONOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESOCKTNOSUPPORT"), WSAESOCKTNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EOPNOTSUPP"), WSAEOPNOTSUPP);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPFNOSUPPORT"), WSAEPFNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EAFNOSUPPORT"), WSAEAFNOSUPPORT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRINUSE"), WSAEADDRINUSE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EADDRNOTAVAIL"), WSAEADDRNOTAVAIL);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETDOWN"), WSAENETDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETUNREACH"), WSAENETUNREACH);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENETRESET"), WSAENETRESET);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNABORTED"), WSAECONNABORTED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNRESET"), WSAECONNRESET);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOBUFS"), WSAENOBUFS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EISCONN"), WSAEISCONN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTCONN"), WSAENOTCONN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESHUTDOWN"), WSAESHUTDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETOOMANYREFS"), WSAETOOMANYREFS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ETIMEDOUT"), WSAETIMEDOUT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ECONNREFUSED"), WSAECONNREFUSED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ELOOP"), WSAELOOP);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENAMETOOLONG"), WSAENAMETOOLONG);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTDOWN"), WSAEHOSTDOWN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EHOSTUNREACH"), WSAEHOSTUNREACH);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ENOTEMPTY"), WSAENOTEMPTY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EPROCLIM"), WSAEPROCLIM);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EUSERS"), WSAEUSERS);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDQUOT"), WSAEDQUOT);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("ESTALE"), WSAESTALE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EREMOTE"), WSAEREMOTE);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("EDISCON"), WSAEDISCON);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("SYSNOTREADY"), WSASYSNOTREADY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("VERNOTSUPPORTED"), WSAVERNOTSUPPORTED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NOTINITIALISED"), WSANOTINITIALISED);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("HOST_NOT_FOUND"), WSAHOST_NOT_FOUND);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("TRY_AGAIN"), WSATRY_AGAIN);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_RECOVERY"), WSANO_RECOVERY);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_DATA"), WSANO_DATA);
	zend_declare_class_constant_long(pthreads_socket_entry,  ZEND_STRL("NO_ADDRESS"), WSANO_ADDRESS);
#endif

	/*
	* Setup object handlers
	*/
	zend_handlers = (zend_object_handlers*)zend_get_std_object_handlers();
	
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

	memcpy(&pthreads_socket_handlers, &pthreads_handlers, sizeof(zend_object_handlers));

	pthreads_socket_handlers.count_elements = pthreads_count_properties_disallow;

	pthreads_socket_handlers.cast_object = zend_handlers->cast_object;
	pthreads_socket_handlers.get_properties = pthreads_read_properties_disallow;
	pthreads_socket_handlers.read_property = pthreads_read_property_disallow;
	pthreads_socket_handlers.write_property = pthreads_write_property_disallow;
	pthreads_socket_handlers.has_property = pthreads_has_property_disallow;
	pthreads_socket_handlers.unset_property = pthreads_unset_property_disallow;
	pthreads_socket_handlers.read_dimension = pthreads_read_dimension_disallow;
	pthreads_socket_handlers.write_dimension = pthreads_write_dimension_disallow;
	pthreads_socket_handlers.has_dimension = pthreads_has_dimension_disallow;
	pthreads_socket_handlers.unset_dimension = pthreads_unset_dimension_disallow;

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

	zend_set_user_opcode_handler(ZEND_RECV, php_pthreads_recv);
	zend_set_user_opcode_handler(ZEND_VERIFY_RETURN_TYPE, php_pthreads_verify_return_type);

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
	void *function;

	zend_hash_init(&PTHREADS_ZG(resolve), 15, NULL, NULL, 0);
	zend_hash_init(&PTHREADS_ZG(filenames), 15, NULL, NULL, 0);

	PTHREADS_ZG(hard_copy_interned_strings) = 0;

	if (pthreads_instance != TSRMLS_CACHE) {
		if (memcmp(sapi_module.name, ZEND_STRL("cli")) == SUCCESS) {
			sapi_module.deactivate = NULL;
		}
	}

	function = zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("putenv"));

	if (function) {
		zend_function *pthreads_putenv;

		memcpy(&zend_putenv_function, function, sizeof(zend_internal_function));

		function_add_ref((zend_function*) &zend_putenv_function);

		memcpy(function, zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("pthreads_putenv")), sizeof(zend_internal_function));

		function_add_ref(function);
	}

	function = zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("getenv"));

	if (function) {
		zend_function *pthreads_getenv;

		memcpy(&zend_getenv_function, function, sizeof(zend_internal_function));

		function_add_ref((zend_function*) &zend_getenv_function);

		memcpy(function, zend_hash_str_find_ptr(CG(function_table), ZEND_STRL("pthreads_getenv")), sizeof(zend_internal_function));

		function_add_ref(function);
	}

	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(pthreads) {
	zend_hash_destroy(&PTHREADS_ZG(resolve));
	zend_hash_destroy(&PTHREADS_ZG(filenames));

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

#ifndef HAVE_PTHREADS_CLASS_SOCKET
#	include <classes/socket.h>
#endif

#endif
