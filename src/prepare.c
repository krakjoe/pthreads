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
#ifndef HAVE_PTHREADS_PREPARE
#define HAVE_PTHREADS_PREPARE

#ifndef HAVE_PTHREADS_PREPARE_H
#	include <src/prepare.h>
#endif

#ifndef HAVE_PTHREADS_OBJECT_H
#	include <src/object.h>
#endif

#ifndef HAVE_PTHREADS_RESOURCES_H
#	include <src/resources.h>
#endif

#ifndef HAVE_PTHREADS_GLOBALS_H
#	include <src/globals.h>
#endif

#ifndef HAVE_PTHREADS_COPY_H
#   include <src/copy.h>
#endif

#define PTHREADS_PREPARATION_BEGIN_CRITICAL() pthreads_globals_lock();
#define PTHREADS_PREPARATION_END_CRITICAL()   pthreads_globals_unlock()

/* {{{ */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(pthreads_object_t* thread, zend_trait_alias *alias); 
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(pthreads_object_t* thread, zend_trait_precedence *precedence);
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(pthreads_object_t* thread, zend_trait_method_reference *reference);
static void pthreads_prepared_resource_dtor(zval *zv); /* }}} */

/* {{{ */
static zend_class_entry* pthreads_copy_entry(pthreads_object_t* thread, zend_class_entry *candidate) {
	zend_class_entry *prepared;

	if (candidate->ce_flags & ZEND_ACC_ANON_CLASS) {
		if (!(candidate->ce_flags & ZEND_ACC_ANON_BOUND)) {
			return NULL;
		}
	}

	prepared = (zend_class_entry*) emalloc(sizeof(zend_class_entry));
	prepared->name = zend_string_new(candidate->name);
	prepared->type = candidate->type;

	zend_initialize_class_data(prepared, 1);

	zend_hash_index_update_ptr(&PTHREADS_ZG(resolve), (zend_ulong) candidate, prepared);

	prepared->ce_flags = candidate->ce_flags;
	prepared->refcount = 1;

	if (candidate->parent) {
		if (candidate->parent->type == ZEND_USER_CLASS) {
			if (zend_hash_index_exists(&PTHREADS_ZG(resolve), (zend_ulong) candidate->parent)) {
				prepared->parent = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong) candidate->parent);
			} else prepared->parent = pthreads_prepared_entry(thread, candidate->parent);
		} else prepared->parent = zend_lookup_class(candidate->parent->name);
	}

	if (candidate->num_interfaces) {
		uint interface;
		prepared->interfaces = emalloc(sizeof(zend_class_entry*) * candidate->num_interfaces);
		for(interface=0; interface<candidate->num_interfaces; interface++)
			prepared->interfaces[interface] = pthreads_prepared_entry(thread, candidate->interfaces[interface]);
		prepared->num_interfaces = candidate->num_interfaces;
	} else prepared->num_interfaces = 0;

	if (candidate->num_traits) {
		uint trait;
		prepared->traits = emalloc(sizeof(zend_class_entry*) * candidate->num_traits);
		for (trait=0; trait<candidate->num_traits; trait++)
			prepared->traits[trait] = pthreads_prepared_entry(thread, candidate->traits[trait]);
		prepared->num_traits = candidate->num_traits;
		
		if (candidate->trait_aliases) {
			size_t alias = 0;

			while (candidate->trait_aliases[alias]) {
				alias++;
			}
			prepared->trait_aliases = emalloc(sizeof(zend_trait_alias*) * (alias+1));
			alias = 0;

			while (candidate->trait_aliases[alias]) {
				prepared->trait_aliases[alias] = pthreads_preparation_copy_trait_alias(
					thread, candidate->trait_aliases[alias]
				);
				alias++;
			}
			prepared->trait_aliases[alias]=NULL;
		} else prepared->trait_aliases = NULL;
		
		if (candidate->trait_precedences) {
			size_t precedence = 0;
			            
            while (candidate->trait_precedences[precedence]) {
                precedence++;
            }
            prepared->trait_precedences = emalloc(sizeof(zend_trait_precedence*) * (precedence+1));
            precedence = 0;
            
            while (candidate->trait_precedences[precedence]) {
	            prepared->trait_precedences[precedence] = pthreads_preparation_copy_trait_precedence(
		            thread, candidate->trait_precedences[precedence]
	            );
	            precedence++;
            }
			prepared->trait_precedences[precedence]=NULL;
		} else prepared->trait_precedences = NULL;
	} else {
		prepared->num_traits = 0;
		prepared->trait_aliases = 0;
		prepared->trait_precedences = 0;
	}

	{
		uint umethod = 0;
		void *usources[7] = {
			candidate->create_object,
			candidate->serialize,
			candidate->unserialize,
			candidate->get_iterator,
			candidate->interface_gets_implemented,
			candidate->get_static_method
		};
		
		do {
			if (usources[umethod]) {
				switch(umethod){
					case 0: prepared->create_object = candidate->create_object; break;
					case 1: prepared->serialize = candidate->serialize; break;
					case 2: prepared->unserialize = candidate->unserialize; break;
					case 3: {
						prepared->get_iterator = candidate->get_iterator;
						prepared->iterator_funcs = candidate->iterator_funcs;
					} break;
					case 4: prepared->interface_gets_implemented = candidate->interface_gets_implemented; break;
					case 5: prepared->get_static_method = candidate->get_static_method; break;
				}
			}
		} while(++umethod < 7);
	}
	
	{
		zend_string *key;
		zend_function *value;
		
		ZEND_HASH_FOREACH_STR_KEY_PTR(&candidate->function_table, key, value) {	
			zend_string *name = zend_string_new(key);
			value = pthreads_copy_function(value);
			zend_hash_add_ptr(
				&prepared->function_table, name, value);
			zend_string_release(name);
		} ZEND_HASH_FOREACH_END();
	}
	
	{
	    zend_function *func;
	    zend_string *name;

	    if (!prepared->constructor && zend_hash_num_elements(&prepared->function_table)) {
	        if ((func = zend_hash_str_find_ptr(&prepared->function_table, "__construct", sizeof("__construct")-1))) {
	            prepared->constructor = func;
	        } else {
			name = zend_string_tolower(prepared->name);
			if ((func = zend_hash_find_ptr(&prepared->function_table, name))) {
				prepared->constructor = func;
			}
			zend_string_release(name);
		}
	    }
	    
#define FIND_AND_SET(f, n) do {\
    if (!prepared->f && zend_hash_num_elements(&prepared->function_table)) { \
        if ((func = zend_hash_str_find_ptr(&prepared->function_table, n, sizeof(n)-1))) { \
            prepared->f = func; \
        } \
    } \
} \
while(0)
        
        FIND_AND_SET(clone, "__clone");
        FIND_AND_SET(__get, "__get");
        FIND_AND_SET(__set, "__set");
        FIND_AND_SET(__unset, "__unset");
        FIND_AND_SET(__isset, "__isset");
        FIND_AND_SET(__call, "__call");
        FIND_AND_SET(__callstatic, "__callstatic");
        FIND_AND_SET(serialize_func, "serialize");
        FIND_AND_SET(unserialize_func, "unserialize");
        FIND_AND_SET(__tostring, "__tostring");
        FIND_AND_SET(destructor, "__destruct");
