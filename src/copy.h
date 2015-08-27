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
		HashPosition pos;
		zval **value;

		char *key_str;
		uint key_str_length;
		ulong num_index;

		ALLOC_HASHTABLE(statics);
		zend_hash_init(statics,
			zend_hash_num_elements(old),
			NULL, ZVAL_PTR_DTOR, 0);

		for (zend_hash_internal_pointer_reset_ex(old, &pos);
			 zend_hash_get_current_data_ex(old, (void **) &value, &pos) == SUCCESS;
			 zend_hash_move_forward_ex(old, &pos)
		) {
			zend_hash_get_current_key_ex(old, &key_str, &key_str_length, &num_index, 0, &pos);

			if (Z_TYPE_PP(value) != IS_ARRAY && Z_TYPE_PP(value) != IS_OBJECT && Z_TYPE_PP(value) != IS_RESOURCE) {
				zval *copy;
				ALLOC_ZVAL(copy);
				MAKE_COPY_ZVAL(value, copy);

				zend_hash_add(statics, key_str, key_str_length, &copy, sizeof(zval *), NULL);
			}
		}
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

#if PHP_VERSION_ID >= 50400
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
#endif

/* {{{ */
#if PHP_VERSION_ID >= 50400
static zend_op* pthreads_copy_opcodes(zend_op_array *op_array, zend_literal *literals) {
	zend_literal *literal;
#else
static zend_op* pthreads_copy_opcodes(zend_op_array *op_array) {
#endif
	zend_uint it = 0;
	zend_op *copy = safe_emalloc(op_array->last, sizeof(zend_op), 0);

	while (it < op_array->last) {
		copy[it] = op_array->opcodes[it];

#if PHP_VERSION_ID >= 50400
		if (copy[it].op1_type == IS_CONST) {
			literal =
				(zend_literal*)(op_array->opcodes[it].op1.zv);
			copy[it].op1.zv =
				&op_array->literals[literal - literals].constant;
#else
		if (copy[it].op1.op_type == IS_CONST) {
            zval_copy_ctor(&copy[it].op1.u.constant);
#endif
		} else {
			switch (copy[it].opcode) {
				case ZEND_GOTO:
				case ZEND_JMP:
#ifdef ZEND_FAST_CALL
				case ZEND_FAST_CALL:
#endif
#if PHP_VERSION_ID >= 50400
					copy[it].op1.jmp_addr = copy +
						(op_array->opcodes[it].op1.jmp_addr - op_array->opcodes);
#else
					copy[it].op1.u.jmp_addr = copy +
						(op_array->opcodes[it].op1.u.jmp_addr - op_array->opcodes);
#endif
				break;
			}
		}

#if PHP_VERSION_ID >= 50400
		if (copy[it].op2_type == IS_CONST) {
			literal =
				(zend_literal*)(op_array->opcodes[it].op2.zv);
			copy[it].op2.zv =
				&op_array->literals[literal - literals].constant;
#else
        if (copy[it].op2.op_type == IS_CONST) {
            zval_copy_ctor(&copy[it].op2.u.constant);
#endif
		} else {
			switch (copy[it].opcode) {
				case ZEND_JMPZ:
				case ZEND_JMPNZ:
				case ZEND_JMPZ_EX:
				case ZEND_JMPNZ_EX:
				case ZEND_JMP_SET:
#ifdef ZEND_JMP_SET_VAR
				case ZEND_JMP_SET_VAR:
#endif
#if PHP_VERSION_ID >= 50400
                    copy[it].op2.jmp_addr = copy +
						(op_array->opcodes[it].op2.jmp_addr - op_array->opcodes);
#else
                    copy[it].op2.u.jmp_addr = copy +
						(op_array->opcodes[it].op2.u.jmp_addr - op_array->opcodes);
#endif
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
#if PHP_VERSION_ID >= 50400
		zend_literal  *literals = op_array->literals;
#endif
		zend_arg_info *arg_info = op_array->arg_info;

		op_array->function_name = estrdup(op_array->function_name);
		op_array->refcount = emalloc(sizeof(zend_uint));
		(*op_array->refcount) = 1;
		op_array->prototype = function;
#if PHP_VERSION_ID >= 50400
		op_array->run_time_cache = NULL;
#endif

		if (op_array->doc_comment) {
			op_array->doc_comment = estrndup
				(op_array->doc_comment, op_array->doc_comment_len);
		}

		op_array->static_variables = pthreads_copy_statics(op_array->static_variables);
		op_array->vars = pthreads_copy_variables(variables, op_array->last_var);
#if PHP_VERSION_ID >= 50400
		op_array->literals = pthreads_copy_literals (literals, op_array->last_literal);
#endif
		op_array->arg_info = pthreads_copy_arginfo(arg_info, op_array->num_args);
#if PHP_VERSION_ID >= 50400
		op_array->opcodes = pthreads_copy_opcodes(op_array, literals);
#else
		op_array->opcodes = pthreads_copy_opcodes(op_array);
#endif
		op_array->try_catch_array = pthreads_copy_try(op_array->try_catch_array, op_array->last_try_catch);
		op_array->brk_cont_array = pthreads_copy_brk(op_array->brk_cont_array, op_array->last_brk_cont);

		*function = copy;
	}
} /* }}} */
#endif
