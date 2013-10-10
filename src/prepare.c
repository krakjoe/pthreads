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

/* {{{ prepared property info ctor */
static void pthreads_preparation_property_info_copy_ctor(zend_property_info *pi); /* }}} */
/* {{{ prepared property info dtor */
static void pthreads_preparation_property_info_copy_dtor(zend_property_info *pi); /* }}} */

/* {{{ prepared property info dummy ctor */
static void pthreads_preparation_property_info_dummy_ctor(zend_property_info *pi); /* }}} */
/* {{{ prepared property info dummy dtor */
static void pthreads_preparation_property_info_dummy_dtor(zend_property_info *pi); /* }}} */

#if PHP_VERSION_ID < 50400
/* {{{ default property ctor for 5.3 */
static void pthreads_preparation_default_properties_ctor(zval **property); /* }}} */
/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property); /* }}} */
#else
/* {{{ trail alias copy for 5.4 */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(PTHREAD thread, zend_trait_alias *alias TSRMLS_DC); /* }}} */
/* {{{ trait precendence for 5.4 */
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(PTHREAD thread, zend_trait_precedence *precedence TSRMLS_DC); /* }}} */
/* {{{ method reference copy for traits */
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(PTHREAD thread, zend_trait_method_reference *reference TSRMLS_DC); /* }}} */
#endif

