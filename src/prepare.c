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

#ifndef HAVE_PTHREADS_STORE_H
#	include <src/store.h>
#endif

#ifndef HAVE_PTHREADS_COPY_H
#   include <src/copy.h>
#endif

/* {{{ prepared property info ctor */
static void pthreads_preparation_property_info_copy_ctor(zval *); /* }}} */

/* {{{ prepared property info dummy ctor */
static void pthreads_preparation_property_info_dummy_ctor(zval *); /* }}} */
/* {{{ prepared property info dummy dtor */
static void pthreads_preparation_property_info_dummy_dtor(zval *); /* }}} */

#if PHP_VERSION_ID < 50400
/* {{{ default property ctor for 5.3 */
static void pthreads_preparation_default_properties_ctor(zval **property); /* }}} */
/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property); /* }}} */
#else
/* {{{ trail alias copy for 5.4 */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(PTHREAD thread, zend_trait_alias *alias); /* }}} */
/* {{{ trait precendence for 5.4 */
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(PTHREAD thread, zend_trait_precedence *precedence); /* }}} */
/* {{{ method reference copy for traits */
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(PTHREAD thread, zend_trait_method_reference *reference); /* }}} */
#endif

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_method_scope(zend_function *function); /* }}} */

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_property_scope(zend_property_info *info); /* }}} */

/* {{{ prepared resource destructor */
static void pthreads_prepared_resource_dtor(zval *zv); /* }}} */

static void pthreads_function_add_ref(zval *bucket) {
	zend_function *function = Z_PTR_P(bucket);
	
	function_add_ref(function);
}

static zend_class_entry* pthreads_copy_entry(PTHREAD thread, zend_class_entry *candidate) {
	zend_class_entry *prepared = NULL;

	/* create a new user class for this context */
	prepared = (zend_class_entry*) emalloc(sizeof(zend_class_entry));
	prepared->name = zend_string_new(candidate->name);
	prepared->type = candidate->type;
	
	/* initialize class data */
	zend_initialize_class_data(prepared, 1);
	
	zend_hash_index_update_ptr(
		&PTHREADS_ZG(resolve),
		(zend_ulong) candidate, prepared);
	
	/* set ce flags (reset by initialize) */
	prepared->ce_flags = candidate->ce_flags;
	prepared->refcount = 1;
	
	/* parents are set late */
	if (candidate->parent)
	    prepared->parent = candidate->parent;
	
	/* declare interfaces */
	if (candidate->num_interfaces) {
		uint interface;
		prepared->interfaces = emalloc(sizeof(zend_class_entry*) * candidate->num_interfaces);
		for(interface=0; interface<candidate->num_interfaces; interface++)
			prepared->interfaces[interface] = pthreads_prepared_entry(thread, candidate->interfaces[interface]);
		prepared->num_interfaces = candidate->num_interfaces;
	} else prepared->num_interfaces = 0;

	/* traits */
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

	/* copy uternals ! */
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
	
	/* copy function table */
	zend_hash_copy(&prepared->function_table, &candidate->function_table, (copy_ctor_func_t) pthreads_copy_function_ctor);
	
	{
	    zend_function *func;
	    zend_string *name;
		
	    if (!prepared->constructor && zend_hash_num_elements(&prepared->function_table)) {
	        if ((func = zend_hash_str_find_ptr(&prepared->function_table, "__construct", sizeof("__construct")))) {
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

	/* copy property info structures */
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
	
	/* copy default properties */
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
				/* we use real separation for a reason */
				pthreads_store_separate(
					&candidate->default_properties_table[i],
					&prepared->default_properties_table[i], 1);
			}
		}
		prepared->default_properties_count = candidate->default_properties_count;
	} else prepared->default_properties_count = 0;

	/* copy static properties */
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
					ZVAL_UNDEF(&prepared->default_static_members_table[i]);
				} 
			}
		}
		prepared->static_members_table = prepared->default_static_members_table;
	} else prepared->default_static_members_count = 0;

	/* copy user info struct */
	memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));

	/* copy comments where required */
	if ((thread->options & PTHREADS_INHERIT_COMMENTS) &&
	   (candidate->info.user.doc_comment)) {
        	prepared->info.user.doc_comment = zend_string_new(candidate->info.user.doc_comment);
    	} else {
        	prepared->info.user.doc_comment = NULL;
    	}
	
	/* always copy filename so that errors make sense (and don't corrupt heap) */
	//if (prepared->info.user.filename)
	//	prepared->info.user.filename = zend_string_new(candidate->info.user.filename);
	
	/* copy constants */
	//zend_hash_copy(
	//    &prepared->constants_table, &candidate->constants_table, (copy_ctor_func_t) zval_add_ref);

    return prepared;
}

