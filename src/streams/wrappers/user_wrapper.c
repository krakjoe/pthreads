/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2018                                       |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Authors: Bastian Schneider <b.schneider@badnoob.com>                 |
  | Borrowed code from php-src                                           |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER
#define HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER

#ifndef HAVE_PTHREADS_H
#	include <src/pthreads.h>
#endif

#ifndef HAVE_PTHREADS_STREAM_H
#	include <src/streams.h>
#endif

#include "php_globals.h"
#include "ext/standard/file.h"
#include "ext/standard/flock_compat.h"
#ifdef HAVE_SYS_FILE_H
#include <sys/file.h>
#endif
#include <stddef.h>

#if HAVE_UTIME
# ifdef PHP_WIN32
#  include <sys/utime.h>
# else
#  include <utime.h>
# endif
#endif

#ifndef HAVE_PTHREADS_STREAMS_WRAPPERS_USER_WRAPPER_H
#	include <src/streams/wrappers/user_wrapper.h>
#endif

static pthreads_stream_t *pthreads_user_wrapper_opener(pthreads_stream_wrapper_t *threaded_wrapper,
		const char *filename, const char *mode, int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);
static int pthreads_user_wrapper_stat_url(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int flags, pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context);
static int pthreads_user_wrapper_unlink(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context);
static int pthreads_user_wrapper_rename(pthreads_stream_wrapper_t *threaded_wrapper, const char *url_from, const char *url_to, int options, pthreads_stream_context_t *threaded_context);
static int pthreads_user_wrapper_mkdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int mode, int options, pthreads_stream_context_t *threaded_context);
static int pthreads_user_wrapper_rmdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context);
static int pthreads_user_wrapper_metadata(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int option, void *value, pthreads_stream_context_t *threaded_context);
static pthreads_stream_t *pthreads_user_wrapper_opendir(pthreads_stream_wrapper_t *threaded_wrapper, const char *filename, const char *mode,
		int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce);

const pthreads_stream_wrapper_ops pthreads_user_stream_wops = {
	pthreads_user_wrapper_opener,
	NULL, /* close - the streams themselves know how */
	NULL, /* stat - the streams themselves know how */
	pthreads_user_wrapper_stat_url,
	pthreads_user_wrapper_opendir,
	"user-space",
	pthreads_user_wrapper_unlink,
	pthreads_user_wrapper_rename,
	pthreads_user_wrapper_mkdir,
	pthreads_user_wrapper_rmdir,
	pthreads_user_wrapper_metadata
};

/* names of methods */
#define PTHREADS_USERSTREAM_OPEN		"stream_open"
#define PTHREADS_USERSTREAM_CLOSE		"stream_close"
#define PTHREADS_USERSTREAM_READ		"stream_read"
#define PTHREADS_USERSTREAM_WRITE		"stream_write"
#define PTHREADS_USERSTREAM_FLUSH		"stream_flush"
#define PTHREADS_USERSTREAM_SEEK		"stream_seek"
#define PTHREADS_USERSTREAM_TELL		"stream_tell"
#define PTHREADS_USERSTREAM_EOF			"stream_eof"
#define PTHREADS_USERSTREAM_STAT		"stream_stat"
#define PTHREADS_USERSTREAM_STATURL		"url_stat"
#define PTHREADS_USERSTREAM_UNLINK		"unlink"
#define PTHREADS_USERSTREAM_RENAME		"rename"
#define PTHREADS_USERSTREAM_MKDIR		"mkdir"
#define PTHREADS_USERSTREAM_RMDIR		"rmdir"
#define PTHREADS_USERSTREAM_DIR_OPEN	"dir_opendir"
#define PTHREADS_USERSTREAM_DIR_READ	"dir_readdir"
#define PTHREADS_USERSTREAM_DIR_REWIND	"dir_rewinddir"
#define PTHREADS_USERSTREAM_DIR_CLOSE	"dir_closedir"
#define PTHREADS_USERSTREAM_LOCK     	"stream_lock"
#define PTHREADS_USERSTREAM_CAST		"stream_cast"
#define PTHREADS_USERSTREAM_SET_OPTION	"stream_set_option"
#define PTHREADS_USERSTREAM_TRUNCATE	"stream_truncate"
#define PTHREADS_USERSTREAM_METADATA	"stream_metadata"

