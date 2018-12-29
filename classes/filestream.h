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
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_CLASS_FILESTREAM_H
#define HAVE_PTHREADS_CLASS_FILESTREAM_H

// FileStream
/* {{{ */
static PHP_METHOD(FileStream, __construct) {
	zend_throw_error(NULL, "Instantiation of 'FileStream' is not allowed");
} /* }}} */
PHP_METHOD(FileStream, lock);
PHP_METHOD(FileStream, close);
PHP_METHOD(FileStream, pclose);
PHP_METHOD(FileStream, eof);
PHP_METHOD(FileStream, gets);
PHP_METHOD(FileStream, getc);
PHP_METHOD(FileStream, getss);
PHP_METHOD(FileStream, scanf);
PHP_METHOD(FileStream, write);
PHP_METHOD(FileStream, flush);
PHP_METHOD(FileStream, rewind);
PHP_METHOD(FileStream, tell);
PHP_METHOD(FileStream, seek);
PHP_METHOD(FileStream, passthru);
PHP_METHOD(FileStream, truncate);
PHP_METHOD(FileStream, stat);
PHP_METHOD(FileStream, read);
PHP_METHOD(FileStream, putcsv);
PHP_METHOD(FileStream, getcsv);

// File
/* {{{ */
static PHP_METHOD(File, __construct) {
	zend_throw_error(NULL, "Instantiation of 'File' is not allowed");
} /* }}} */
PHP_METHOD(File, open);
PHP_METHOD(File, popen);
PHP_METHOD(File, getMetaTags);
PHP_METHOD(File, getContents);
PHP_METHOD(File, putContents);
PHP_METHOD(File, file);
PHP_METHOD(File, tempName);
PHP_METHOD(File, tempFile);
PHP_METHOD(File, mkdir);
PHP_METHOD(File, rmdir);
PHP_METHOD(File, readfile);
PHP_METHOD(File, rename);
PHP_METHOD(File, unlink);
PHP_METHOD(File, copy);
PHP_METHOD(File, sockopen);

