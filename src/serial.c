/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                                		 |
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
#ifndef HAVE_PTHREADS_SERIAL
#define HAVE_PTHREADS_SERIAL

/*
* @TODO
*	1. write in support for other serialization methods like msgpack
*/

#ifndef HAVE_PTHREADS_SERIAL_H
#	include <src/serial.h>
#endif

/* {{{ Will serialize the zval into a newly allocated buffer which must be free'd by the caller */
char * pthreads_serialize(zval *unserial TSRMLS_DC){					
	char *result = NULL;
	
	if (unserial && Z_TYPE_P(unserial) != IS_NULL) {
		smart_str *output;
		php_serialize_data_t vars;
		
		PHP_VAR_SERIALIZE_INIT(vars);				
		output = (smart_str*) calloc(1, sizeof(smart_str));
		php_var_serialize(							
			output, 
			&unserial, 
			&vars TSRMLS_CC
		);
		PHP_VAR_SERIALIZE_DESTROY(vars);			
		result = (char*) calloc(1, output->len+1);	
		memcpy(result, output->c, output->len);
		smart_str_free(output);						
		free(output);	
	}							
	return result;														
}
/* }}} */

/* {{{ Will unserialize data into the allocated zval passed */
int pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC){	
	if (serial) {
		const unsigned char *pointer = (const unsigned char *)serial;
		php_unserialize_data_t vars;
		
		PHP_VAR_UNSERIALIZE_INIT(vars);
		if (!php_var_unserialize(&result, &pointer, pointer+strlen(serial), &vars TSRMLS_CC)) {
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
			zval_dtor(result);
			zend_error(E_WARNING, "The thread attempted to declare properties (%ld bytes of %s) that do not support serialization", strlen(serial), serial);
			return FAILURE;
		} else { 
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		}
		
		return SUCCESS;														
	} else return SUCCESS;
}
/* }}} */

/* {{{ Will unserialze data into a newly allocated buffer which must be free'd by the caller */
zval *	pthreads_unserialize(char *serial TSRMLS_DC){					
	zval *result;
	ALLOC_ZVAL(result);
	
	if (pthreads_unserialize_into(serial, result TSRMLS_CC)==SUCCESS) {
			return result;												
	} else return NULL;
}
/* }}} */
#endif