/* {{{ class should have methods like these:

	function stream_open($path, $mode, $options, &$opened_path)
	{
	  	return true/false;
	}

	function stream_read($count)
	{
	   	return false on error;
		else return string;
	}

	function stream_write($data)
	{
	   	return false on error;
		else return count written;
	}

	function stream_close()
	{
	}

	function stream_flush()
	{
		return true/false;
	}

	function stream_seek($offset, $whence)
	{
		return true/false;
	}

	function stream_tell()
	{
		return (int)$position;
	}

	function stream_eof()
	{
		return true/false;
	}

	function stream_stat()
	{
		return array( just like that returned by fstat() );
	}

	function stream_cast($castas)
	{
		if ($castas == STREAM_CAST_FOR_SELECT) {
			return $this->underlying_stream;
		}
		return false;
	}

	function stream_set_option($option, $arg1, $arg2)
	{
		switch($option) {
		case STREAM_OPTION_BLOCKING:
			$blocking = $arg1;
			...
		case STREAM_OPTION_READ_TIMEOUT:
			$sec = $arg1;
			$usec = $arg2;
			...
		case STREAM_OPTION_WRITE_BUFFER:
			$mode = $arg1;
			$size = $arg2;
			...
		default:
			return false;
		}
	}

	function url_stat(string $url, int $flags)
	{
		return array( just like that returned by stat() );
	}

	function unlink(string $url)
	{
		return true / false;
	}

	function rename(string $from, string $to)
	{
		return true / false;
	}

	function mkdir($dir, $mode, $options)
	{
		return true / false;
	}

	function rmdir($dir, $options)
	{
		return true / false;
	}

	function dir_opendir(string $url, int $options)
	{
		return true / false;
	}

	function dir_readdir()
	{
		return string next filename in dir ;
	}

	function dir_closedir()
	{
		release dir related resources;
	}

	function dir_rewinddir()
	{
		reset to start of dir list;
	}

	function stream_lock($operation)
	{
		return true / false;
	}

 	function stream_truncate($new_size)
	{
		return true / false;
	}

	}}} **/

static void pthreads_user_stream_create_object(struct pthreads_user_stream_wrapper *uwrap, pthreads_stream_context_t *threaded_context, zval *object)
{
	zend_class_entry *ce = zend_lookup_class(uwrap->classname);

	if(ce == NULL) {
		php_error_docref(NULL, E_WARNING, "class '%s' is undefined", ZSTR_VAL(uwrap->classname));
		return;
	}

	if (ce->ce_flags & (ZEND_ACC_INTERFACE|ZEND_ACC_TRAIT|ZEND_ACC_IMPLICIT_ABSTRACT_CLASS|ZEND_ACC_EXPLICIT_ABSTRACT_CLASS)) {
		ZVAL_UNDEF(object);
		return;
	}

	/* create an instance of our class */
	object_init_ex(object, ce);

	if (threaded_context) {
		zval zcontext;
		ZVAL_OBJ(&zcontext, PTHREADS_STD_P(threaded_context));
		add_property_zval(object, "context", &zcontext);
	} else {
		add_property_null(object, "context");
	}

	if (ce->constructor) {
		zend_fcall_info fci;
		zend_fcall_info_cache fcc;
		zval retval;

		fci.size = sizeof(fci);
		ZVAL_UNDEF(&fci.function_name);
		fci.object = Z_OBJ_P(object);
		fci.retval = &retval;
		fci.param_count = 0;
		fci.params = NULL;
		fci.no_separation = 1;

		fcc.function_handler = ce->constructor;
		fcc.called_scope = Z_OBJCE_P(object);
		fcc.object = Z_OBJ_P(object);

		if (zend_call_function(&fci, &fcc) == FAILURE) {
			php_error_docref(NULL, E_WARNING, "Could not execute %s::%s()", ZSTR_VAL(ce->name), ZSTR_VAL(ce->constructor->common.function_name));
			zval_ptr_dtor(object);
			ZVAL_UNDEF(object);
		} else {
			zval_ptr_dtor(&retval);
		}
	}
}

static pthreads_stream_t *pthreads_user_wrapper_opener(pthreads_stream_wrapper_t *threaded_wrapper, const char *filename, const char *mode,
									   int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	pthreads_stream_wrapper *user_wrapper = NULL;
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	pthreads_userstream_data_t *us;
	zval zretval, zfuncname;
	zval args[4];
	int call_result;
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;
	zend_bool old_in_user_include;

	/* Try to catch bad usage without preventing flexibility */
	if (FG(user_stream_current_filename) != NULL && strcmp(filename, FG(user_stream_current_filename)) == 0) {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "infinite recursion prevented");
		return NULL;
	}
	FG(user_stream_current_filename) = filename;

	/* if the user stream was registered as local and we are in include context,
		we add allow_url_include restrictions to allow_url_fopen ones */
	/* we need only is_url == 0 here since if is_url == 1 and remote wrappers
		were restricted we wouldn't get here */
	old_in_user_include = PG(in_user_include);

	user_wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(uwrap->threaded_wrapper);
	if(user_wrapper->is_url == 0 &&
		(options & PTHREADS_STREAM_OPEN_FOR_INCLUDE) &&
		!PG(allow_url_include)) {
		PG(in_user_include) = 1;
	}

	us = malloc(sizeof(*us));
	us->wrapper = uwrap;

	pthreads_user_stream_create_object(uwrap, threaded_context, &us->object);

	if (Z_TYPE(us->object) == IS_UNDEF) {
		FG(user_stream_current_filename) = NULL;
		PG(in_user_include) = old_in_user_include;
		free(us);
		return NULL;
	}

	/* call it's stream_open method - set up params first */
	ZVAL_STRING(&args[0], filename);
	ZVAL_STRING(&args[1], mode);
	ZVAL_LONG(&args[2], options);
	ZVAL_NEW_REF(&args[3], &EG(uninitialized_zval));

	ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_OPEN);

	zend_try {
		call_result = call_user_function_ex(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&zfuncname,
				&zretval,
				4, args,
				0, NULL	);
	} zend_catch {
		FG(user_stream_current_filename) = NULL;
		zend_bailout();
	} zend_end_try();

	if (call_result == SUCCESS && Z_TYPE(zretval) != IS_UNDEF && zval_is_true(&zretval)) {
		/* the stream is now open! */
		threaded_stream = PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_userspace_ops, us, mode, ce);

		/* if the opened path is set, copy it out */
		if (Z_ISREF(args[3]) && Z_TYPE_P(Z_REFVAL(args[3])) == IS_STRING && opened_path) {
			*opened_path = zend_string_copy(Z_STR_P(Z_REFVAL(args[3])));
		}
		/* set wrapper data to be a reference to our object */
		pthreads_stream_set_wrapperdata(threaded_stream, PTHREADS_FETCH_FROM(Z_OBJ(us->object)));
	} else {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "\"%s::" PTHREADS_USERSTREAM_OPEN "\" call failed",
			ZSTR_VAL(us->wrapper->classname));
	}

	/* destroy everything else */
	if (threaded_stream == NULL) {
		zval_ptr_dtor(&us->object);
		ZVAL_UNDEF(&us->object);
		free(us);
	}
	zval_ptr_dtor(&zretval);
	zval_ptr_dtor(&zfuncname);
	zval_ptr_dtor(&args[3]);
	zval_ptr_dtor(&args[2]);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&args[0]);

	FG(user_stream_current_filename) = NULL;

	PG(in_user_include) = old_in_user_include;
	return threaded_stream;
}