/**
 * FileStream
 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_lock, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, operation, IS_LONG, 0)
	ZEND_ARG_INFO(1, wouldblock)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_close, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_pclose, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_eof, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_gets, 0, 0, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_getc, 0, 0, IS_STRING, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_getss, 0, 0, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, allowable_tags, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(FileStream_scanf, 0, 0, 1)
	ZEND_ARG_TYPE_INFO(0, format, IS_STRING, 0)
	ZEND_ARG_VARIADIC_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_write, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, input, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, maxlen, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_flush, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_rewind, 0, 0, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_tell, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_seek, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, whence, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_passthru, 0, 0, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_truncate, 0, 0, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, size, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_stat, 0, 0, IS_ARRAY, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_read, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, length, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_putcsv, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, fields, IS_ARRAY, 0)
	ZEND_ARG_TYPE_INFO(0, delimiter, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, enclosure, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, escape_char, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(FileStream_getcsv, 0, 0, IS_ARRAY, 1)
	ZEND_ARG_INFO(0, length)
	ZEND_ARG_TYPE_INFO(0, delimiter, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, enclosure, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, escape_char, IS_STRING, 0)
ZEND_END_ARG_INFO()

/**
 * File
 */

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_getMetaTags, 0, 1, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, use_include_path, _IS_BOOL, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_getContents, 0, 1, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, use_include_path, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
	ZEND_ARG_TYPE_INFO(0, offset, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, maxlen, IS_LONG, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_putContents, 0, 2, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_INFO(0, data)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_file, 0, 1, IS_ARRAY, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, flags, IS_LONG, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_tempName, 0, 2, IS_STRING, 1)
	ZEND_ARG_TYPE_INFO(0, dir, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, prefix, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(File_tempFile, 0, 0, FileStream, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(File_open, 0, 2, FileStream, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, use_include_path, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(File_popen, 0, 2, FileStream, 1)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_STRING, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_mkdir, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, pathname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, mode, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, recursive, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_rmdir, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, pathname, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_readfile, 0, 1, IS_LONG, 0)
	ZEND_ARG_TYPE_INFO(0, pathname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, use_include_path, _IS_BOOL, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_rename, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, old_name, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, new_name, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_unlink, 0, 1, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, filename, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO_EX(File_copy, 0, 2, _IS_BOOL, 0)
	ZEND_ARG_TYPE_INFO(0, source_file, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, destination_file, IS_STRING, 0)
	ZEND_ARG_OBJ_INFO(0, context, StreamContext, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_WITH_RETURN_OBJ_INFO_EX(File_sockopen, 0, 1, FileStream, 1)
	ZEND_ARG_TYPE_INFO(0, hostname, IS_STRING, 0)
	ZEND_ARG_TYPE_INFO(0, port, IS_LONG, 0)
	ZEND_ARG_INFO(1, errno)
	ZEND_ARG_INFO(1, errstr)
	ZEND_ARG_TYPE_INFO(0, timeout, IS_DOUBLE, 0)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_streams_file_stream_methods[];
extern zend_function_entry pthreads_file_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_FILESTREAM
#	define HAVE_PTHREADS_CLASS_FILESTREAM

zend_function_entry pthreads_streams_file_stream_methods[] = {
	PHP_ME(FileStream, __construct  , NULL				    , ZEND_ACC_PRIVATE)
	PHP_ME(FileStream, lock         , FileStream_lock       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, close        , FileStream_close      , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, pclose       , FileStream_pclose     , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, eof          , FileStream_eof        , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, gets         , FileStream_gets       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, getc         , FileStream_getc       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, getss        , FileStream_getss      , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, scanf        , FileStream_scanf      , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, write        , FileStream_write      , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, flush        , FileStream_flush      , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, rewind       , FileStream_rewind     , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, tell         , FileStream_tell       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, seek         , FileStream_seek       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, passthru     , FileStream_passthru   , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, truncate     , FileStream_truncate   , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, stat         , FileStream_stat       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, read         , FileStream_read       , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, putcsv       , FileStream_putcsv     , ZEND_ACC_PUBLIC)
	PHP_ME(FileStream, getcsv       , FileStream_getcsv     , ZEND_ACC_PUBLIC)
	PHP_FE_END
};

zend_function_entry pthreads_file_methods[] = {
	PHP_ME(File, __construct    , NULL                  , ZEND_ACC_PRIVATE)
	PHP_ME(File, open           , File_open             , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, popen          , File_popen            , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, getMetaTags    , File_getMetaTags      , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, getContents    , File_getContents      , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, putContents    , File_putContents      , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, file           , File_file             , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, tempName       , File_tempName         , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, tempFile       , File_tempFile         , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, mkdir          , File_mkdir            , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, rmdir          , File_rmdir            , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, readfile       , File_readfile         , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, rename         , File_rename           , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, unlink         , File_unlink           , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, copy           , File_copy             , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_ME(File, sockopen       , File_sockopen         , ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
	PHP_FE_END
};

/**
 *
 * FileStream
 *
 */

/* {{{ proto bool FileStream::lock(int operation [, int &wouldblock])
   Portable file locking */
PHP_METHOD(FileStream, lock) {
	zval *wouldblock = NULL;
	int act;
	zend_long operation = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|z/", &operation, &wouldblock) != SUCCESS) {
		RETURN_FALSE;
	}

	act = operation & 3;
	if (act < 1 || act > 3) {
		php_error_docref(NULL, E_WARNING, "Illegal operation argument");
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_lock(getThis(), act, operation, wouldblock, return_value);

} /* }}} */

/* {{{ proto bool FileStream::close(void)
   Close an open file pointer */
PHP_METHOD(FileStream, close) {

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_close(getThis(), return_value);
} /* }}} */

/* {{{ proto bool FileStream::pclose(void)
   Close a file pointer opened by File::popen() */
PHP_METHOD(FileStream, pclose) {

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_pclose(getThis(), return_value);
} /* }}} */

/* {{{ proto bool FileStream::eof(void)
   Test for end-of-file on a file pointer */
PHP_METHOD(FileStream, eof) {

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_eof(getThis(), return_value);
} /* }}} */

/* {{{ proto string|null FileStream::gets([int length])
   Get a line from file pointer */
PHP_METHOD(FileStream, gets) {
	zend_long len = 1024;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|l", &len) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_gets(getThis(), ZEND_NUM_ARGS(), len, return_value);
} /* }}} */

/* {{{ proto string|null FileStream::getc()
   Get a character from file pointer */
PHP_METHOD(FileStream, getc) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_getc(getThis(), return_value);
} /* }}} */

/* {{{ proto string|null FileStream::fgetss([int length [, string allowable_tags]])
   Get a line from file pointer and strip HTML tags */
PHP_METHOD(FileStream, getss) {
	zend_long bytes = 1024;
	zend_string *allowed_tags;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lS", &bytes, &allowed_tags) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_getss(getThis(), ZEND_NUM_ARGS(), bytes, allowed_tags, return_value);
} /* }}} */

/* {{{ proto mixed FileStream::scanf(string format [, string ...])
   Implements a mostly ANSI compatible fscanf() */
PHP_METHOD(FileStream, scanf) {
	zval *args = NULL;
	int argc = 0;
	zend_string *format;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|*", &format, &args, &argc) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_scanf(getThis(), format, args, argc, return_value);
} /* }}} */

/* {{{ proto int FileStream::write(string str [, int length])
   Binary-safe file write */
PHP_METHOD(FileStream, write) {
	zend_long maxlen = 0;
	zend_string *input;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &input, &maxlen) != SUCCESS) {
		RETURN_LONG(-1);
	}

	pthreads_streams_api_filestream_write(getThis(), ZEND_NUM_ARGS(), input, maxlen, return_value);
} /* }}} */

/* {{{ proto bool FileStream::flush()
   Flushes output */
PHP_METHOD(FileStream, flush) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_flush(getThis(), return_value);
} /* }}} */

/* {{{ proto bool FileStream::rewind()
   Rewind the position of a file pointer */
PHP_METHOD(FileStream, rewind) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_rewind(getThis(), return_value);
} /* }}} */

/* {{{ proto int FileStream::tell()
   Get file pointer's read/write position */
PHP_METHOD(FileStream, tell) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_LONG(-1);
	}

	pthreads_streams_api_filestream_tell(getThis(), return_value);
} /* }}} */