#undef FIND_AND_SET
	}

	{
		zend_property_info *info;
		zend_string *name;
		ZEND_HASH_FOREACH_STR_KEY_PTR(&candidate->properties_info, name, info) {
			zend_property_info dup = *info;

			dup.name = zend_string_new(info->name);
			
			if (info->doc_comment) {
				if (thread->options & PTHREADS_INHERIT_COMMENTS) {
					dup.doc_comment = zend_string_new(info->doc_comment);
				} else dup.doc_comment = NULL;
			}

			if (info->ce) {
				if (info->ce == candidate) {
					dup.ce = prepared;
				} else dup.ce = pthreads_prepared_entry(thread, info->ce);
			}
			
			if (!zend_hash_str_add_mem(&prepared->properties_info, name->val, name->len, &dup, sizeof(zend_property_info))) {		
				zend_string_release(dup.name);
				if (dup.doc_comment)
					zend_string_release(dup.doc_comment);
			}
		} ZEND_HASH_FOREACH_END();
	}

#define SET_ITERATOR_FUNC(f) do { \
	if (candidate->iterator_funcs.f) { \
		prepared->iterator_funcs.f = zend_hash_index_find_ptr( \
			&PTHREADS_ZG(resolve), (zend_ulong) candidate->iterator_funcs.f); \
	} \
} while (0)
	
	memcpy(&prepared->iterator_funcs, &candidate->iterator_funcs, sizeof(zend_class_iterator_funcs));

	SET_ITERATOR_FUNC(zf_new_iterator);
	SET_ITERATOR_FUNC(zf_valid);
	SET_ITERATOR_FUNC(zf_current);
	SET_ITERATOR_FUNC(zf_key);
	SET_ITERATOR_FUNC(zf_next);
	SET_ITERATOR_FUNC(zf_rewind);