static pthreads_stream_t *pthreads_user_wrapper_opendir(pthreads_stream_wrapper_t *threaded_wrapper, const char *filename, const char *mode,
		int options, zend_string **opened_path, pthreads_stream_context_t *threaded_context, zend_class_entry *ce) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	pthreads_userstream_data_t *us;
	zval zretval, zfuncname;
	zval args[2];
	int call_result;
	pthreads_stream_t *threaded_stream = NULL;
	pthreads_stream *stream = NULL;

	/* Try to catch bad usage without preventing flexibility */
	if (FG(user_stream_current_filename) != NULL && strcmp(filename, FG(user_stream_current_filename)) == 0) {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "infinite recursion prevented");
		return NULL;
	}
	FG(user_stream_current_filename) = filename;

	us = malloc(sizeof(*us));
	us->wrapper = uwrap;

	pthreads_user_stream_create_object(uwrap, threaded_context, &us->object);
	if (Z_TYPE(us->object) == IS_UNDEF) {
		FG(user_stream_current_filename) = NULL;
		free(us);
		return NULL;
	}

	/* call it's dir_open method - set up params first */
	ZVAL_STRING(&args[0], filename);
	ZVAL_LONG(&args[1], options);

	ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_DIR_OPEN);

	call_result = call_user_function_ex(NULL,
			Z_ISUNDEF(us->object)? NULL : &us->object,
			&zfuncname,
			&zretval,
			2, args,
			0, NULL	);

	if (call_result == SUCCESS && Z_TYPE(zretval) != IS_UNDEF && zval_is_true(&zretval)) {
		/* the stream is now open! */
		threaded_stream = PTHREADS_STREAM_CLASS_NEW(&pthreads_stream_userspace_dir_ops, us, mode, ce);

		/* set wrapper data to be a reference to our object */
		pthreads_stream_set_wrapperdata(threaded_stream, PTHREADS_FETCH_FROM(Z_OBJ(us->object)));
	} else {
		pthreads_stream_wrapper_log_error(threaded_wrapper, options, "\"%s::" PTHREADS_USERSTREAM_DIR_OPEN "\" call failed",
			ZSTR_VAL(us->wrapper->classname));
	}

	/* destroy everything else */
	if (threaded_stream == NULL) {
		zval_ptr_dtor(&us->object);
		ZVAL_UNDEF(&us->object);
		free(us);
	}
	zval_ptr_dtor(&zretval);

	zval_ptr_dtor(&zfuncname);
	zval_ptr_dtor(&args[1]);
	zval_ptr_dtor(&args[0]);

	FG(user_stream_current_filename) = NULL;

	return threaded_stream;
}

static size_t pthreads_userstreamop_write(pthreads_stream_t *threaded_stream, const char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result;
	pthreads_userstream_data_t *us;
	zval args[1];
	size_t didwrite = 0;

	if(stream_lock(threaded_stream)) {
		us = (pthreads_userstream_data_t *)stream->abstract;

		assert(us != NULL);

		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_WRITE, sizeof(PTHREADS_USERSTREAM_WRITE)-1);

		ZVAL_STRINGL(&args[0], (char*)buf, count);

		call_result = call_user_function_ex(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				1, args,
				0, NULL);

		zval_ptr_dtor(&args[0]);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}

	didwrite = 0;

	if (EG(exception)) {
		return 0;
	}

	if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
		convert_to_long(&retval);
		didwrite = Z_LVAL(retval);
	} else if (call_result == FAILURE) {
		php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_WRITE " is not implemented!",
				ZSTR_VAL(us->wrapper->classname));
	}

	/* don't allow strange buffer overruns due to bogus return */
	if (didwrite > count) {
		php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_WRITE " wrote " ZEND_LONG_FMT " bytes more data than requested (" ZEND_LONG_FMT " written, " ZEND_LONG_FMT " max)",
				ZSTR_VAL(us->wrapper->classname),
				(zend_long)(didwrite - count), (zend_long)didwrite, (zend_long)count);
		didwrite = count;
	}

	zval_ptr_dtor(&retval);

	return didwrite;
}

