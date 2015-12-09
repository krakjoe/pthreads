PHP_ARG_ENABLE(pthreads, whether to enable pthreads,
[  --enable-pthreads          Enable pthreads])

PHP_ARG_WITH(pthreads-sanitize, whether to enable AddressSanitizer for pthreads,
[  --with-pthreads-sanitize   Enable AddressSanitizer for pthreads], no, no)

PHP_ARG_WITH(pthreads-dmalloc, wether to enable dmalloc for pthreads,
[  --with-pthreads-dmalloc   Enable dmalloc for pthreads], no, no)

if test "$PHP_PTHREADS" != "no"; then
	AC_MSG_CHECKING([for ZTS])   
	if test "$PHP_THREAD_SAFETY" != "no"; then
		AC_MSG_RESULT([ok])
	else
		AC_MSG_ERROR([pthreads requires ZTS, please re-compile PHP with ZTS enabled])
	fi

	AC_DEFINE(HAVE_PTHREADS, 1, [Wether you have pthreads support])

	if test "$PHP_PTHREADS_SANITIZE" != "no"; then
		EXTRA_LDFLAGS="-lasan"
		EXTRA_CFLAGS="-fsanitize=address -fno-omit-frame-pointer"
	fi
	
	if test "$PHP_PTHREADS_DMALLOC" != "no"; then
		EXTRA_LDFLAGS="$EXTRA_LDFLAGS -ldmalloc"
		EXTRA_CFLAGS="$EXTRA_CFLAGS -DDMALLOC"
	fi

	PHP_NEW_EXTENSION(pthreads, php_pthreads.c src/monitor.c src/stack.c src/globals.c src/prepare.c src/store.c src/resources.c src/handlers.c src/object.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
	PHP_ADD_BUILD_DIR($ext_builddir/src, 1)
	PHP_ADD_INCLUDE($ext_builddir)
	PHP_SUBST(PTHREADS_SHARED_LIBADD)
    PHP_SUBST(EXTRA_LDFLAGS)
    PHP_SUBST(EXTRA_CFLAGS)
	PHP_ADD_MAKEFILE_FRAGMENT
fi