/* {{{ proto int FileStream::seek(int offset [, int whence])
   Seek on a file pointer */
PHP_METHOD(FileStream, seek) {
	zend_long offset = 0, whence = SEEK_SET;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &offset, &whence) != SUCCESS) {
		RETURN_LONG(-1);
	}

	pthreads_streams_api_filestream_seek(getThis(), offset, whence, return_value);
} /* }}} */

/* {{{ proto int FileStream::passthru()
   Output all remaining data from a file pointer */
PHP_METHOD(FileStream, passthru) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_LONG(-1);
	}

	pthreads_streams_api_filestream_passthru(getThis(), return_value);
} /* }}} */

/* {{{ proto bool FileStream::truncate(int size)
   Truncate file to 'size' length */
PHP_METHOD(FileStream, truncate) {
	zend_long size;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &size) != SUCCESS) {
		RETURN_FALSE;
	}

	if (size < 0) {
		php_error_docref(NULL, E_WARNING, "Negative size is not supported");
		RETURN_FALSE;
	}

	pthreads_streams_api_filestream_truncate(getThis(), size, return_value);
} /* }}} */

/* {{{ proto array|null FileStream::stat()
   Stat() on a filehandle */
PHP_METHOD(FileStream, stat) {
	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_stat(getThis(), return_value);
} /* }}} */

