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
 
/*
* This header serves to resolve differences in versions of PHP and that is all
* @NOTE I hope to do away with this header completely, in time ...
*/
#ifndef HAVE_PTHREADS_COMPAT_H
#define HAVE_PTHREADS_COMPAT_H

#if PHP_VERSION_ID > 50399
static void zend_extension_op_array_dtor_handler(zend_extension *extension, zend_op_array *op_array TSRMLS_DC)
{
	if (extension->op_array_dtor) {
		extension->op_array_dtor(op_array);
	}
}

/*
* The only difference between this and the release of zend_function_dtor in 5.4.6
* is we free the run_time_cache if there are no more references to it and the release version free's it regardless
* Unsure of what this affects and how ...
*/
static void pthreads_method_del_ref(zend_function *function);
static void pthreads_method_del_ref(zend_function *function){
	if (function && function->type == ZEND_USER_FUNCTION ) {
		TSRMLS_FETCH();
		
		zend_op_array *op_array = (zend_op_array*) function;
		
		if (op_array) {
			zend_literal *literal = op_array->literals;
			zend_literal *end;
			zend_uint i;

			if (op_array->static_variables) {
				zend_hash_destroy(op_array->static_variables);
				FREE_HASHTABLE(op_array->static_variables);
			}
		 
			if (--(*op_array->refcount)>0) {
				return;
			}

			if (op_array->run_time_cache) {	
				free(
					op_array->run_time_cache
				);
			}
			
			free(op_array->refcount);

			if (op_array->vars) {
				i = op_array->last_var;
				while (i > 0) {
					i--;
					str_free(op_array->vars[i].name);
				}
				free(op_array->vars);
			}

			if (literal) {
				end = literal + op_array->last_literal;
				while (literal < end) {
					zval_dtor(&literal->constant);
					literal++;
				}
				free(op_array->literals);
			}
			free(op_array->opcodes);

			if (op_array->function_name) {
				free((char*)op_array->function_name);
			}
			if (op_array->doc_comment) {
				free((char*)op_array->doc_comment);
			}
			if (op_array->brk_cont_array) {
				free(op_array->brk_cont_array);
			}
			if (op_array->try_catch_array) {
				free(op_array->try_catch_array);
			}
			if (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO) {
				zend_llist_apply_with_argument(&zend_extensions, (llist_apply_with_arg_func_t) zend_extension_op_array_dtor_handler, op_array TSRMLS_CC);
			}
			if (op_array->arg_info) {
				for (i=0; i<op_array->num_args; i++) {
					str_free(op_array->arg_info[i].name);
					if (op_array->arg_info[i].class_name) {
						str_free(op_array->arg_info[i].class_name);
					}
				}
				free(op_array->arg_info);
			}
		}
	}
}
#endif /* PHP_VERSION_ID > 50399 */

#endif /* HAVE_PTHREADS_COMPAT_H */
