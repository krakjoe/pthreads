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

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_method_scope(zend_function *function TSRMLS_DC); /* }}} */

/* {{{ fix the scope of methods such that self:: and parent:: work everywhere */
static int pthreads_apply_property_scope(zend_property_info *info TSRMLS_DC); /* }}} */

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
	
	zend_hash_index_update(
		PTHREADS_ZG(resolve),
		(zend_ulong) candidate, 
		(void**)&prepared, sizeof(zend_class_entry*), NULL);
	
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

			while (candidate->trait_aliases[alias]) {
				alias++;
			}
			prepared->trait_aliases = emalloc(sizeof(zend_trait_alias*) * (alias+1));
			alias = 0;

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

			while (candidate->trait_precedences[precedence]) {
				precedence++;
			}
			prepared->trait_precedences = emalloc(sizeof(zend_trait_precedence*) * (precedence+1));
			precedence = 0;

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
	zend_hash_copy(&prepared->function_table, &candidate->function_table, (copy_ctor_func_t) pthreads_copy_function, &tf, sizeof(zend_function));
	
	{
	    zend_function *func;
	    char *lcname = NULL;
	    
	    if (!prepared->constructor && zend_hash_num_elements(&prepared->function_table)) {
	        lcname = zend_str_tolower_dup(prepared->name, prepared->name_length);
	        if (zend_hash_find(&prepared->function_table, lcname, prepared->name_length+1, (void**)&func) == SUCCESS) {
	            prepared->constructor = func;
	        } else if (zend_hash_find(&prepared->function_table, "__construct", sizeof("__construct"), (void**)&func) == SUCCESS) {
	            prepared->constructor = func;
	        }
	        efree(lcname);
	    }
	    
#define FIND_AND_SET(f, n) do {\
    if (!prepared->f && zend_hash_num_elements(&prepared->function_table)) { \
        if (zend_hash_find(&prepared->function_table, n, sizeof(n), (void**)&func) == SUCCESS) { \
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
					switch (Z_TYPE_PP(property)) {
					    case IS_ARRAY:
					    case IS_OBJECT:
					        zend_hash_update(&prepared->default_static_members, n, l, (void**) &EG(uninitialized_zval_ptr), sizeof(zval*), NULL);
					        Z_ADDREF_P(EG(uninitialized_zval_ptr));
					    break;
					    
					    default: if (pthreads_store_separate(*property, &separated, 1, 0 TSRMLS_CC)==SUCCESS) {
						    zend_hash_update(&prepared->default_static_members, n, l, (void**) &separated, sizeof(zval*), NULL);
					    }
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
					switch (Z_TYPE_P(candidate->default_static_members_table[i])) {
					    case IS_OBJECT:
					    case IS_ARRAY:
					       prepared->default_static_members_table[i] =  
					            EG(uninitialized_zval_ptr);
					       Z_ADDREF_P(EG(uninitialized_zval_ptr));
					    break;
					    
					    default: {
					        prepared->default_static_members_table[i] = (zval*) emalloc(sizeof(zval));
					        
					        /* only copy simple variables from statics */
					        memcpy(
						        (prepared->default_static_members_table[i]), 
						        (candidate->default_static_members_table[i]), sizeof(zval));
					
					        pthreads_store_separate(
						        prepared->default_static_members_table[i],
						        &prepared->default_static_members_table[i],
						        1, 0 TSRMLS_CC
					        );
					    }
					}
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
                    CG(class_table), lower, prepared->name_length+1, (void**) &prepared, sizeof(zend_class_entry*), (void**)&searched);      
			} else prepared = *searched;
		}
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
int pthreads_prepare(PTHREAD thread TSRMLS_DC){
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
			(copy_ctor_func_t) pthreads_copy_function,
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
						    return FAILURE;
					    }
						
					    zend_hash_next_index_insert(&store, (void**) &prepared, sizeof(zend_class_entry*), NULL);
				    }
			    }
			}
		}
		
		for(zend_hash_internal_pointer_reset_ex(&store, &position);
			zend_hash_get_current_data_ex(&store, (void**) &entry, &position)==SUCCESS;
			zend_hash_move_forward_ex(&store, &position)) {
			
			if ((*entry)->parent) {
				zend_class_entry **search = NULL;
				if (zend_hash_index_find(PTHREADS_ZG(resolve), (zend_ulong) (*entry)->parent, (void**)&search) == SUCCESS) {
					(*entry)->parent = *search;
				}
			}
			
			zend_hash_apply(
				&(*entry)->function_table, (apply_func_t) pthreads_apply_method_scope TSRMLS_CC);
			zend_hash_apply(
				&(*entry)->properties_info, (apply_func_t) pthreads_apply_property_scope TSRMLS_CC);
			
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
	
	/* inherit globals, I don't like this ... */
	if (thread->options & PTHREADS_ALLOW_GLOBALS) {
		HashPosition position;
		HashTable *tables[] = {&PTHREADS_EG(thread->cls, symbol_table), &EG(symbol_table)};
		zval **symbol = NULL;
		
		for (zend_hash_internal_pointer_reset_ex(tables[0], &position);
			 zend_hash_get_current_data_ex(tables[0], (void**) &symbol, &position) == SUCCESS;
			 zend_hash_move_forward_ex(tables[0], &position)) {
			 char *symname = NULL;
		 	 int   symlen = 0;
		 	 zend_ulong symidx = 0L;

			 if (zend_hash_get_current_key_ex(tables[0], &symname, &symlen, &symidx, 0, &position) == HASH_KEY_IS_STRING) {
			 	zval *separated = NULL;
			 	
			 	if (pthreads_store_separate_from(*symbol, &separated, 1, 1, thread->cls TSRMLS_CC) == SUCCESS) {
		 			Z_SET_REFCOUNT_P(separated, 1);
			 		Z_SET_ISREF_P(separated);
			 		zend_hash_update(tables[1], symname, symlen, (void**) &separated, sizeof(zval*), NULL);
				} else {
					zval_ptr_dtor(&separated);
				}
			 }
		}
	}

	/* set sensible resource destructor */
	if (!PTHREADS_G(default_resource_dtor))
		PTHREADS_G(default_resource_dtor)=(EG(regular_list).pDestructor);
	EG(regular_list).pDestructor =  (dtor_func_t) pthreads_prepared_resource_dtor;	
	
	return SUCCESS;
} /* }}} */

