PHP_ARG_ENABLE(pthreads, whether to enable pthreads Threading API,
[  --enable-pthreads     Enable pthreads Threading API])
if test "$PHP_PTHREADS" != "no"; then
	AC_DEFINE(HAVE_PTHREADS, 1, [Wether you have pthreads support])
	PHP_NEW_EXTENSION(pthreads, php_pthreads.c \
								src/globals.c \
								src/prepare.c \
								src/synchro.c \
								src/state.c \
								src/serial.c \
								src/modifiers.c\
								src/handlers.c \
								src/object.c, $ext_shared)
	PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
	PHP_SUBST(PTHREADS_SHARED_LIBADD)
	PHP_ADD_MAKEFILE_FRAGMENT
fi
