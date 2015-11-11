/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012 - 2015                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Joe Watkins <krakjoe@php.net>                                |
  +----------------------------------------------------------------------+
 */
#ifndef HAVE_PTHREADS_COPY_H
#define HAVE_PTHREADS_COPY_H

/* {{{ */
static HashTable* pthreads_copy_statics(HashTable *old) {
	HashTable *statics = NULL;
	
	if (old) {
		zend_string *key;
		zval *value;

		ALLOC_HASHTABLE(statics);
		zend_hash_init(statics,
			zend_hash_num_elements(old),
			NULL, ZVAL_PTR_DTOR, 0);

		ZEND_HASH_FOREACH_STR_KEY_VAL(old, key, value) {
			zend_string *name = zend_string_new(key);
			zval *next = value;
			while (Z_TYPE_P(next) == IS_REFERENCE)
				next = &Z_REF_P(next)->val;

			if (Z_REFCOUNTED_P(next)) {
				zval copy;

				switch (Z_TYPE_P(next)) {
					case IS_STRING:
						ZVAL_STR(&copy, 
							zend_string_new(Z_STR_P(next)));
						zend_hash_add(statics, name, &copy);
					break;
					
					case IS_OBJECT:
						if (instanceof_function(Z_OBJCE_P(next), pthreads_threaded_entry) ||
							instanceof_function(Z_OBJCE_P(next), zend_ce_closure)) {
							pthreads_store_separate(next, &copy, 1);
							zend_hash_add(statics, name, &copy);
						} else zend_hash_add_empty_element(statics, name);
					break;

					case IS_ARRAY:
						pthreads_store_separate(next, &copy, 1);
						zend_hash_add(statics, name, &copy);
					break;
					
					default:
						zend_hash_add_empty_element(statics, name);					
				}
			} else zend_hash_add(statics, name, next);
			zend_string_release(name);
		} ZEND_HASH_FOREACH_END();
	}
	
	return statics;
} /* }}} */