/* {{{ fix the scope of methods such that inheritance works correctly */
static int pthreads_apply_method_prototype(zend_op_array *ops, zend_class_entry **ce TSRMLS_DC); /* }}} */

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_method_scope(zend_op_array *ops, PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_property_scope(zend_property_info *info, PTHREAD thread TSRMLS_DC); /* }}} */

/* {{{ prepared resource destructor */
static void pthreads_prepared_resource_dtor(zend_rsrc_list_entry *entry); /* }}} */

static zend_class_entry* pthreads_copy_entry(PTHREAD thread, zend_class_entry *candidate TSRMLS_DC) {
    zval *tz;
	zend_function *tf;
	zend_property_info *ti;
    zend_class_entry *prepared = NULL;
    
	/* create a new user class for this context */
	prepared = (zend_class_entry*) emalloc(sizeof(zend_class_entry));
	prepared->name = estrndup(candidate->name, candidate->name_length);
	prepared->name_length = candidate->name_length;
	prepared->type = candidate->type;
	
	/* initialize class data */
	zend_initialize_class_data(prepared, 1 TSRMLS_CC);
	
	/* set ce flags (reset by initialize) */
	prepared->ce_flags = candidate->ce_flags;
	
	/* set parent */
	if (candidate->parent)
	    prepared->parent = pthreads_prepared_entry(thread, candidate->parent TSRMLS_CC);
	
	/* declare interfaces */
	if (candidate->num_interfaces) {
		uint interface;
		prepared->interfaces = emalloc(sizeof(zend_class_entry*) * candidate->num_interfaces);
		for(interface=0; interface<candidate->num_interfaces; interface++)
			prepared->interfaces[interface] = pthreads_prepared_entry(thread, candidate->interfaces[interface] TSRMLS_CC);
		prepared->num_interfaces = candidate->num_interfaces;
	} else prepared->num_interfaces = 0;

#if PHP_VERSION_ID > 50399
	/* traits */
	if (candidate->num_traits) {
		uint trait;
		prepared->traits = emalloc(sizeof(zend_class_entry*) * candidate->num_traits);
		for (trait=0; trait<candidate->num_traits; trait++)
			prepared->traits[trait] = pthreads_prepared_entry(thread, candidate->traits[trait] TSRMLS_CC);
		prepared->num_traits = candidate->num_traits;
		
		if (candidate->trait_aliases) {
			size_t alias = 0;
			prepared->trait_aliases = emalloc(sizeof(zend_trait_alias*)*prepared->num_traits);
			while (candidate->trait_aliases[alias]) {
				prepared->trait_aliases[alias] = pthreads_preparation_copy_trait_alias(
					thread, candidate->trait_aliases[alias] TSRMLS_CC
				);
				alias++;
			}
			prepared->trait_aliases[alias]=NULL;
		} else prepared->trait_aliases = NULL;
		
		if (candidate->trait_precedences) {
			size_t precedence = 0;
			prepared->trait_precedences = emalloc(sizeof(zend_trait_precedence*)*prepared->num_traits);
			while (candidate->trait_precedences[precedence]) {
				prepared->trait_precedences[precedence] = pthreads_preparation_copy_trait_precedence(
					thread, candidate->trait_precedences[precedence] TSRMLS_CC
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
#endif

	/* copy uternals ! */
	{
		zend_uint umethod = 0;
		void *usources[18] = {
			candidate->constructor,
			candidate->destructor,
			candidate->clone,

			candidate->__get,
			candidate->__set,
			candidate->__unset,
			candidate->__isset,
			candidate->__call,
			candidate->__callstatic,
			candidate->__tostring,

			candidate->serialize_func,
			candidate->unserialize_func,

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
					/* user internals, I call them uternals */
					case 0: zend_hash_update(&prepared->function_table, "__construct", sizeof("__construct"), &candidate->constructor, sizeof(zend_function), (void**) &prepared->constructor); break;
					case 1: zend_hash_update(&prepared->function_table, "__destruct", sizeof("__destruct"), &candidate->destructor, sizeof(zend_function), (void**) &prepared->destructor); break;
					case 2: zend_hash_update(&prepared->function_table, "__clone", sizeof("__clone"), &candidate->clone, sizeof(zend_function), (void**) &prepared->clone); break;
					case 3: zend_hash_update(&prepared->function_table, "__get", sizeof("__get"), &candidate->__get, sizeof(zend_function), (void**) &prepared->__get); break;
					case 4: zend_hash_update(&prepared->function_table, "__set", sizeof("__set"), &candidate->__set, sizeof(zend_function), (void**) &prepared->__set); break;
					case 5: zend_hash_update(&prepared->function_table, "__unset", sizeof("__unset"), &candidate->__unset, sizeof(zend_function), (void**) &prepared->__unset); break;
					case 6: zend_hash_update(&prepared->function_table, "__isset", sizeof("__isset"), &candidate->__isset, sizeof(zend_function), (void**) &prepared->__isset); break;
					case 7: zend_hash_update(&prepared->function_table, "__call", sizeof("__call"), &candidate->__call, sizeof(zend_function), (void**) &prepared->__call); break;
					case 8: zend_hash_update(&prepared->function_table, "__callstatic", sizeof("__callstatic"), &candidate->__callstatic, sizeof(zend_function), (void**) &prepared->__callstatic); break;
					case 9: zend_hash_update(&prepared->function_table, "__tostring", sizeof("__tostring"), &candidate->__tostring, sizeof(zend_function), (void**) &prepared->__tostring); break;
					
					case 10: zend_hash_update(&prepared->function_table, "__serialize", sizeof("__serialize"), &candidate->serialize_func, sizeof(zend_function), (void**) &prepared->serialize_func); break;
					case 11: zend_hash_update(&prepared->function_table, "__unserialize", sizeof("__unserialize"), &candidate->unserialize_func, sizeof(zend_function), (void**) &prepared->unserialize_func); break;
					/* handlers */
					case 12: prepared->create_object = candidate->create_object; break;
					case 13: prepared->serialize = candidate->serialize; break;
					case 14: prepared->unserialize = candidate->unserialize; break;
					case 15: {
						prepared->get_iterator = candidate->get_iterator;
						prepared->iterator_funcs = candidate->iterator_funcs;
					} break;
					case 16: prepared->interface_gets_implemented = candidate->interface_gets_implemented; break;
					case 17: prepared->get_static_method = candidate->get_static_method; break;
				}
			}
		} while(++umethod < 18);
	}
	
	/* copy function table */
	zend_hash_copy(&prepared->function_table, &candidate->function_table, (copy_ctor_func_t) function_add_ref, &tf, sizeof(zend_function));
	
	/* copy property info structures */
	if ((thread->options & PTHREADS_INHERIT_COMMENTS)) {
	    zend_hash_copy(
	        &prepared->properties_info,
	        &candidate->properties_info, (copy_ctor_func_t) pthreads_preparation_property_info_copy_ctor, &ti, sizeof(zend_property_info));
	    prepared->properties_info.pDestructor = (dtor_func_t) pthreads_preparation_property_info_copy_dtor;
	} else {
	    zend_hash_copy(
	        &prepared->properties_info,
	        &candidate->properties_info, (copy_ctor_func_t) pthreads_preparation_property_info_dummy_ctor, &ti, sizeof(zend_property_info));
	    prepared->properties_info.pDestructor = (dtor_func_t) pthreads_preparation_property_info_dummy_dtor;
	}
	
	
	/* copy statics and defaults */
	{
#if PHP_VERSION_ID < 50400
		{
			HashPosition position;
			zval **property;
			
			for (zend_hash_internal_pointer_reset_ex(&candidate->default_properties, &position);
				zend_hash_get_current_data_ex(&candidate->default_properties, (void**) &property, &position)==SUCCESS;
				zend_hash_move_forward_ex(&candidate->default_properties, &position)) {

				char *n;
				ulong i;
				uint l;
				zval *separated;

				if (zend_hash_get_current_key_ex(&candidate->default_properties, &n, &l, &i, 0, &position)) {
					if (pthreads_store_separate(*property, &separated, 1, 1 TSRMLS_CC)==SUCCESS) {
						zend_hash_update(&prepared->default_properties, n, l, (void**) &separated, sizeof(zval*), NULL);
					}
				}
			}

			for (zend_hash_internal_pointer_reset_ex(&candidate->default_static_members, &position);
				zend_hash_get_current_data_ex(&candidate->default_static_members, (void**) &property, &position)==SUCCESS;
				zend_hash_move_forward_ex(&candidate->default_static_members, &position)) {

				char *n;
				ulong i;
				uint l;
				zval *separated;

				if (zend_hash_get_current_key_ex(&candidate->default_static_members, &n, &l, &i, 0, &position)) {
					if (pthreads_store_separate(*property, &separated, 1, 0 TSRMLS_CC)==SUCCESS) {
						zend_hash_update(&prepared->default_static_members, n, l, (void**) &separated, sizeof(zval*), NULL);
					}
				}
			}
		}

		/** copy comments where required **/
		if ((thread->options & PTHREADS_INHERIT_COMMENTS) &&
		    (prepared->doc_comment)) {
		    prepared->doc_comment_len = candidate->doc_comment_len;
	        prepared->doc_comment = estrndup(
	            candidate->doc_comment, candidate->doc_comment_len
	        );
	    } else {
	        prepared->doc_comment = NULL;
		    prepared->doc_comment_len = 0;
	    }
		
#else
		if (candidate->default_properties_count) {
			int i;
			prepared->default_properties_table = emalloc(
				sizeof(zval*) * candidate->default_properties_count
			);
			for (i=0; i<candidate->default_properties_count; i++) {
				if (candidate->default_properties_table[i]) {
					/* we use real separation for a reason */
					pthreads_store_separate(
						candidate->default_properties_table[i],
						&prepared->default_properties_table[i],
						1, 1 TSRMLS_CC
					);
				} else prepared->default_properties_table[i] = NULL;
			}
			prepared->default_properties_count = candidate->default_properties_count;
		} else prepared->default_properties_count = 0;
		
		if (candidate->default_static_members_count) {
			int i;
			prepared->default_static_members_table = emalloc(
				sizeof(zval*) * candidate->default_static_members_count
			);
			prepared->default_static_members_count = candidate->default_static_members_count;
			for (i=0; i<prepared->default_static_members_count; i++) {
				if (candidate->default_static_members_table[i]) {
					/* we use real separation for a reason */
					pthreads_store_separate(
						candidate->default_static_members_table[i],
						&prepared->default_static_members_table[i],
						1, 0 TSRMLS_CC
					);
				} else prepared->default_static_members_table[i]=NULL;
			}
			prepared->static_members_table = prepared->default_static_members_table;
		} else prepared->default_static_members_count = 0;
		
		/* copy user info struct */
		memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));
		
		/* copy comments where required */
		if ((thread->options & PTHREADS_INHERIT_COMMENTS) &&
		   (candidate->info.user.doc_comment)) {
	        prepared->info.user.doc_comment = estrndup(
	            prepared->info.user.doc_comment, prepared->info.user.doc_comment_len);
	    } else {
	        prepared->info.user.doc_comment = NULL;
		    prepared->info.user.doc_comment_len = 0;
	    }
#endif
	}
	
	/* copy constants */
	zend_hash_copy(
	    &prepared->constants_table, &candidate->constants_table, (copy_ctor_func_t) zval_add_ref, &tz, sizeof(zval*));

    return prepared;
}

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(PTHREAD thread, zend_class_entry *candidate TSRMLS_DC) {
	zend_class_entry *prepared = NULL, **searched = NULL;
	
	if (candidate) {
        char *lower = pthreads_global_string(
            (char*) candidate->name, candidate->name_length, 1 TSRMLS_CC);
        
		if (lower != NULL) {
			/* perform lookup for existing class */
			if (zend_hash_find(CG(class_table), lower, candidate->name_length+1, (void**)&searched)!=SUCCESS) {
			
			    /* create a new user class for this context */
	            prepared = pthreads_copy_entry(
	                thread, candidate TSRMLS_CC);
	            
			    /* update class table */
                zend_hash_update(   
                    CG(class_table), lower, prepared->name_length+1, &prepared, sizeof(zend_class_entry*), (void**)&searched);
                            
			} else prepared = *searched;

		} else zend_error(E_ERROR, "pthreads has detected a memory error while attempting to prepare %s for execution in %s %lu", candidate->name, thread->std.ce->name, thread->tid);
	}
	return prepared;
} /* }}} */

static inline zend_bool pthreads_constant_exists(char *name, zend_uint name_len TSRMLS_DC) {
    int retval = 1;
    char *lookup_name;

    if (!zend_hash_exists(EG(zend_constants), name, name_len+1)) {
        lookup_name = zend_str_tolower_dup(name, name_len);

        retval = zend_hash_exists(
            EG(zend_constants), lookup_name, name_len+1);
            
        efree(lookup_name);
    }

    return retval;
}

/* {{{ prepares the current context to execute the referenced thread */
void pthreads_prepare(PTHREAD thread TSRMLS_DC){
	HashPosition position;
	
	/* inherit ini entries from parent ... */
	if (thread->options & PTHREADS_INHERIT_INI) {
		zend_ini_entry *entry[2];
		HashTable *table[2] = {PTHREADS_EG(thread->cls, ini_directives), EG(ini_directives)};
        
		for(zend_hash_internal_pointer_reset_ex(table[0], &position);
			zend_hash_get_current_data_ex(table[0], (void**) &entry[0], &position)==SUCCESS;
			zend_hash_move_forward_ex(table[0], &position)) {
			char *setting;
			uint slength;
			ulong idx;

			if ((zend_hash_get_current_key_ex(table[0], &setting, &slength, &idx, 0, &position)==HASH_KEY_IS_STRING) &&
				(zend_hash_find(table[1], setting, slength, (void**) &entry[1])==SUCCESS)) {
				if (((entry[0]->value && entry[1]->value) && 
				    ((memcmp(entry[0]->value, entry[1]->value, entry[0]->value_length) != SUCCESS)))) {
				    char *duplicate = NULL;
				    zend_bool resmod = entry[1]->modifiable;

				    if (!EG(modified_ini_directives)) {
				        ALLOC_HASHTABLE(EG(modified_ini_directives));
				        zend_hash_init(EG(modified_ini_directives), 8, NULL, NULL, 0);
				    }
				    
				    if (!entry[1]->modified) {
				        entry[1]->orig_value = entry[1]->value;
				        entry[1]->orig_value_length = entry[1]->value_length;
				        entry[1]->orig_modifiable = entry[1]->modifiable;
				        entry[1]->modified = 1;
				        zend_hash_add(
				            EG(modified_ini_directives), setting, slength, &entry[1], sizeof(zend_ini_entry*), NULL);
				    }
				    
				    duplicate = estrndup(entry[0]->value, entry[0]->value_length);
				    
				    entry[1]->modifiable = ZEND_INI_SYSTEM;
				    if (!entry[1]->on_modify ||
				        entry[1]->on_modify(    
				            entry[1], duplicate, entry[0]->value_length, 
				            entry[1]->mh_arg1, entry[1]->mh_arg2, entry[1]->mh_arg3, ZEND_INI_STAGE_ACTIVATE TSRMLS_CC) == SUCCESS) {
				        if (entry[1]->modified && (entry[1]->orig_value != entry[1]->value)) {
				            efree(entry[1]->value);
				        }
				        entry[1]->value = duplicate;
				        entry[1]->value_length = entry[0]->value_length;
				    } else {
				        efree(duplicate);
				    }
				    entry[1]->modifiable = resmod;
				}
			}
		}
	}
	
	/* copy constants */
	if (thread->options & PTHREADS_INHERIT_CONSTANTS) {
		zend_constant *zconstant;
		HashTable *table[2] = {PTHREADS_EG(thread->cls, zend_constants), EG(zend_constants)};
		
		for(zend_hash_internal_pointer_reset_ex(table[0], &position);
			zend_hash_get_current_data_ex(table[0], (void**) &zconstant, &position)==SUCCESS;
			zend_hash_move_forward_ex(table[0], &position)) {
			if (strncmp(zconstant->name, "STDIN", zconstant->name_len-1)==0||
				strncmp(zconstant->name, "STDOUT", zconstant->name_len-1)==0||
				strncmp(zconstant->name, "STDERR", zconstant->name_len-1)==0){
				continue;
			} else {
				zend_constant constant;

				if (!pthreads_constant_exists(zconstant->name, zconstant->name_len-1 TSRMLS_CC)) {

					constant.flags = zconstant->flags;
					constant.module_number = zconstant->module_number;
					constant.name = zend_strndup(zconstant->name, zconstant->name_len);
					constant.name_len = zconstant->name_len;
				
					switch((Z_TYPE(constant.value)=Z_TYPE(zconstant->value))) {
						case IS_BOOL:
						case IS_LONG: {
							Z_LVAL(constant.value)=Z_LVAL(zconstant->value);
						} break;
						case IS_DOUBLE: Z_DVAL(constant.value)=Z_DVAL(zconstant->value); break;
						case IS_STRING: {
							Z_STRVAL(constant.value)=estrndup(Z_STRVAL(zconstant->value), Z_STRLEN(zconstant->value)); 
							Z_STRLEN(constant.value)=Z_STRLEN(zconstant->value);
						} break;
					}
				
					zend_register_constant(&constant TSRMLS_CC);
				}
			}
		}
	}
	
	/* inherit function table from parent ... */
	if (thread->options & PTHREADS_INHERIT_FUNCTIONS) {
		zend_function function;
		zend_hash_merge(
			EG(function_table),
			PTHREADS_EG(thread->cls, function_table), 
			(copy_ctor_func_t) function_add_ref, 
			&function, sizeof(zend_function), 0
		);
	}
	
	/* inherit class table from parent ... */
	if (thread->options & PTHREADS_INHERIT_CLASSES) {
		zend_class_entry **entry;
		HashTable *table[2] = {PTHREADS_CG(thread->cls, class_table), CG(class_table)};
        HashTable store;
        
        zend_hash_init(&store, zend_hash_num_elements(table[1]), NULL, NULL, 0);
        
		for(zend_hash_internal_pointer_reset_ex(table[0], &position);
			zend_hash_get_current_data_ex(table[0], (void**) &entry, &position)==SUCCESS;
			zend_hash_move_forward_ex(table[0], &position)) {
			if ((*entry)->type == ZEND_USER_CLASS) {
			    char *lcname;
			    uint lcnamel;
			    ulong idx;
			    zend_class_entry *prepared;
			    zend_class_entry **exists;
			
			    if (zend_hash_get_current_key_ex(table[0], &lcname, &lcnamel, &idx, 0, &position)==HASH_KEY_IS_STRING) {
				    if (zend_hash_find(table[1], lcname, lcnamel, (void**)&exists) != SUCCESS){
					    if ((prepared=pthreads_prepared_entry(thread, *entry TSRMLS_CC))==NULL) {
						    zend_error(
							    E_ERROR, "pthreads detected failure while preparing %s in %s", (*entry)->name, thread->std.ce->name, thread->tid
						    );
						    return;
					    }
					
					    zend_hash_next_index_insert(&store, (void**) &prepared, sizeof(zend_class_entry*), NULL);
				    }
			    }
			}
		}
		
		for(zend_hash_internal_pointer_reset_ex(&store, &position);
			zend_hash_get_current_data_ex(&store, (void**) &entry, &position)==SUCCESS;
			zend_hash_move_forward_ex(&store, &position)) {
			
			zend_hash_apply_with_argument(
			    &(*entry)->properties_info, (apply_func_arg_t) pthreads_apply_property_scope, (void*) thread TSRMLS_CC);
			zend_hash_apply_with_argument(
			    &(*entry)->function_table, (apply_func_arg_t) pthreads_apply_method_scope, (void*) thread TSRMLS_CC);
			zend_hash_apply_with_argument(
			    &(*entry)->function_table, (apply_func_arg_t) pthreads_apply_method_prototype, (void*) entry TSRMLS_CC);
		}
		
		zend_hash_destroy(&store);     
	}
	
	/* merge included files with parent */
	if (thread->options & PTHREADS_INHERIT_INCLUDES){
		int included;
		zend_hash_merge(
			&EG(included_files), 
			&PTHREADS_EG(thread->cls, included_files),
			NULL, &included, sizeof(int), 0
		);
	}

	/* set sensible resource destructor */
	if (!PTHREADS_G(default_resource_dtor))
		PTHREADS_G(default_resource_dtor)=(EG(regular_list).pDestructor);
	EG(regular_list).pDestructor =  (dtor_func_t) pthreads_prepared_resource_dtor;	
} /* }}} */

/* {{{ property info ctor */
static void pthreads_preparation_property_info_copy_ctor(zend_property_info *pi) {
    pi->name = estrndup(pi->name, pi->name_length);    
    
    if (pi->doc_comment)
        pi->doc_comment = estrndup(pi->doc_comment, pi->doc_comment_len);
} /* }}} */

/* {{{ property info dtor */
static void pthreads_preparation_property_info_copy_dtor(zend_property_info *pi) {
    str_efree((char*)pi->name);    
        
    if (pi->doc_comment)
        efree((char*)pi->doc_comment);
} /* }}} */

/* {{{ copy property info dummy ctor */
static void pthreads_preparation_property_info_dummy_ctor(zend_property_info *pi) {} /* }}} */

/* {{{ destroy property info dummy dtor */
static void pthreads_preparation_property_info_dummy_dtor(zend_property_info *pi) {} /* }}} */

#if PHP_VERSION_ID < 50400
/* {{{ default property ctor for 5.3 */
static void pthreads_preparation_default_properties_ctor(zval **property) {

} /* }}} */

/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property) {
	zval_dtor(property);
} /* }}} */
#else
/* {{{ trail alias copy for 5.4 */
static zend_trait_alias * pthreads_preparation_copy_trait_alias(PTHREAD thread, zend_trait_alias *alias TSRMLS_DC) {
	zend_trait_alias *copy = emalloc(sizeof(zend_trait_alias));
	if (copy) {
		copy->trait_method = pthreads_preparation_copy_trait_method_reference(thread, alias->trait_method TSRMLS_CC);
		copy->alias = estrndup(alias->alias, alias->alias_len);
		copy->alias_len = alias->alias_len;
		copy->modifiers = alias->modifiers;
#if PHP_VERSION_ID < 50500
		copy->function = alias->function;
#endif
	}
	return copy;
} /* }}} */

