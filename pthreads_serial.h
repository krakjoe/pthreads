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
#ifndef HAVE_PTHREADS_SERIAL_H
#define HAVE_PTHREADS_SERIAL_H
char *					pthreads_serialize(zval *unserial TSRMLS_DC);
zval *					pthreads_unserialize(char *serial TSRMLS_DC);
int 					pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC);

char * pthreads_serialize(zval *unserial TSRMLS_DC){					/* will return newly allocate buffer with serial zval contained */
	char 					*result = NULL;
	
	if (unserial && Z_TYPE_P(unserial) != IS_NULL) {
		smart_str 				*output;
		php_serialize_data_t 	vars;
		
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
	return result;														/* remember to free this when you're done with it */
}

int pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC){	/* will unserialize a zval into the existing zval */
	if (serial) {
		const unsigned char *pointer = (const unsigned char *)serial;
		php_unserialize_data_t vars;
		
		PHP_VAR_UNSERIALIZE_INIT(vars);
		if (!php_var_unserialize(&result, &pointer, pointer+strlen(serial), &vars TSRMLS_CC)) {
			
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
			zval_dtor(result);
			zend_error(E_WARNING, "The thread attempted to use symbols that could not be unserialized");
			return FAILURE;
		} else { 
			PHP_VAR_UNSERIALIZE_DESTROY(vars);
		}
		
		return SUCCESS;														/* will only destroy the zval if there's an error in deserialization */
	} else return SUCCESS;
}

zval *	pthreads_unserialize(char *serial TSRMLS_DC){					/* will allocate a zval to contain the unserialized data */
	zval *result;
	ALLOC_ZVAL(result);
	
	if (pthreads_unserialize_into(serial, result TSRMLS_CC)==SUCCESS) {
			return result;												/* don't forget to free the variable when you're done with it */
	} else return NULL;													/* will return NULL on failure, maybe should return NULL zval ? */
}
#endif