static inline int pthreads_prepared_entry_function_prepare(zval *bucket, int argc, va_list argv, zend_hash_key *key) {
	zend_function *function = (zend_function*) Z_PTR_P(bucket);
	PTHREAD thread = va_arg(argv, PTHREAD);	
	zend_class_entry *prepared = va_arg(argv, zend_class_entry*);
	zend_class_entry *candidate = va_arg(argv, zend_class_entry*);
	zend_class_entry *scope = function->common.scope;
	
	if (function->type == ZEND_USER_FUNCTION) {
		if (scope == candidate) {
			function->common.scope = prepared;
		} else {
			function->common.scope = pthreads_prepared_entry
				(thread, function->common.scope);
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
}

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(PTHREAD thread, zend_class_entry *candidate) {
	zend_class_entry *prepared = NULL;
	zend_string *lookup = NULL;

	if (!candidate) {
		return NULL;
	}
	
	/* use lowercase name for registering new classes and lookups */
	lookup = zend_string_tolower(candidate->name);

	/* perform lookup for existing class */
	if ((prepared = zend_hash_find_ptr(EG(class_table), lookup))) {
	    	zend_string_release(lookup);
		return prepared;
	}

	/* create a new user class for this context */
	if (!(prepared = pthreads_copy_entry(thread, candidate))) {
		zend_string_release(lookup);
		return NULL;
	}

	zend_hash_apply_with_arguments(
		&prepared->function_table, 
		pthreads_prepared_entry_function_prepare, 
		3, thread, prepared, candidate);	
	
    	/* update class table */
	zend_hash_update_ptr(EG(class_table), lookup, prepared);

	/* release lowercase'd name */
	zend_string_release(lookup);

	return prepared;
} /* }}} */

static inline zend_bool pthreads_constant_exists(zend_string *name) {
    int retval = 1;
    zend_string *lookup;

    if (!zend_hash_exists(EG(zend_constants), name)) {
        lookup = zend_string_tolower(name);
        retval = zend_hash_exists(EG(zend_constants), lookup);
        zend_string_release(lookup);
    }

    return retval;
}

/* {{{ prepares the current context to execute the referenced thread */
int pthreads_prepare(PTHREAD thread){
	//TSRMLS_CACHE_UPDATE();

	/* inherit ini entries from parent ... */
	if (thread->options & PTHREADS_INHERIT_INI) {
		zend_ini_entry *entry[2];
		zend_string *name;
		HashTable *table[2] = {PTHREADS_EG(thread->cls, ini_directives), EG(ini_directives)};
		
		ZEND_HASH_FOREACH_STR_KEY_PTR(table[0], name, entry[0]) {
			if ((entry[1] = zend_hash_find_ptr(table[1], name))) {
				if (entry[0]->value && entry[1]->value) {
					if (memcmp(ZSTR_VAL(entry[0]->value), ZSTR_VAL(entry[1]->value), ZSTR_LEN(entry[0]->value)) != SUCCESS) {
						zend_string *copied;
						zend_bool resmod = entry[1]->modifiable;
	
						if (!EG(modified_ini_directives)) {
							ALLOC_HASHTABLE(EG(modified_ini_directives));
							zend_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
						}

						if (!entry[1]->modified) {
							entry[1]->orig_value = entry[1]->value;
							entry[1]->orig_modifiable = entry[1]->modifiable;
							entry[1]->modified = 1;
							zend_hash_add_ptr(EG(modified_ini_directives), name, entry[1]);
						}

						entry[1]->modifiable = 1;
						entry[1]->on_modify(entry[1], entry[0]->value, entry[1]->mh_arg1, entry[1]->mh_arg2, entry[1]->mh_arg3, ZEND_INI_SYSTEM);
						if (entry[1]->modified && entry[1]->orig_value != entry[1]->value) {
							zend_string_release(entry[1]->value);
						}
						entry[1]->value = zend_string_new(entry[0]->value);					
						entry[1]->modifiable = resmod;
					}
				}
			}
		} ZEND_HASH_FOREACH_END();
	}
	
	/* copy constants */
	if (thread->options & PTHREADS_INHERIT_CONSTANTS) {
		zend_constant *zconstant;
		zend_string *name;
		
		ZEND_HASH_FOREACH_STR_KEY_PTR(PTHREADS_EG(thread->cls, zend_constants), name, zconstant) {
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
							    Z_STR(constant.value)=zend_string_new(Z_STR(zconstant->value));
						    } break;
					    }
				
					    zend_register_constant(&constant);
				    }
			    }
			}
		} ZEND_HASH_FOREACH_END();
	}

	/* inherit function table from parent ... */
	if (thread->options & PTHREADS_INHERIT_FUNCTIONS) {
		
		zend_hash_merge(CG(function_table), PTHREADS_CG(thread->cls, function_table),
			pthreads_copy_function_ctor, 0);

	}

	/* inherit class table from parent ... */
	if (thread->options & PTHREADS_INHERIT_CLASSES) {
		zend_class_entry *entry, *prepared;
		zend_string *name;
        	HashTable inherited;

        	zend_hash_init(&inherited, zend_hash_num_elements(PTHREADS_CG(thread->cls, class_table)), NULL, NULL, 0);

		ZEND_HASH_FOREACH_STR_KEY_PTR(PTHREADS_CG(thread->cls, class_table), name, entry) {
			if (entry->type == ZEND_USER_CLASS) {
				if (!zend_hash_exists(PTHREADS_CG(thread->tls, class_table), name)) {
					prepared = pthreads_prepared_entry(thread, entry);
					if (!prepared)
						continue;
					zend_hash_next_index_insert_ptr(&inherited, prepared);
				}
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
	}
	
	/* merge included files with parent */
	if (thread->options & PTHREADS_INHERIT_INCLUDES){
		zend_string *file;
		ZEND_HASH_FOREACH_STR_KEY(&PTHREADS_EG(thread->cls, included_files), file) {
			zend_hash_add_empty_element(&EG(included_files), file);
		} ZEND_HASH_FOREACH_END();
	}
	
	/* inherit globals, I don't like this ... */
	if (thread->options & PTHREADS_ALLOW_GLOBALS) {
		zval *symbol = NULL;
		zend_string *name;
		
		ZEND_HASH_FOREACH_STR_KEY_VAL(&PTHREADS_EG(thread->cls, symbol_table), name, symbol) {
			zval separated;
			if (pthreads_store_separate(symbol, &separated, 1) != SUCCESS) {
				zval_dtor(&separated);
				continue;
			}

			zend_hash_update(&PTHREADS_EG(thread->tls, symbol_table), name, &separated);
			if (Z_REFCOUNTED(separated))
				Z_ADDREF(separated);	
		} ZEND_HASH_FOREACH_END();
	}

	/**
	* set exception handler from parent executor
	**/
	if (Z_TYPE(PTHREADS_EG(thread->cls, user_exception_handler)) != IS_UNDEF) {
		pthreads_store_separate(
			&PTHREADS_EG(thread->cls, user_exception_handler), 
			&PTHREADS_EG(thread->tls, user_exception_handler), 1);
	}

	/* set sensible resource destructor */
	if (!PTHREADS_G(default_resource_dtor))
		PTHREADS_G(default_resource_dtor)=(EG(regular_list).pDestructor);
	EG(regular_list).pDestructor =  (dtor_func_t) pthreads_prepared_resource_dtor;	
	
	return SUCCESS;
} /* }}} */

/* {{{ copy property info dummy ctor */
static void pthreads_preparation_property_info_dummy_ctor(zval *bucket) {} /* }}} */

/* {{{ destroy property info dummy dtor */
static void pthreads_preparation_property_info_dummy_dtor(zval *bucket) {} /* }}} */

/* {{{ trail alias copy for 5.4 */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(PTHREAD thread, zend_trait_alias *alias) {
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

/* {{{ trait precendence for 5.4+ */
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(PTHREAD thread, zend_trait_precedence *precedence) {
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

/* {{{ method reference copy for traits */
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(PTHREAD thread, zend_trait_method_reference *reference) {
	zend_trait_method_reference *copy = ecalloc(1, sizeof(zend_trait_method_reference));
	if (copy) {
		copy->method_name = zend_string_new(reference->method_name);
		if (copy->class_name) {
			copy->class_name = zend_string_new(reference->class_name);
		}
		copy->ce = pthreads_prepared_entry(
			thread, (zend_class_entry*) reference->ce);
	}
	return copy;
} /* }}} */

/* {{{ fix method scope for prepared entries, enabling self:: and parent:: to work */
/*static int pthreads_apply_method_scope(zend_function *function) {
	if (function && function->type == ZEND_USER_FUNCTION) {
		zend_op_array *ops = &function->op_array;
		
		if (ops->scope) {
			zend_class_entry *search = NULL;
			if ((search = zend_hash_index_find_ptr(PTHREADS_ZG(resolve), (zend_ulong) ops->scope))) {
				ops->scope = search;
			} else ops->scope = NULL;
		} else ops->scope = NULL;
		
		function->common.prototype = NULL;
	}
	
	return ZEND_HASH_APPLY_KEEP;
} */ /* }}} */

/* {{{ fix scope for prepared entry properties, enabling private members in foreign objects to work */
static int pthreads_apply_property_scope(zend_property_info *info) {
	if (info->ce) {
		zend_class_entry *search = NULL;
		if ((search = zend_hash_index_find_ptr(&PTHREADS_ZG(resolve), (zend_ulong) info->ce))) {
			info->ce = search;
		} else info->ce = NULL;
	}
	
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ destroy a resource, if we created it ( ie. it is not being kept by another thread ) */
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