/* {{{ trait precendence for 5.4+ */
static zend_trait_precedence * pthreads_preparation_copy_trait_precedence(PTHREAD thread, zend_trait_precedence *precedence TSRMLS_DC) {
	zend_trait_precedence *copy = emalloc(sizeof(zend_trait_precedence));
	if (copy) {
		copy->trait_method = pthreads_preparation_copy_trait_method_reference(thread, precedence->trait_method TSRMLS_CC);
		if (precedence->exclude_from_classes) {
			size_t exclude = 0;
			copy->exclude_from_classes = emalloc(sizeof(zend_class_entry**));
			while (precedence->exclude_from_classes[exclude]) {
				copy->exclude_from_classes[exclude] = pthreads_prepared_entry(
					thread, precedence->exclude_from_classes[exclude] TSRMLS_CC
				);
				exclude++;
			}
			precedence->exclude_from_classes[exclude]=NULL;
		}
#if PHP_VERSION_ID < 50500
		copy->function = precedence->function;
#endif
	}
	return copy;
} /* }}} */

/* {{{ method reference copy for traits */
static  zend_trait_method_reference * pthreads_preparation_copy_trait_method_reference(PTHREAD thread, zend_trait_method_reference *reference TSRMLS_DC) {
	zend_trait_method_reference *copy = emalloc(sizeof(zend_trait_method_reference));
	if (copy) {
		copy->mname_len = reference->mname_len;
		copy->method_name = estrndup(
			reference->method_name, 
			copy->mname_len
		);
		copy->cname_len = reference->cname_len;
		copy->class_name = estrndup(
			reference->class_name, 
			copy->cname_len
		);
		
		copy->ce = pthreads_prepared_entry(
			thread, reference->ce TSRMLS_CC
		);
	}
	return copy;
} /* }}} */
#endif