/* {{{ property info ctor */
static void pthreads_preparation_property_info_copy_ctor(zend_property_info *pi) {
    pi->name = estrndup(pi->name, pi->name_length);    
    
    if (pi->doc_comment)
        pi->doc_comment = estrndup(pi->doc_comment, pi->doc_comment_len);
} /* }}} */

/* {{{ property info dtor */
static void pthreads_preparation_property_info_copy_dtor(zend_property_info *pi) {
#if PHP_VERSION_ID > 50399
    str_efree((char*)pi->name);
#else
    efree((char*)pi->name);
#endif
        
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

/* {{{ fix method scope for prepared entries, enabling self:: and parent:: to work */
static int pthreads_apply_method_scope(zend_function *function TSRMLS_DC) {
	if (function && function->type == ZEND_USER_FUNCTION) {
		zend_op_array *ops = &function->op_array;
		
		if (ops->scope) {
			zend_class_entry **search = NULL;
			if (zend_hash_index_find(PTHREADS_ZG(resolve), (zend_ulong) ops->scope, (void**)&search) == SUCCESS) {
				ops->scope = *search;
			}
		} else ops->scope = NULL;
		
		function->common.prototype = NULL;
	}
	
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ fix scope for prepared entry properties, enabling private members in foreign objects to work */
static int pthreads_apply_property_scope(zend_property_info *info TSRMLS_DC) {
	if (info->ce) {
		zend_class_entry **search = NULL;
		if (zend_hash_index_find(PTHREADS_ZG(resolve), (zend_ulong) info->ce, (void**)&search) == SUCCESS) {
			info->ce = *search;
		}
	}
	
	return ZEND_HASH_APPLY_KEEP;
} /* }}} */

/* {{{ destroy a resource, if we created it ( ie. it is not being kept by another thread ) */
static void pthreads_prepared_resource_dtor(zend_rsrc_list_entry *entry) {
	TSRMLS_FETCH();
	
	zend_try {
		if (!pthreads_resources_kept(entry TSRMLS_CC)){
			if (PTHREADS_G(default_resource_dtor))
				PTHREADS_G(default_resource_dtor)(entry);
		}
	} zend_end_try();
} /* }}} */

#endif