static size_t pthreads_userstreamop_read(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	zval args[1];
	int call_result;
	size_t didread = 0;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	if(stream_lock(threaded_stream)) {

		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_READ, sizeof(PTHREADS_USERSTREAM_READ)-1);

		ZVAL_LONG(&args[0], count);

		call_result = call_user_function_ex(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				1, args,
				0, NULL);

		zval_ptr_dtor(&args[0]);
		zval_ptr_dtor(&func_name);

		if (EG(exception)) {
			stream_unlock(threaded_stream);
			return -1;
		}

		if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
			convert_to_string(&retval);
			didread = Z_STRLEN(retval);
			if (didread > count) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_READ " - read " ZEND_LONG_FMT " bytes more data than requested (" ZEND_LONG_FMT " read, " ZEND_LONG_FMT " max) - excess data will be lost",
						ZSTR_VAL(us->wrapper->classname), (zend_long)(didread - count), (zend_long)didread, (zend_long)count);
				didread = count;
			}
			if (didread > 0)
				memcpy(buf, Z_STRVAL(retval), didread);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_READ " is not implemented!", ZSTR_VAL(us->wrapper->classname));
		}

		zval_ptr_dtor(&retval);
		ZVAL_UNDEF(&retval);

		/* since the user stream has no way of setting the eof flag directly, we need to ask it if we hit eof */

		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_EOF, sizeof(PTHREADS_USERSTREAM_EOF)-1);

		call_result = call_user_function(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				0, NULL);

		if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF && zval_is_true(&retval)) {
			stream->eof = 1;
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING,
					"%s::" PTHREADS_USERSTREAM_EOF " is not implemented! Assuming EOF", ZSTR_VAL(us->wrapper->classname));

			stream->eof = 1;
		}

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return didread;
}

static int pthreads_userstreamop_close(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_CLOSE, sizeof(PTHREADS_USERSTREAM_CLOSE)-1);

	call_user_function(NULL,
			Z_ISUNDEF(us->object)? NULL : &us->object,
			&func_name,
			&retval,
			0, NULL);

	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&func_name);

	return 0;
}

static void pthreads_userstreamop_free(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	zval_ptr_dtor(&us->object);
	ZVAL_UNDEF(&us->object);

	free(us);
}

static int pthreads_userstreamop_flush(pthreads_stream_t *threaded_stream) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_FLUSH, sizeof(PTHREADS_USERSTREAM_FLUSH)-1);

		call_result = call_user_function(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				0, NULL);

		if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF && zval_is_true(&retval))
			call_result = 0;
		else
			call_result = -1;

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return call_result;
}

static int pthreads_userstreamop_seek(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffs) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result, ret;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;
	zval args[2];

	assert(us != NULL);

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_SEEK, sizeof(PTHREADS_USERSTREAM_SEEK)-1);

		ZVAL_LONG(&args[0], offset);
		ZVAL_LONG(&args[1], whence);

		call_result = call_user_function_ex(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				2, args,
				0, NULL);

		zval_ptr_dtor(&args[0]);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&func_name);

		if (call_result == FAILURE) {
			/* stream_seek is not implemented, so disable seeks for this stream */
			stream->flags |= PTHREADS_STREAM_FLAG_NO_SEEK;
			/* there should be no retval to clean up */

			zval_ptr_dtor(&retval);
			stream_unlock(threaded_stream);

			return -1;
		} else if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF && zval_is_true(&retval)) {
			ret = 0;
		} else {
			ret = -1;
		}

		zval_ptr_dtor(&retval);
		ZVAL_UNDEF(&retval);

		if (ret) {
			stream_unlock(threaded_stream);
			return ret;
		}

		/* now determine where we are */
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_TELL, sizeof(PTHREADS_USERSTREAM_TELL)-1);

		call_result = call_user_function(NULL,
			Z_ISUNDEF(us->object)? NULL : &us->object,
			&func_name,
			&retval,
			0, NULL);

		if (call_result == SUCCESS && Z_TYPE(retval) == IS_LONG) {
			*newoffs = Z_LVAL(retval);
			ret = 0;
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_TELL " is not implemented!", ZSTR_VAL(us->wrapper->classname));
			ret = -1;
		} else {
			ret = -1;
		}

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return ret;
}

/* parse the return value from one of the stat functions and store the
 * relevant fields into the statbuf provided */
