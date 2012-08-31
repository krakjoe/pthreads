PHP_ARG_ENABLE(pthreads, whether to enable pthreads extension, 		[  --enable-pthreads   		Enable pthreads extension])

if test "$PHP_PTHREADS" != "no"; then
  PHP_EVAL_LIBLINE("-lpthread", PTHREADS_SHARED_LIBADD)
  AC_DEFINE(HAVE_PTHREADS, 1, [Wether you have pthreads support])
  PHP_NEW_EXTENSION(pthreads, php_pthreads.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/pthreads], [php_pthreads.h])
fi
