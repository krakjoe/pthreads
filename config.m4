PHP_ARG_ENABLE(pthreads, whether to enable pthreads Threading API,
[  --enable-pthreads     Enable pthreads Threading API])
if test "$PHP_PTHREADS" != "no"; then
	AC_DEFINE(HAVE_PTHREADS, 1, [Wether you have pthreads support])
	AC_MSG_CHECKING([checking for PHP zts])   
	if test "$PHP_THREAD_SAFETY" != "no"; then
		AC_MSG_RESULT([ok])
	else
		AC_MSG_ERROR([pthreads require ZTS, please re-compile your PHP with ZTS enabled])
	fi
	PHP_NEW_EXTENSION(pthreads, php_pthreads.c \
								src/globals.c \
								src/prepare.c \
								src/synchro.c \
								src/state.c \
								src/store.c \
								src/modifiers.c\
								src/handlers.c \
								src/object.c, $ext_shared)
	PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
	PHP_ADD_INCLUDE($ext_builddir)
	PHP_SUBST(PTHREADS_SHARED_LIBADD)
	PHP_ADD_MAKEFILE_FRAGMENT
fi