static int statbuf_from_array(zval *array, pthreads_stream_statbuf *ssb)
{
	zval *elem;

#define STAT_PROP_ENTRY_EX(name, name2)                        \
	if (NULL != (elem = zend_hash_str_find(Z_ARRVAL_P(array), #name, sizeof(#name)-1))) {     \
		ssb->sb.st_##name2 = zval_get_long(elem);                                                      \
	}

#define STAT_PROP_ENTRY(name) STAT_PROP_ENTRY_EX(name,name)

	memset(ssb, 0, sizeof(pthreads_stream_statbuf));
	STAT_PROP_ENTRY(dev);
	STAT_PROP_ENTRY(ino);
	STAT_PROP_ENTRY(mode);
	STAT_PROP_ENTRY(nlink);
	STAT_PROP_ENTRY(uid);
	STAT_PROP_ENTRY(gid);
#if HAVE_STRUCT_STAT_ST_RDEV
	STAT_PROP_ENTRY(rdev);
#endif
	STAT_PROP_ENTRY(size);
	STAT_PROP_ENTRY(atime);
	STAT_PROP_ENTRY(mtime);
	STAT_PROP_ENTRY(ctime);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
	STAT_PROP_ENTRY(blksize);
#endif
#ifdef HAVE_ST_BLOCKS
	STAT_PROP_ENTRY(blocks);
#endif

#undef STAT_PROP_ENTRY
#undef STAT_PROP_ENTRY_EX
	return SUCCESS;
}

static int pthreads_userstreamop_stat(pthreads_stream_t *threaded_stream, pthreads_stream_statbuf *ssb) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;
	int ret = -1;

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_STAT, sizeof(PTHREADS_USERSTREAM_STAT)-1);

		call_result = call_user_function(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				0, NULL);

		if (call_result == SUCCESS && Z_TYPE(retval) == IS_ARRAY) {
			if (SUCCESS == statbuf_from_array(&retval, ssb))
				ret = 0;
		} else {
			if (call_result == FAILURE) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_STAT " is not implemented!",
						ZSTR_VAL(us->wrapper->classname));
			}
		}

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return ret;
}


static int pthreads_userstreamop_set_option(pthreads_stream_t *threaded_stream, int option, int value, void *ptrparam) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;
	int ret = PTHREADS_STREAM_OPTION_RETURN_NOTIMPL;
	zval args[3];

	if(stream_lock(threaded_stream)) {
		switch (option) {
			case PTHREADS_STREAM_OPTION_CHECK_LIVENESS:
				ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_EOF, sizeof(PTHREADS_USERSTREAM_EOF)-1);
				call_result = call_user_function(NULL, Z_ISUNDEF(us->object)? NULL : &us->object, &func_name, &retval, 0, NULL);
				if (call_result == SUCCESS && (Z_TYPE(retval) == IS_FALSE || Z_TYPE(retval) == IS_TRUE)) {
					ret = zval_is_true(&retval) ? PTHREADS_STREAM_OPTION_RETURN_ERR : PTHREADS_STREAM_OPTION_RETURN_OK;
				} else {
					ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
					php_error_docref(NULL, E_WARNING,
							"%s::" PTHREADS_USERSTREAM_EOF " is not implemented! Assuming EOF",
							ZSTR_VAL(us->wrapper->classname));
				}
				zval_ptr_dtor(&retval);
				zval_ptr_dtor(&func_name);
				break;

			case PTHREADS_STREAM_OPTION_LOCKING:
				ZVAL_LONG(&args[0], 0);

				if (value & LOCK_NB) {
					Z_LVAL_P(&args[0]) |= PHP_LOCK_NB;
				}
				switch(value & ~LOCK_NB) {
				case LOCK_SH:
					Z_LVAL_P(&args[0]) |= PHP_LOCK_SH;
					break;
				case LOCK_EX:
					Z_LVAL_P(&args[0]) |= PHP_LOCK_EX;
					break;
				case LOCK_UN:
					Z_LVAL_P(&args[0]) |= PHP_LOCK_UN;
					break;
				}

				/* TODO wouldblock */
				ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_LOCK, sizeof(PTHREADS_USERSTREAM_LOCK)-1);

				call_result = call_user_function_ex(NULL,
								Z_ISUNDEF(us->object)? NULL : &us->object,
								&func_name,
								&retval,
								1, args, 0, NULL);

				if (call_result == SUCCESS && (Z_TYPE(retval) == IS_FALSE || Z_TYPE(retval) == IS_TRUE)) {
					ret = (Z_TYPE(retval) == IS_FALSE);
				} else if (call_result == FAILURE) {
					if (value == 0) {
						/* lock support test (TODO: more check) */
						ret = PTHREADS_STREAM_OPTION_RETURN_OK;
					} else {
						php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_LOCK " is not implemented!",
								ZSTR_VAL(us->wrapper->classname));
						ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
					}
				}

				zval_ptr_dtor(&retval);
				zval_ptr_dtor(&func_name);
				zval_ptr_dtor(&args[0]);
				break;

			case PTHREADS_STREAM_OPTION_TRUNCATE_API:
				ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_TRUNCATE, sizeof(PTHREADS_USERSTREAM_TRUNCATE)-1);

				switch (value) {
				case PTHREADS_STREAM_TRUNCATE_SUPPORTED:
					if (zend_is_callable_ex(&func_name,
							Z_ISUNDEF(us->object)? NULL : Z_OBJ(us->object),
							IS_CALLABLE_CHECK_SILENT, NULL, NULL, NULL))
						ret = PTHREADS_STREAM_OPTION_RETURN_OK;
					else
						ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
					break;

				case PTHREADS_STREAM_TRUNCATE_SET_SIZE: {
					ptrdiff_t new_size = *(ptrdiff_t*) ptrparam;
					if (new_size >= 0 && new_size <= (ptrdiff_t)LONG_MAX) {
						ZVAL_LONG(&args[0], (zend_long)new_size);
						call_result = call_user_function_ex(NULL,
										Z_ISUNDEF(us->object)? NULL : &us->object,
										&func_name,
										&retval,
										1, args, 0, NULL);
						if (call_result == SUCCESS && Z_TYPE(retval) != IS_UNDEF) {
							if (Z_TYPE(retval) == IS_FALSE || Z_TYPE(retval) == IS_TRUE) {
								ret = (Z_TYPE(retval) == IS_TRUE) ? PTHREADS_STREAM_OPTION_RETURN_OK :
										PTHREADS_STREAM_OPTION_RETURN_ERR;
							} else {
								php_error_docref(NULL, E_WARNING,
										"%s::" PTHREADS_USERSTREAM_TRUNCATE " did not return a boolean!",
										ZSTR_VAL(us->wrapper->classname));
							}
						} else {
							php_error_docref(NULL, E_WARNING,
									"%s::" PTHREADS_USERSTREAM_TRUNCATE " is not implemented!",
									ZSTR_VAL(us->wrapper->classname));
						}
						zval_ptr_dtor(&retval);
						zval_ptr_dtor(&args[0]);
					} else { /* bad new size */
						ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
					}
					break;
				}
				}
				zval_ptr_dtor(&func_name);
				break;

			case PTHREADS_STREAM_OPTION_READ_BUFFER:
			case PTHREADS_STREAM_OPTION_WRITE_BUFFER:
			case PTHREADS_STREAM_OPTION_READ_TIMEOUT:
			case PTHREADS_STREAM_OPTION_BLOCKING: {

				ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_SET_OPTION, sizeof(PTHREADS_USERSTREAM_SET_OPTION)-1);

				ZVAL_LONG(&args[0], option);
				ZVAL_NULL(&args[1]);
				ZVAL_NULL(&args[2]);

				switch(option) {
				case PTHREADS_STREAM_OPTION_READ_BUFFER:
				case PTHREADS_STREAM_OPTION_WRITE_BUFFER:
					ZVAL_LONG(&args[1], value);
					if (ptrparam) {
						ZVAL_LONG(&args[2], *(long *)ptrparam);
					} else {
						ZVAL_LONG(&args[2], BUFSIZ);
					}
					break;
				case PTHREADS_STREAM_OPTION_READ_TIMEOUT: {
					struct timeval tv = *(struct timeval*)ptrparam;
					ZVAL_LONG(&args[1], tv.tv_sec);
					ZVAL_LONG(&args[2], tv.tv_usec);
					break;
					}
				case PTHREADS_STREAM_OPTION_BLOCKING:
					ZVAL_LONG(&args[1], value);
					break;
				default:
					break;
				}

				call_result = call_user_function_ex(NULL,
					Z_ISUNDEF(us->object)? NULL : &us->object,
					&func_name,
					&retval,
					3, args, 0, NULL);

				if (call_result == FAILURE) {
					php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_SET_OPTION " is not implemented!",
							ZSTR_VAL(us->wrapper->classname));
					ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
				} else if (zend_is_true(&retval)) {
					ret = PTHREADS_STREAM_OPTION_RETURN_OK;
				} else {
					ret = PTHREADS_STREAM_OPTION_RETURN_ERR;
				}

				zval_ptr_dtor(&retval);
				zval_ptr_dtor(&args[2]);
				zval_ptr_dtor(&args[1]);
				zval_ptr_dtor(&args[0]);
				zval_ptr_dtor(&func_name);

				break;
			}
		}
		stream_unlock(threaded_stream);
	}

	return ret;
}