/* {{{ fix method prototype for prepared entries, enabling inheritance to function correctly */
static int pthreads_apply_method_prototype(zend_op_array *ops, zend_class_entry **ce TSRMLS_DC) {
	if (ops && ce) {
	    if (ops->prototype && ops->line_end) {
	        zend_class_entry **scope;
	        zend_function *prototype;
	        
	        if ((*ce)->parent) {
	            if (memcmp(ops->prototype->common.scope->name, (*ce)->parent->name, (*ce)->parent->name_length+1) == SUCCESS) {
	                if ((zend_hash_find(
	                        &(*ce)->parent->function_table, 
	                        ops->prototype->common.function_name,
	                        strlen(ops->prototype->common.function_name)+1,
	                        (void**) &prototype) == SUCCESS)) {
	                    ops->prototype = prototype;
	                } else ops->prototype = NULL;
	            }
	        } else if (memcmp(ops->prototype->common.scope->name, (*ce)->name, (*ce)->name_length+1) == SUCCESS) {
	            if ((zend_hash_find(
	                    &(*ce)->function_table, 
	                    ops->prototype->common.function_name,
	                    strlen(ops->prototype->common.function_name)+1,
	                    (void**) &prototype) == SUCCESS)) {
	                ops->prototype = prototype;
	            } else ops->prototype = NULL;
	        }
	    } else {
	        ops->prototype = NULL;
	    }
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ fix method scope for prepared entries, enabling self:: and parent:: to work */
static int pthreads_apply_method_scope(zend_op_array *ops, PTHREAD thread TSRMLS_DC) {
	if (thread && ops) {
	    if (ops->scope) {
	        ops->scope = pthreads_prepared_entry(thread, ops->scope TSRMLS_CC);
	    }
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ fix scope for prepared entry properties, enabling private members in foreign objects to work */
static int pthreads_apply_property_scope(zend_property_info *info, PTHREAD thread TSRMLS_DC) {
	if (thread && info) {
	    info->ce = pthreads_prepared_entry(thread, info->ce TSRMLS_CC);
	}
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ destroy a resource, if we created it ( ie. it is not being kept by another thread ) */
static void pthreads_prepared_resource_dtor(zend_rsrc_list_entry *entry) {
	TSRMLS_FETCH();
	if (EG(This)) {
		zend_try {
			PTHREAD object = PTHREADS_ZG(pointer);
			if (object) {
				if (object->resources) {
					if (!pthreads_resources_kept(object->resources, entry TSRMLS_CC)){
						if (PTHREADS_G(default_resource_dtor))
							PTHREADS_G(default_resource_dtor)(entry);
					}
				}
			}
		} zend_catch {
			zend_error(E_WARNING, "pthreads has detected failure while cleaning resources and is likely to fail");
		} zend_end_try();
	}
} /* }}} */

#endif