/* {{{ proto string|null FileStream::read(int length)
   Binary-safe file read */
PHP_METHOD(FileStream, read) {
	zend_long length;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &length) != SUCCESS) {
		RETURN_NULL();
	}

	if (length <= 0) {
		php_error_docref(NULL, E_WARNING, "Length parameter must be greater than 0");
		RETURN_NULL();
	}

	pthreads_streams_api_filestream_read(getThis(), length, return_value);
} /* }}} */

/* {{{ proto int FileStream::putcsv(array fields [, string delimiter [, string enclosure [, string escape_char]]])
   Format line as CSV and write to file pointer */
PHP_METHOD(FileStream, putcsv) {

	char delimiter = ',';	 /* allow this to be set as parameter */
	char enclosure = '"';	 /* allow this to be set as parameter */
	char escape_char = '\\'; /* allow this to be set as parameter */

	zval *fields = NULL;
	zend_string *delimiter_str = NULL, *enclosure_str = NULL, *escape_str = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "A|SSS", &fields, &delimiter_str, enclosure_str, escape_str) != SUCCESS) {
		RETURN_NULL();
	}

	if (delimiter_str != NULL) {
		/* Make sure that there is at least one character in string */
		if (ZSTR_LEN(delimiter_str) < 1) {
			php_error_docref(NULL, E_WARNING, "delimiter must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(delimiter_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "delimiter must be a single character");
		}

		/* use first character from string */
		delimiter = ZSTR_VAL(delimiter_str)[0];
	}

	if (enclosure_str != NULL) {
		if (ZSTR_LEN(enclosure_str) < 1) {
			php_error_docref(NULL, E_WARNING, "enclosure must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(enclosure_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "enclosure must be a single character");
		}
		/* use first character from string */
		enclosure = ZSTR_VAL(enclosure_str)[0];
	}

	if (escape_str != NULL) {
		if (ZSTR_LEN(escape_str) < 1) {
			php_error_docref(NULL, E_WARNING, "escape must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(escape_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "escape must be a single character");
		}
		/* use first character from string */
		escape_char = ZSTR_VAL(escape_str)[0];
	}

	pthreads_streams_api_filestream_putcsv(getThis(), fields, delimiter, enclosure, escape_char, return_value);
} /* }}} */

/* {{{ proto array|null FileStream::getcsv([,int length [, string delimiter [, string enclosure [, string escape]]]])
   Format line as CSV and write to file pointer */
PHP_METHOD(FileStream, getcsv) {

	char delimiter = ',';	 /* allow this to be set as parameter */
	char enclosure = '"';	 /* allow this to be set as parameter */
	char escape_char = '\\'; /* allow this to be set as parameter */

	zend_long len = 0;
	zval *len_zv = NULL;
	zend_string *delimiter_str = NULL, *enclosure_str = NULL, *escape_str = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|zSSS", &len_zv, &delimiter_str, &enclosure_str, &escape_str) != SUCCESS) {
		RETURN_NULL();
	}

	if (delimiter_str != NULL) {
		/* Make sure that there is at least one character in string */
		if (ZSTR_LEN(delimiter_str) < 1) {
			php_error_docref(NULL, E_WARNING, "delimiter must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(delimiter_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "delimiter must be a single character");
		}

		/* use first character from string */
		delimiter = ZSTR_VAL(delimiter_str)[0];
	}

	if (enclosure_str != NULL) {
		if (ZSTR_LEN(enclosure_str) < 1) {
			php_error_docref(NULL, E_WARNING, "enclosure must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(enclosure_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "enclosure must be a single character");
		}
		/* use first character from string */
		enclosure = ZSTR_VAL(enclosure_str)[0];
	}

	if (escape_str != NULL) {
		if (ZSTR_LEN(escape_str) < 1) {
			php_error_docref(NULL, E_WARNING, "escape must be a character");
			RETURN_FALSE;
		} else if (ZSTR_LEN(escape_str) > 1) {
			php_error_docref(NULL, E_NOTICE, "escape must be a single character");
		}
		/* use first character from string */
		escape_char = ZSTR_VAL(escape_str)[0];
	}

	if (len_zv != NULL && Z_TYPE_P(len_zv) != IS_NULL) {
		len = zval_get_long(len_zv);
		if (len < 0) {
			php_error_docref(NULL, E_WARNING, "Length parameter may not be negative");
			RETURN_FALSE;
		} else if (len == 0) {
			len = -1;
		}
	} else {
		len = -1;
	}

	pthreads_streams_api_filestream_getcsv(getThis(), len, delimiter, enclosure, escape_char, return_value);
}


/**
 *
 * File
 *
 */

/* {{{ proto array|null File::getMetaTags(string filename [, bool use_include_path])
   Extracts all meta tag content attributes from a file and returns an array */
PHP_METHOD(File, getMetaTags) {
	zend_string *filename;
	zend_bool use_include_path = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|b", &filename, &use_include_path) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_file_get_meta_data(filename, use_include_path, return_value);
} /* }}} */

/* {{{ proto string|null File::getContents(string filename [, bool use_include_path [, StreamContext context [, int offset [, int maxlen]]]])
   Read the entire file into a string */
PHP_METHOD(File, getContents) {
	zend_string *filename;
	zend_bool use_include_path = 0;
	zend_long offset = 0;
	zend_long maxlen = (ssize_t) PTHREADS_STREAM_COPY_ALL;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|bOll", &filename, &use_include_path, &zcontext, pthreads_stream_context_entry, &offset, &maxlen) != SUCCESS) {
		RETURN_NULL();
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_NULL();
	}

	if (ZEND_NUM_ARGS() == 5 && maxlen < 0) {
		php_error_docref(NULL, E_WARNING, "length must be greater than or equal to zero");
		RETURN_NULL();
	}

	pthreads_streams_api_file_get_contents(filename, use_include_path, zcontext, offset, maxlen, return_value);
} /* }}} */

/* {{{ proto int|null File::putContents(string file, mixed data [, int flags [, StreamContext context]])
   Write/Create a file with contents data and return the number of bytes written */
PHP_METHOD(File, putContents) {
	zend_string *filename;
	zval *data;
	zend_long flags = 0;
	zend_long maxlen = (ssize_t) PTHREADS_STREAM_COPY_ALL;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sz|lO", &filename, &data, &flags, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_LONG(0);
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_LONG(0);
	}

	pthreads_streams_api_file_put_contents(ZSTR_VAL(filename), ZSTR_LEN(filename), data, flags, zcontext, return_value);
} /* }}} */

/* {{{ proto array|null File::file(string filename [, int flags[, resource context]])
   Read entire file into an array */
PHP_METHOD(File, file) {
	zend_string *filename;
	zend_long flags = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lO", &filename, &flags, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_NULL();
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_NULL();
	}

	pthreads_streams_api_file_file(ZSTR_VAL(filename), flags, zcontext, return_value);
} /* }}} */