static int pthreads_user_wrapper_unlink(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[1];
	int call_result;
	zval object;
	int ret = 0;

	if(MONITOR_LOCK(threaded_wrapper)) {
		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			MONITOR_UNLOCK(threaded_wrapper);
			return ret;
		}

		/* call the unlink method */
		ZVAL_STRING(&args[0], url);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_UNLINK);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				1, args,
				0, NULL	);

		if (call_result == SUCCESS && (Z_TYPE(zretval) == IS_FALSE || Z_TYPE(zretval) == IS_TRUE)) {
			ret = (Z_TYPE(zretval) == IS_TRUE);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_UNLINK " is not implemented!", ZSTR_VAL(uwrap->classname));
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);
		zval_ptr_dtor(&zfuncname);

		zval_ptr_dtor(&args[0]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;
}

static int pthreads_user_wrapper_rename(pthreads_stream_wrapper_t *threaded_wrapper, const char *url_from, const char *url_to,
							   int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[2];
	int call_result;
	zval object;
	int ret = 0;

	if(MONITOR_LOCK(threaded_wrapper)) {
		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			MONITOR_UNLOCK(threaded_wrapper);
			return ret;
		}

		/* call the rename method */
		ZVAL_STRING(&args[0], url_from);
		ZVAL_STRING(&args[1], url_to);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_RENAME);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				2, args,
				0, NULL	);

		if (call_result == SUCCESS && (Z_TYPE(zretval) == IS_FALSE || Z_TYPE(zretval) == IS_TRUE)) {
			ret = (Z_TYPE(zretval) == IS_TRUE);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_RENAME " is not implemented!", ZSTR_VAL(uwrap->classname));
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);

		zval_ptr_dtor(&zfuncname);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&args[0]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;
}