#undef SET_ITERATOR_FUNC
	
	if (candidate->default_properties_count) {
		int i;
		prepared->default_properties_table = emalloc(
			sizeof(zval) * candidate->default_properties_count);

		memcpy(
			prepared->default_properties_table,
			candidate->default_properties_table,
			sizeof(zval) * candidate->default_properties_count);

		for (i=0; i<candidate->default_properties_count; i++) {
			if (Z_REFCOUNTED(prepared->default_properties_table[i])) {
				pthreads_store_separate(
					&candidate->default_properties_table[i],
					&prepared->default_properties_table[i], 1);
			}
		}
		prepared->default_properties_count = candidate->default_properties_count;
	} else prepared->default_properties_count = 0;

	if (candidate->default_static_members_count) {
		int i;
		prepared->default_static_members_table = (zval*) ecalloc(
			sizeof(zval), candidate->default_static_members_count);
		prepared->default_static_members_count = candidate->default_static_members_count;
		memcpy(prepared->default_static_members_table,
		       candidate->default_static_members_table,
			sizeof(zval) * candidate->default_static_members_count);
		for (i=0; i<prepared->default_static_members_count; i++) {
			switch (Z_TYPE(prepared->default_static_members_table[i])) {
				case IS_STRING: {
					ZVAL_STR(
						&prepared->default_static_members_table[i], 
						zend_string_new(Z_STR(prepared->default_static_members_table[i])));
				} break;

				default: if (Z_REFCOUNTED(prepared->default_static_members_table[i])) {
					ZVAL_NULL(&prepared->default_static_members_table[i]);
				} 
			}
		}
		prepared->static_members_table = prepared->default_static_members_table;
	} else prepared->default_static_members_count = 0;

	memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));

	if ((thread->options & PTHREADS_INHERIT_COMMENTS) &&
	   (candidate->info.user.doc_comment)) {
        	prepared->info.user.doc_comment = zend_string_new(candidate->info.user.doc_comment);
    	} else prepared->info.user.doc_comment = NULL;
	
	if (prepared->info.user.filename)
		prepared->info.user.filename = zend_string_new(candidate->info.user.filename);

	{
		zend_string *key;
		zval *value;

		ZEND_HASH_FOREACH_STR_KEY_VAL(&candidate->constants_table, key, value) {
			zend_string *name;
			zval separated;

			if (zend_hash_exists(&prepared->constants_table, key)) {
				continue;
			}

			switch (Z_TYPE_P(value)) {
				case IS_STRING:
				case IS_ARRAY:
				case IS_OBJECT: {
					if (pthreads_store_separate(value, &separated, 1) != SUCCESS) {
						continue;				
					}
				} break;

				default: ZVAL_COPY(&separated, value);	
			}

			name = zend_string_new(key);
			zend_hash_update(
				&prepared->constants_table, name, &separated);
			zend_string_release(name);
		} ZEND_HASH_FOREACH_END();
	}

	return prepared;
} /* }}} */