/* {{{ proto string|null File::tempName(string dir, string prefix)
   Create a unique filename in a directory */
PHP_METHOD(File, tempName) {
	char *dir, *prefix;
	size_t dir_len, prefix_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "pp", &dir, &dir_len, &prefix, &prefix_len) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_file_temp_name(dir, prefix, prefix_len, return_value);
} /* }}} */

/* {{{ proto FileStream|null File::tempFile(void)
   Create a temporary file that will be deleted automatically after use */
PHP_METHOD(File, tempFile) {

	if (zend_parse_parameters_none() == FAILURE) {
		RETURN_NULL();
	}

	pthreads_streams_api_file_temp_file(return_value);
} /* }}} */

/* {{{ proto static FileStream|null File::open(string filename, string mode [, bool use_include_path [, resource context]])
   Open a file or a URL and return a file pointer */
PHP_METHOD(File, open) {
	zend_string *filename, *mode;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS|bO", &filename, &mode, &use_include_path, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_NULL();
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_NULL();
	}

	pthreads_streams_api_file_open(filename, mode, use_include_path, zcontext, return_value);
} /* }}} */

/* {{{ proto static FileStream|null File::open(string filename, string mode)
   Execute a command and open either a read or a write pipe to it */
PHP_METHOD(File, popen) {
	zend_string *filename, *mode;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS", &filename, &mode) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_file_popen(ZSTR_VAL(filename), ZSTR_VAL(mode), ZSTR_LEN(mode), return_value);
} /* }}} */