/* {{{ */
static zend_string** pthreads_copy_variables(zend_string **old, int end) {
	zend_string **variables = safe_emalloc(end, sizeof(zend_string*), 0);
	int it = 0;
	
	while (it < end) {
		variables[it] = 
			zend_string_new(old[it]);
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

#if PHP_VERSION_ID < 70100
/* {{{ */
static zend_brk_cont_element* pthreads_copy_brk(zend_brk_cont_element *old, int end) {
	zend_brk_cont_element *brk_cont = safe_emalloc(end, sizeof(zend_brk_cont_element), 0);
	
	memcpy(
		brk_cont,
		old, 
		sizeof(zend_brk_cont_element) * end);
	
	return brk_cont;
} /* }}} */
#else
static zend_live_range* pthreads_copy_live(zend_live_range *old, int end) {
	zend_live_range *range = safe_emalloc(end, sizeof(zend_live_range), 0);

	memcpy(
		range,
		old,
		sizeof(zend_live_range) * end);

	return range;
}
#endif

/* {{{ */
static zval* pthreads_copy_literals(zval *old, int last) {
	zval *literals = (zval*) safe_emalloc(last, sizeof(zval), 0);
	zval *literal = literals,
		 *end = literals + last;

	memcpy(literals, old, sizeof(zval) * last);	

	while (literal < end) {
		switch (Z_TYPE_P(literal)) {
			case IS_ARRAY:
				pthreads_store_separate(literal, literal, 1);
			break;
			
			case IS_CONSTANT:
			case IS_STRING:
			case IS_CONSTANT_AST:
				zval_copy_ctor(literal);
			break;

		}
		literal++;
	}
	
	return literals;
} /* }}} */

/* {{{ */
static zend_op* pthreads_copy_opcodes(zend_op_array *op_array, zval *literals) {
	zend_op *copy = safe_emalloc(
		op_array->last, sizeof(zend_op), 0);

	memcpy(copy, op_array->opcodes, sizeof(zend_op) * op_array->last);

#if ZEND_USE_ABS_CONST_ADDR || ZEND_USE_ABS_JMP_ADDR
	zend_op *opline = copy;
	zend_op *end    = copy + op_array->last;

	for (; opline < end; opline++) {
#if ZEND_USE_ABS_CONST_ADDR
		if (opline->op1_type == IS_CONST)
			opline->op1.zv = (zval*)((char*)opline->op1.zv + ((char*)op_array->literals - (char*)literals));
		if (opline->op2_type == IS_CONST) 
			opline->op2.zv = (zval*)((char*)opline->op2.zv + ((char*)op_array->literals - (char*)literals));
#endif
#if ZEND_USE_ABS_JMP_ADDR
		if ((op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO) != 0) {
			switch (opline->opcode) {
				case ZEND_JMP:
				case ZEND_FAST_CALL:
				case ZEND_DECLARE_ANON_CLASS:
				case ZEND_DECLARE_ANON_INHERITED_CLASS:
					 opline->op1.jmp_addr = &copy[opline->op1.jmp_addr - op_array->opcodes];
				break;

				case ZEND_JMPZNZ:
				case ZEND_JMPZ:
				case ZEND_JMPNZ:
				case ZEND_JMPZ_EX:
				case ZEND_JMPNZ_EX:
				case ZEND_JMP_SET:
				case ZEND_COALESCE:
				case ZEND_NEW:
				case ZEND_FE_RESET_R:
				case ZEND_FE_RESET_RW:
				case ZEND_ASSERT_CHECK:
					opline->op2.jmp_addr = &copy[opline->op2.jmp_addr - op_array->opcodes];
				break;
			}
		}
#endif
	}
#endif

		

	return copy;
} /* }}} */

/* {{{ */
static zend_arg_info* pthreads_copy_arginfo(zend_op_array *op_array, zend_arg_info *old, uint32_t end) {
	zend_arg_info *info;
	uint32_t it = 0;

	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		old--;
		end++;
	}

	if (op_array->fn_flags & ZEND_ACC_VARIADIC) {
		end++;
	}

	info = safe_emalloc
		(end, sizeof(zend_arg_info), 0);
	memcpy(info, old, sizeof(zend_arg_info) * end);	

	while (it < end) {
		if (info[it].name)
			info[it].name = zend_string_new(old[it].name);
		if (info[it].class_name)
			info[it].class_name = zend_string_new(old[it].class_name);
		it++;
	}
	
	if (op_array->fn_flags & ZEND_ACC_HAS_RETURN_TYPE) {
		info++;
	}
	
	return info;
} /* }}} */

/* {{{ */
static inline zend_function* pthreads_copy_user_function(zend_function *function) {
	zend_function  *copy;
	zend_op_array  *op_array;
	zend_string   **variables;
	zval           *literals;
	zend_arg_info  *arg_info;
	
	copy = (zend_function*)
		zend_arena_alloc(&CG(arena), sizeof(zend_op_array));
	memcpy(copy, function, sizeof(zend_op_array));
	
	op_array = &copy->op_array;
	variables = op_array->vars;
	literals = op_array->literals;
	arg_info = op_array->arg_info;

	op_array->function_name = zend_string_new(op_array->function_name);
	/* we don't care about prototypes */
	op_array->prototype = NULL;
	op_array->refcount = emalloc(sizeof(uint32_t));
	(*op_array->refcount) = 1;
	/* we don't care if it's a closure */
	op_array->fn_flags &= ~ZEND_ACC_CLOSURE;
	/* we never want to share the same runtime cache */
	op_array->run_time_cache = NULL;

	if (op_array->doc_comment) {
		op_array->doc_comment = zend_string_new(op_array->doc_comment);
	}
	
	if (op_array->literals) op_array->literals = pthreads_copy_literals (literals, op_array->last_literal);

	op_array->opcodes = pthreads_copy_opcodes(op_array, literals);
	
	if (op_array->arg_info) 	op_array->arg_info = pthreads_copy_arginfo(op_array, arg_info, op_array->num_args);
#if PHP_VERSION_ID < 70100
	if (op_array->brk_cont_array) 	op_array->brk_cont_array = pthreads_copy_brk(op_array->brk_cont_array, op_array->last_brk_cont);
#else
	if (op_array->live_range)		op_array->live_range = pthreads_copy_live(op_array->live_range, op_array->last_live_range);
#endif
	if (op_array->try_catch_array)  op_array->try_catch_array = pthreads_copy_try(op_array->try_catch_array, op_array->last_try_catch);
	if (op_array->vars) 		op_array->vars = pthreads_copy_variables(variables, op_array->last_var);
	if (op_array->static_variables) op_array->static_variables = pthreads_copy_statics(op_array->static_variables);

	return copy;
} /* }}} */

/* {{{ */
static inline zend_function* pthreads_copy_internal_function(zend_function *function) {
	zend_function *copy = calloc(1, sizeof(zend_internal_function));
	memcpy(copy, function, sizeof(zend_internal_function));
	copy->common.function_name = 
		zend_string_new(function->common.function_name);
	return copy;
} /* }}} */

/* {{{ */
static zend_function* pthreads_copy_function(zend_function *function) {
	zend_function *copy = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong)function);
	
	if (copy) {
		function_add_ref(copy);
		return copy;
	}
	
	if (function->type == ZEND_USER_FUNCTION) {
		copy = pthreads_copy_user_function(function);
	} else {
		copy = pthreads_copy_internal_function(function);
	}

	return zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong) function, copy);
} /* }}} */

/* {{{ */
static void pthreads_copy_function_ctor(zval *bucket) {
	Z_PTR_P(bucket) = pthreads_copy_function(Z_PTR_P(bucket));
} /* }}} */
#endif