static int pthreads_user_wrapper_mkdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int mode,
							  int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[3];
	int call_result;
	zval object;
	int ret = 0;

	if(MONITOR_LOCK(threaded_wrapper)) {
		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			return ret;
		}

		/* call the mkdir method */
		ZVAL_STRING(&args[0], url);
		ZVAL_LONG(&args[1], mode);
		ZVAL_LONG(&args[2], options);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_MKDIR);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				3, args,
				0, NULL	);

		if (call_result == SUCCESS && (Z_TYPE(zretval) == IS_FALSE || Z_TYPE(zretval) == IS_TRUE)) {
			ret = (Z_TYPE(zretval) == IS_TRUE);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_MKDIR " is not implemented!", ZSTR_VAL(uwrap->classname));
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);

		zval_ptr_dtor(&zfuncname);
		zval_ptr_dtor(&args[2]);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&args[0]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;
}

static int pthreads_user_wrapper_rmdir(pthreads_stream_wrapper_t *threaded_wrapper, const char *url,
							  int options, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[2];
	int call_result;
	zval object;
	int ret = 0;

	if(MONITOR_LOCK(threaded_wrapper)) {
		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			MONITOR_UNLOCK(threaded_wrapper);
			return ret;
		}

		/* call the rmdir method */
		ZVAL_STRING(&args[0], url);
		ZVAL_LONG(&args[1], options);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_RMDIR);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				2, args,
				0, NULL	);

		if (call_result == SUCCESS && (Z_TYPE(zretval) == IS_FALSE || Z_TYPE(zretval) == IS_TRUE)) {
			ret = (Z_TYPE(zretval) == IS_TRUE);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_RMDIR " is not implemented!", ZSTR_VAL(uwrap->classname));
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);

		zval_ptr_dtor(&zfuncname);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&args[0]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;
}

static int pthreads_user_wrapper_metadata(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int option,
								 void *value, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[3];
	int call_result;
	zval object;
	int ret = 0;

	if(MONITOR_LOCK(threaded_wrapper)) {
		switch(option) {
			case PTHREADS_STREAM_META_TOUCH:
				array_init(&args[2]);
				if(value) {
					struct utimbuf *newtime = (struct utimbuf *)value;
					add_index_long(&args[2], 0, newtime->modtime);
					add_index_long(&args[2], 1, newtime->actime);
				}
				break;
			case PTHREADS_STREAM_META_GROUP:
			case PTHREADS_STREAM_META_OWNER:
			case PTHREADS_STREAM_META_ACCESS:
				ZVAL_LONG(&args[2], *(long *)value);
				break;
			case PTHREADS_STREAM_META_GROUP_NAME:
			case PTHREADS_STREAM_META_OWNER_NAME:
				ZVAL_STRING(&args[2], value);
				break;
			default:
				php_error_docref(NULL, E_WARNING, "Unknown option %d for " PTHREADS_USERSTREAM_METADATA, option);
				zval_ptr_dtor(&args[2]);
				MONITOR_UNLOCK(threaded_wrapper);
				return ret;
		}

		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			zval_ptr_dtor(&args[2]);
			MONITOR_UNLOCK(threaded_wrapper);
			return ret;
		}

		/* call the mkdir method */
		ZVAL_STRING(&args[0], url);
		ZVAL_LONG(&args[1], option);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_METADATA);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				3, args,
				0, NULL	);

		if (call_result == SUCCESS && (Z_TYPE(zretval) == IS_FALSE || Z_TYPE(zretval) == IS_TRUE)) {
			ret = Z_TYPE(zretval) == IS_TRUE;
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_METADATA " is not implemented!", ZSTR_VAL(uwrap->classname));
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);

		zval_ptr_dtor(&zfuncname);
		zval_ptr_dtor(&args[0]);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&args[2]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;
}


static int pthreads_user_wrapper_stat_url(pthreads_stream_wrapper_t *threaded_wrapper, const char *url, int flags,
								 pthreads_stream_statbuf *ssb, pthreads_stream_context_t *threaded_context) {
	pthreads_stream_wrapper *wrapper = PTHREADS_FETCH_STREAMS_WRAPPER(threaded_wrapper);
	struct pthreads_user_stream_wrapper *uwrap = (struct pthreads_user_stream_wrapper*)wrapper->abstract;
	zval zfuncname, zretval;
	zval args[2];
	int call_result;
	zval object;
	int ret = -1;

	if(MONITOR_LOCK(threaded_wrapper)) {
		/* create an instance of our class */
		pthreads_user_stream_create_object(uwrap, threaded_context, &object);
		if (Z_TYPE(object) == IS_UNDEF) {
			MONITOR_UNLOCK(threaded_wrapper);
			return ret;
		}

		/* call it's stat_url method - set up params first */
		ZVAL_STRING(&args[0], url);
		ZVAL_LONG(&args[1], flags);

		ZVAL_STRING(&zfuncname, PTHREADS_USERSTREAM_STATURL);

		call_result = call_user_function_ex(NULL,
				&object,
				&zfuncname,
				&zretval,
				2, args,
				0, NULL	);

		if (call_result == SUCCESS && Z_TYPE(zretval) == IS_ARRAY) {
			/* We got the info we needed */
			if (SUCCESS == statbuf_from_array(&zretval, ssb))
				ret = 0;
		} else {
			if (call_result == FAILURE) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_STATURL " is not implemented!",
						ZSTR_VAL(uwrap->classname));
			}
		}

		/* clean up */
		zval_ptr_dtor(&object);
		zval_ptr_dtor(&zretval);

		zval_ptr_dtor(&zfuncname);
		zval_ptr_dtor(&args[1]);
		zval_ptr_dtor(&args[0]);

		MONITOR_UNLOCK(threaded_wrapper);
	}
	return ret;

}