/* {{{ proto bool File::mkdir(string pathname [, int mode [, bool recursive [, resource context]]])
   Create a directory */
PHP_METHOD(File, mkdir) {
	zend_string *pathname;
	zend_long mode = 0777;
	zend_bool *recursive = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lbO", &pathname, &mode, &recursive, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_mkdir(pathname, mode, recursive, zcontext, return_value);
} /* }}} */

/* {{{ proto bool File::rmdir(string pathname [, resource context])
   Remove a directory */
PHP_METHOD(File, rmdir) {
	zend_string *pathname;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|O", &pathname, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_rmdir(pathname, zcontext, return_value);
} /* }}} */

/* {{{ proto int File::readfile(string filename [, bool use_include_path[, resource context]])
   Output a file or a URL */
PHP_METHOD(File, readfile) {
	zend_string *filename;
	zend_bool use_include_path = 0;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|bO", &filename, &use_include_path, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_readfile(filename, use_include_path, zcontext, return_value);
} /* }}} */

/* {{{ proto bool File::rename(string old_name, string new_name[, resource context])
   Remove a directory */
PHP_METHOD(File, rename) {
	zend_string *old_name, new_name;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS|O", &old_name, &new_name, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_rename(old_name, new_name, zcontext, return_value);
} /* }}} */

/* {{{ proto bool File::unlink(string filename[, context context])
   Remove a directory */
PHP_METHOD(File, unlink) {
	zend_string *filename;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|O", &filename, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_unlink(filename, zcontext, return_value);
} /* }}} */

/* {{{ proto bool File::copy(string source_file, string destination_file [, resource context])
   Copy a file */
PHP_METHOD(File, copy) {
	zend_string *source, *destination;
	zval *zcontext = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "SS|O", &source, &destination, &zcontext, pthreads_stream_context_entry) != SUCCESS) {
		RETURN_FALSE;
	}

	if (zcontext != NULL && !Z_ISNULL_P(zcontext)  && !instanceof_function(Z_OBJCE_P(zcontext), pthreads_stream_context_entry)) {
		zend_throw_exception_ex(spl_ce_RuntimeException,
			0, "only StreamContext objects may be submitted, %s is no StreamContext",
			ZSTR_VAL(Z_OBJCE_P(zcontext)->name));
		RETURN_FALSE;
	}

	pthreads_streams_api_file_copy(ZSTR_VAL(source), ZSTR_VAL(destination), zcontext, return_value);
} /* }}} */


/* {{{ proto FileStream File::sockopen(string $hostname [, int $port = -1 [, int &$errno [, string &$errstr [, float $timeout = ini_get("default_socket_timeout") ]]]])
   */
PHP_METHOD(File, sockopen) {
	zend_string *host;
	zval *zerrno = NULL, *zerrstr = NULL;
	zend_long port = -1;
	double timeout = (double)FG(default_socket_timeout);

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|lz/z/d", &host, &port, &zerrno, &zerrstr, &timeout) != SUCCESS) {
		RETURN_NULL();
	}

	pthreads_streams_api_file_sockopen(host, port, zerrno, zerrstr, timeout, return_value);
} /* }}} */

#	endif
#endif
