/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2014                                |
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
#ifndef HAVE_PTHREADS_COPY_H
#define HAVE_PTHREADS_COPY_H

/* {{{ */
static HashTable* pthreads_copy_statics(HashTable *old) {
	HashTable *statics = NULL;
	
	if (old) {
		zval *tmp;
	
		ALLOC_HASHTABLE(statics);
		zend_hash_init(statics,
			zend_hash_num_elements(old), 
			NULL, ZVAL_PTR_DTOR, 0);
		zend_hash_copy(
			statics, 
			old, (copy_ctor_func_t) zval_add_ref, 
			(void*)&tmp, sizeof(zval*));
	}
	
	return statics;
} /* }}} */

/* {{{ */
static zend_compiled_variable* pthreads_copy_variables(zend_compiled_variable *old, int end) {
	zend_compiled_variable *variables = safe_emalloc(end, sizeof(zend_compiled_variable), 0);
	int it = 0;
	
	while (it < end) {
		variables[it] = old[it];
		variables[it].name = estrndup(
			variables[it].name, 
			variables[it].name_len);
		it++;
	}
	
	return variables;
} /* }}} */

/* {{{ */
static zend_try_catch_element* pthreads_copy_try(zend_try_catch_element *old, int end) {	
	zend_try_catch_element *try_catch = safe_emalloc(end, sizeof(zend_try_catch_element), 0);
	
	memcpy(
		try_catch, 
		old,
		sizeof(zend_try_catch_element) * end);
	
	return try_catch;
} /* }}} */

/* {{{ */
static zend_brk_cont_element* pthreads_copy_brk(zend_brk_cont_element *old, int end) {
	zend_brk_cont_element *brk_cont = safe_emalloc(end, sizeof(zend_brk_cont_element), 0);
	
	memcpy(
		brk_cont,
		old, 
		sizeof(zend_brk_cont_element) * end);
	
	return brk_cont;
} /* }}} */

/* {{{ */
static zend_literal* pthreads_copy_literals(zend_literal *old, int end) {
	zend_literal *literals = safe_emalloc(end, sizeof(zend_literal), 0);
	int it = 0;
	
	while (it < end) {
		literals[it] = old[it];
		zval_copy_ctor(&literals[it].constant);
		it++;
	}
	
	return literals;
} /* }}} */

/* {{{ */
static zend_op* pthreads_copy_opcodes(zend_op_array *op_array, zend_literal *literals) {
	zend_literal *literal;
	zend_uint it = 0;
	zend_op *copy = safe_emalloc(op_array->last, sizeof(zend_op), 0);
	
	while (it < op_array->last) {
		copy[it] = op_array->opcodes[it];

		if (copy[it].op1_type == IS_CONST) {
			literal = 
				(zend_literal*)(op_array->opcodes[it].op1.zv);
			copy[it].op1.zv = 
				&op_array->literals[literal - literals].constant;
		} else {
			switch (copy[it].opcode) {
				case ZEND_GOTO:
				case ZEND_JMP:
#ifdef ZEND_FAST_CALL
				case ZEND_FAST_CALL:
#endif
					copy[it].op1.jmp_addr = copy + 
						(op_array->opcodes[it].op1.jmp_addr - op_array->opcodes);
				break;
			}
		}

		if (copy[it].op2_type == IS_CONST) {
			literal = 
				(zend_literal*)(op_array->opcodes[it].op2.zv);
			copy[it].op2.zv = 
				&op_array->literals[literal - literals].constant;
		} else {
			switch (copy[it].opcode) {
				case ZEND_JMPZ:
				case ZEND_JMPNZ:
				case ZEND_JMPZ_EX:
				case ZEND_JMPNZ_EX:
				case ZEND_JMP_SET:
				case ZEND_JMP_SET_VAR:
					copy[it].op2.jmp_addr = copy +
						(op_array->opcodes[it].op2.jmp_addr - op_array->opcodes);
				break;
			}
		}

		it++;
	}

	return copy;
} /* }}} */

/* {{{ */
static zend_arg_info* pthreads_copy_arginfo(zend_arg_info *old, zend_uint end) {
	zend_arg_info *info = safe_emalloc(end, sizeof(zend_arg_info), 0);
	zend_uint it = 0;	
	
	while (it < end) {
		info[it] = old[it];
		info[it].name = estrndup(
			info[it].name, info[it].name_len);
		if (info[it].class_name) {
			info[it].class_name = estrndup(
				info[it].class_name, info[it].class_name_len);
		}
		it++;
	}
	
	return info;
} /* }}} */

/* {{{ */
static void pthreads_copy_function(zend_function *function) {
	if (function->type == ZEND_USER_FUNCTION) {
		zend_function copy = *function;
		zend_function *copied = NULL;
		
		zend_op_array *op_array = &copy.op_array;
		zend_compiled_variable *variables = op_array->vars;
		zend_literal  *literals = op_array->literals;
		zend_arg_info *arg_info = op_array->arg_info;
		
		op_array->function_name = estrdup(op_array->function_name);
		op_array->refcount = emalloc(sizeof(zend_uint));
		(*op_array->refcount) = 1;
		op_array->prototype = function;
		op_array->run_time_cache = NULL;
		
		if (op_array->doc_comment) {
			op_array->doc_comment = estrndup
				(op_array->doc_comment, op_array->doc_comment_len);
		}
		
		op_array->static_variables = pthreads_copy_statics(op_array->static_variables);
		op_array->vars = pthreads_copy_variables(variables, op_array->last_var);
		op_array->literals = pthreads_copy_literals (literals, op_array->last_literal);
		op_array->arg_info = pthreads_copy_arginfo(arg_info, op_array->num_args);
		op_array->opcodes = pthreads_copy_opcodes(op_array, literals);
		op_array->try_catch_array = pthreads_copy_try(op_array->try_catch_array, op_array->last_try_catch);
		op_array->brk_cont_array = pthreads_copy_brk(op_array->brk_cont_array, op_array->last_brk_cont);
		
		*function = copy;
	}
} /* }}} */
#endif