/* {{{ */
static inline int pthreads_prepared_entry_function_prepare(zval *bucket, int argc, va_list argv, zend_hash_key *key) {
	zend_function *function = (zend_function*) Z_PTR_P(bucket);
	pthreads_object_t* thread = va_arg(argv, pthreads_object_t*);	
	zend_class_entry *prepared = va_arg(argv, zend_class_entry*);
	zend_class_entry *candidate = va_arg(argv, zend_class_entry*);
	zend_class_entry *scope = function->common.scope;
	
	if (function->type == ZEND_USER_FUNCTION) {
		if (scope == candidate) {
			function->common.scope = prepared;
		} else {
			if (function->common.scope->type == ZEND_USER_CLASS) {
				function->common.scope = pthreads_prepared_entry(thread, function->common.scope);
			}
		}

		/* runtime cache relies on immutable scope, so if scope changed, reallocate runtime cache */
		/* IT WOULD BE NICE IF THIS WERE DOCUMENTED SOMEWHERE OTHER THAN PHP-SRC */
		if (function->common.scope != scope) {
			zend_op_array *op_array = &function->op_array;
			op_array->run_time_cache = emalloc(op_array->cache_size);
			memset(op_array->run_time_cache, 0, op_array->cache_size);
			op_array->fn_flags |= ZEND_ACC_NO_RT_ARENA;
		}
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ */
zend_class_entry* pthreads_prepared_entry(pthreads_object_t* thread, zend_class_entry *candidate) {
	zend_class_entry *prepared = NULL;
	zend_string *lookup = NULL;

	if (!candidate || 
		(prepared = zend_hash_find_ptr(EG(class_table), candidate->name))) {
		return prepared;
	}

	lookup = zend_string_tolower(candidate->name);

	if ((prepared = zend_hash_find_ptr(EG(class_table), lookup))) {
	    zend_string_release(lookup);
		return prepared;
	}
	
	if (!(prepared = pthreads_copy_entry(thread, candidate))) {
		zend_string_release(lookup);
		return NULL;
	}

	zend_hash_apply_with_arguments(
		&prepared->function_table, 
		pthreads_prepared_entry_function_prepare, 
		3, thread, prepared, candidate);	
	
	zend_hash_update_ptr(EG(class_table), lookup, prepared);

	zend_string_release(lookup);

	return prepared;
} /* }}} */

/* {{{ */
static inline zend_bool pthreads_constant_exists(zend_string *name) {
    int retval = 1;
    zend_string *lookup;

    if (!zend_hash_exists(EG(zend_constants), name)) {
        lookup = zend_string_tolower(name);
        retval = zend_hash_exists(EG(zend_constants), lookup);
        zend_string_release(lookup);
    }

    return retval;
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_ini(pthreads_object_t* thread) {
	zend_ini_entry *entry[2];
	zend_string *name;
	HashTable *table[2] = {PTHREADS_EG(thread->creator.ls, ini_directives), EG(ini_directives)};

	if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
		zend_alter_ini_entry_chars(
			PTHREADS_G(strings).session.cache_limiter,
			"nocache", sizeof("nocache")-1, 
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
		zend_alter_ini_entry_chars(
			PTHREADS_G(strings).session.use_cookies,
			"0", sizeof("0")-1,
			PHP_INI_USER, PHP_INI_STAGE_ACTIVATE);
	}	

	ZEND_HASH_FOREACH_STR_KEY_PTR(table[0], name, entry[0]) {
		if ((entry[1] = zend_hash_find_ptr(table[1], name))) {
			if (entry[0]->value && entry[1]->value) {
				if (memcmp(ZSTR_VAL(entry[0]->value), ZSTR_VAL(entry[1]->value), ZSTR_LEN(entry[0]->value)) != SUCCESS) {
					zend_bool resmod = entry[1]->modifiable;
					zend_string *copied = zend_string_new(name); 

					if (!EG(modified_ini_directives)) {
						ALLOC_HASHTABLE(EG(modified_ini_directives));
						zend_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
					}

					if (!entry[1]->modified) {
						entry[1]->orig_value = entry[1]->value;
						entry[1]->orig_modifiable = entry[1]->modifiable;
						entry[1]->modified = 1;
						zend_hash_add_ptr(EG(modified_ini_directives), copied, entry[1]);
					}

					entry[1]->modifiable = 1;
					entry[1]->on_modify(entry[1], entry[0]->value, entry[1]->mh_arg1, entry[1]->mh_arg2, entry[1]->mh_arg3, ZEND_INI_SYSTEM);
					if (entry[1]->modified && entry[1]->orig_value != entry[1]->value) {
						zend_string_release(entry[1]->value);
					}
					entry[1]->value = zend_string_new(entry[0]->value);			
					entry[1]->modifiable = resmod;

					zend_string_release(copied);
				}
			}
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_constants(pthreads_object_t* thread) {
	zend_constant *zconstant;
	zend_string *name;
	
	ZEND_HASH_FOREACH_STR_KEY_PTR(PTHREADS_EG(thread->creator.ls, zend_constants), name, zconstant) {
		if (zconstant->name) {
		    if (strncmp(name->val, "STDIN", name->len-1)==0||
			    strncmp(name->val, "STDOUT", name->len-1)==0||
			    strncmp(name->val, "STDERR", name->len-1)==0){
			    continue;
		    } else {
			    zend_constant constant;

			    if (!pthreads_constant_exists(name)) {

				    constant.flags = zconstant->flags;
				    constant.module_number = zconstant->module_number;
				    constant.name = zend_string_new(name);

				    switch((Z_TYPE_INFO(constant.value)=Z_TYPE(zconstant->value))) {
					    case IS_TRUE:
					    case IS_FALSE:
					    case IS_LONG: {
						    Z_LVAL(constant.value)=Z_LVAL(zconstant->value);
					    } break;
					    case IS_DOUBLE: Z_DVAL(constant.value)=Z_DVAL(zconstant->value); break;
					    case IS_STRING: {
							ZVAL_NEW_STR(&constant.value, zend_string_new(Z_STR(zconstant->value)));
					    } break;
				    }
			
				    zend_register_constant(&constant);
			    }
		    }
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_functions(pthreads_object_t* thread) {
	zend_string *key, *name;
	zend_function *value = NULL, *prepared = NULL;

	ZEND_HASH_FOREACH_STR_KEY_PTR(PTHREADS_CG(thread->creator.ls, function_table), key, value) {
		if (value->type == ZEND_INTERNAL_FUNCTION ||
			zend_hash_exists(PTHREADS_CG(thread->local.ls, function_table), key))
			continue;

		name = zend_string_new(key);
		prepared = pthreads_copy_function(value);

		if (!zend_hash_add_ptr(CG(function_table), name, prepared)) {
			destroy_op_array((zend_op_array*)prepared);
			zend_string_release(name);
			continue;
		}

		if (!(GC_FLAGS(name) & IS_STR_PERSISTENT)) {
			zend_string_release(name);
		}
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_classes(pthreads_object_t* thread) {
	zend_class_entry *entry, *prepared;
	zend_string *name;
	HashTable inherited;

	zend_hash_init(&inherited, zend_hash_num_elements(PTHREADS_CG(thread->creator.ls, class_table)), NULL, NULL, 0);

	ZEND_HASH_FOREACH_STR_KEY_PTR(PTHREADS_CG(thread->creator.ls, class_table), name, entry) {
		if (entry->type == ZEND_USER_CLASS) {
			zend_string *lookup;

			if (zend_hash_exists(PTHREADS_CG(thread->local.ls, class_table), name)) {
				continue;
			}

			lookup = zend_string_tolower(name);
			
			if (zend_hash_exists(PTHREADS_CG(thread->local.ls, class_table), lookup)) {
				zend_string_release(lookup);
				continue;
			}

			zend_string_release(lookup);

			if (!(prepared = pthreads_prepared_entry(thread, entry))) {
				continue;
			}

			zend_hash_next_index_insert_ptr(&inherited, prepared);			
		}
	} ZEND_HASH_FOREACH_END();		

	ZEND_HASH_FOREACH_PTR(&inherited, entry) {
		if (entry->parent) {
			prepared = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong) entry->parent);
			if (prepared) {
				entry->parent = prepared;
			}
		}
	} ZEND_HASH_FOREACH_END();

	zend_hash_destroy(&inherited);  
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_includes(pthreads_object_t* thread) {
	zend_string *file;
	ZEND_HASH_FOREACH_STR_KEY(&PTHREADS_EG(thread->creator.ls, included_files), file) {
		zend_string *name = zend_string_new(file);
		zend_hash_add_empty_element(&EG(included_files), name);
		zend_string_release(name);
	} ZEND_HASH_FOREACH_END();
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_exception_handler(pthreads_object_t* thread) {
	zval *handler = &PTHREADS_EG(thread->creator.ls, user_exception_handler);

	if (thread->options & PTHREADS_INHERIT_CLASSES|PTHREADS_INHERIT_FUNCTIONS) {
		if (Z_TYPE_P(handler) != IS_UNDEF) {
			if (Z_TYPE_P(handler) == IS_ARRAY) {
				if (zend_hash_num_elements(Z_ARRVAL_P(handler)) > 1) {
					if (!(thread->options & PTHREADS_INHERIT_CLASSES)) {
						return;
					}
				} else if(!(thread->options & PTHREADS_INHERIT_FUNCTIONS)) {
					return;
				}
			}

			pthreads_store_separate(handler, &EG(user_exception_handler), 1);
		}
	}
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_resource_destructor(pthreads_object_t* thread) {
	if (!PTHREADS_G(default_resource_dtor))
		PTHREADS_G(default_resource_dtor)=(EG(regular_list).pDestructor);
	EG(regular_list).pDestructor =  (dtor_func_t) pthreads_prepared_resource_dtor;	
} /* }}} */

/* {{{ */
static inline void pthreads_prepare_sapi(pthreads_object_t* thread) {
	SG(sapi_started) = 0;

	if (!(thread->options & PTHREADS_ALLOW_HEADERS)) {
		SG(headers_sent)=1;
		SG(request_info).no_headers = 1;
	}
} /* }}} */

/* {{{ */
static inline void pthreads_rebuild_object(zval *zv) {
	if (Z_TYPE_P(zv) == IS_OBJECT) {
		rebuild_object_properties(Z_OBJ_P(zv));
	} else if (Z_TYPE_P(zv) == IS_ARRAY) {
		zval *object = zend_hash_index_find(Z_ARRVAL_P(zv), 0);
		if (object && Z_TYPE_P(object) == IS_OBJECT) {
			rebuild_object_properties(Z_OBJ_P(object));
		}
	}
} /* }}} */

/* {{{ */
void pthreads_prepare_parent(pthreads_object_t *thread) {
	if (Z_TYPE(EG(user_exception_handler)) != IS_UNDEF)
		pthreads_rebuild_object(&EG(user_exception_handler));
} /* }}} */

/* {{{ */
int pthreads_prepared_startup(pthreads_object_t* thread, pthreads_monitor_t *ready) {

	PTHREADS_PREPARATION_BEGIN_CRITICAL() {
		thread->local.id = pthreads_self();
		thread->local.ls = ts_resource(0);
		TSRMLS_CACHE_UPDATE();

		SG(server_context) = 
			PTHREADS_SG(thread->creator.ls, server_context);

		PG(expose_php) = 0;
		PG(auto_globals_jit) = 0;

		php_request_startup();
		pthreads_prepare_sapi(thread);
	
		if (thread->options & PTHREADS_INHERIT_INI)
			pthreads_prepare_ini(thread);

		if (thread->options & PTHREADS_INHERIT_CONSTANTS)
			pthreads_prepare_constants(thread);

		if (thread->options & PTHREADS_INHERIT_FUNCTIONS)
			pthreads_prepare_functions(thread);

		if (thread->options & PTHREADS_INHERIT_CLASSES)
			pthreads_prepare_classes(thread);

		if (thread->options & PTHREADS_INHERIT_INCLUDES)
			pthreads_prepare_includes(thread);

		pthreads_prepare_exception_handler(thread);
		pthreads_prepare_resource_destructor(thread);
		pthreads_monitor_add(ready, PTHREADS_MONITOR_READY);
	} PTHREADS_PREPARATION_END_CRITICAL();

	return SUCCESS;
} /* }}} */

/* {{{ */
static inline int pthreads_resources_cleanup(zval *bucket) {
	if (pthreads_resources_kept(Z_RES_P(bucket))) {
		return ZEND_HASH_APPLY_REMOVE;
	} else return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ */
int pthreads_prepared_shutdown(pthreads_object_t* thread) {
	PTHREADS_PREPARATION_BEGIN_CRITICAL() {
		zend_hash_apply(&EG(regular_list), pthreads_resources_cleanup);		

		PG(report_memleaks) = 0;

		php_request_shutdown((void*)NULL);

		ts_free_thread();
	} PTHREADS_PREPARATION_END_CRITICAL();

	return SUCCESS;
} /* }}} */

/* {{{ */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(pthreads_object_t* thread, zend_trait_alias *alias) {
	zend_trait_alias *copy = ecalloc(1, sizeof(zend_trait_alias));
	if (copy) {
		if (copy->trait_method) {
			copy->trait_method = pthreads_preparation_copy_trait_method_reference(thread, alias->trait_method);
		}
		
		copy->alias = zend_string_new(alias->alias);
		copy->modifiers = alias->modifiers;
	}
	return copy;
} /* }}} */

/* {{{ */
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(pthreads_object_t* thread, zend_trait_precedence *precedence) {
	zend_trait_precedence *copy = ecalloc(1, sizeof(zend_trait_precedence));
	if (copy) {
		copy->trait_method = pthreads_preparation_copy_trait_method_reference(thread, precedence->trait_method);
		if (precedence->exclude_from_classes) {
			copy->exclude_from_classes = emalloc(sizeof(*copy->exclude_from_classes));
			copy->exclude_from_classes->ce = pthreads_prepared_entry(
				thread, precedence->exclude_from_classes->ce
			);
			copy->exclude_from_classes->class_name = zend_string_new(precedence->exclude_from_classes->class_name);
		}
	}
	return copy;
} /* }}} */

/* {{{  */
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(pthreads_object_t* thread, zend_trait_method_reference *reference) {
	zend_trait_method_reference *copy = ecalloc(1, sizeof(zend_trait_method_reference));
	if (copy) {
		copy->method_name = zend_string_new(reference->method_name);
		if (reference->class_name) {
			copy->class_name = zend_string_new(reference->class_name);
		}
		copy->ce = pthreads_prepared_entry(thread, (zend_class_entry*) reference->ce);
	}
	return copy;
} /* }}} */

/* {{{ */
static void pthreads_prepared_resource_dtor(zval *zv) {
	zend_try {
		if (!pthreads_resources_kept(Z_RES_P(zv))){
			if (PTHREADS_G(default_resource_dtor))
				PTHREADS_G(default_resource_dtor)(zv);
		} else if (PTHREADS_ZG(resources)) {
                
                }
	} zend_end_try();
} /* }}} */
#endif