static size_t pthreads_userstreamop_readdir(pthreads_stream_t *threaded_stream, char *buf, size_t count) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	int call_result;
	size_t didread = 0;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;
	pthreads_stream_dirent *ent = (pthreads_stream_dirent*)buf;

	/* avoid problems if someone mis-uses the stream */
	if (count != sizeof(pthreads_stream_dirent))
		return 0;

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_DIR_READ, sizeof(PTHREADS_USERSTREAM_DIR_READ)-1);

		call_result = call_user_function(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				0, NULL);

		if (call_result == SUCCESS && Z_TYPE(retval) != IS_FALSE && Z_TYPE(retval) != IS_TRUE) {
			convert_to_string(&retval);
			PHP_STRLCPY(ent->d_name, Z_STRVAL(retval), sizeof(ent->d_name), Z_STRLEN(retval));

			didread = sizeof(pthreads_stream_dirent);
		} else if (call_result == FAILURE) {
			php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_DIR_READ " is not implemented!",
					ZSTR_VAL(us->wrapper->classname));
		}

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return didread;
}

static int pthreads_userstreamop_closedir(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_DIR_CLOSE, sizeof(PTHREADS_USERSTREAM_DIR_CLOSE)-1);

	call_user_function(NULL,
			Z_ISUNDEF(us->object)? NULL : &us->object,
			&func_name,
			&retval,
			0, NULL);

	zval_ptr_dtor(&retval);
	zval_ptr_dtor(&func_name);

	return 0;
}

static void pthreads_userstreamop_freedir(pthreads_stream_t *threaded_stream, int close_handle) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	assert(us != NULL);

	zval_ptr_dtor(&us->object);
	ZVAL_UNDEF(&us->object);

	free(us);
}

static int pthreads_userstreamop_rewinddir(pthreads_stream_t *threaded_stream, zend_off_t offset, int whence, zend_off_t *newoffs) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	zval func_name;
	zval retval;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_DIR_REWIND, sizeof(PTHREADS_USERSTREAM_DIR_REWIND)-1);

		call_user_function(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				0, NULL);

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);

		stream_unlock(threaded_stream);
	}
	return 0;

}

static int pthreads_userstreamop_cast(pthreads_stream_t *threaded_stream, int castas, void **retptr) {
	pthreads_stream *stream = PTHREADS_FETCH_STREAMS_STREAM(threaded_stream);
	pthreads_stream_t *intstream = NULL;
	pthreads_userstream_data_t *us = (pthreads_userstream_data_t *)stream->abstract;
	zval func_name;
	zval retval;
	zval args[1];
	int call_result;
	int ret = FAILURE;

	if(stream_lock(threaded_stream)) {
		ZVAL_STRINGL(&func_name, PTHREADS_USERSTREAM_CAST, sizeof(PTHREADS_USERSTREAM_CAST)-1);

		switch(castas) {
		case PTHREADS_STREAM_AS_FD_FOR_SELECT:
			ZVAL_LONG(&args[0], PTHREADS_STREAM_AS_FD_FOR_SELECT);
			break;
		default:
			ZVAL_LONG(&args[0], PTHREADS_STREAM_AS_STDIO);
			break;
		}

		call_result = call_user_function_ex(NULL,
				Z_ISUNDEF(us->object)? NULL : &us->object,
				&func_name,
				&retval,
				1, args, 0, NULL);

		do {
			if (call_result == FAILURE) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_CAST " is not implemented!",
						ZSTR_VAL(us->wrapper->classname));
				break;
			}
			if (!zend_is_true(&retval)) {
				break;
			}
			if(!IS_PTHREADS_OBJECT(&retval)) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_CAST " must return a stream object",
						ZSTR_VAL(us->wrapper->classname));
				break;
			}
			intstream = PTHREADS_FETCH_FROM(Z_OBJ_P(&retval));

			if (intstream == threaded_stream) {
				php_error_docref(NULL, E_WARNING, "%s::" PTHREADS_USERSTREAM_CAST " must not return itself",
						ZSTR_VAL(us->wrapper->classname));
				intstream = NULL;
				break;
			}
			ret = pthreads_stream_cast(intstream, castas, retptr, 1);
		} while (0);

		zval_ptr_dtor(&retval);
		zval_ptr_dtor(&func_name);
		zval_ptr_dtor(&args[0]);

		stream_unlock(threaded_stream);
	}
	return ret;
}

const pthreads_stream_ops pthreads_stream_userspace_ops = {
	pthreads_userstreamop_write, pthreads_userstreamop_read,
	pthreads_userstreamop_close, pthreads_userstreamop_free,
	pthreads_userstreamop_flush,
	"user-space",
	pthreads_userstreamop_seek,
	pthreads_userstreamop_cast,
	pthreads_userstreamop_stat,
	pthreads_userstreamop_set_option,
};

const pthreads_stream_ops pthreads_stream_userspace_dir_ops = {
	NULL, /* write */
	pthreads_userstreamop_readdir,
	pthreads_userstreamop_closedir,
	pthreads_userstreamop_freedir,
	NULL, /* flush */
	"user-space-dir",
	pthreads_userstreamop_rewinddir,
	NULL, /* cast */
	NULL, /* stat */
	NULL  /* set_option */
};

#endif
