PHP_ARG_ENABLE(pthreads, whether to enable Threading API,
[  --enable-pthreads     Enable Threading API])

if test "$PHP_PTHREADS" != "no"; then
	AC_DEFINE(HAVE_PTHREADS, 1, [Wether you have user-land threading support])
	AC_MSG_CHECKING([for ZTS])   
	if test "$PHP_THREAD_SAFETY" != "no"; then
		AC_MSG_RESULT([ok])
	else
		AC_MSG_ERROR([pthreads requires ZTS, please re-compile PHP with ZTS enabled])
	fi
	PHP_NEW_EXTENSION(pthreads, php_pthreads.c src/monitor.c src/stack.c src/globals.c src/prepare.c src/store.c src/resources.c src/handlers.c src/object.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
	PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
	PHP_ADD_INCLUDE($ext_builddir)
	PHP_SUBST(PTHREADS_SHARED_LIBADD)
	PHP_ADD_MAKEFILE_FRAGMENT
fi
