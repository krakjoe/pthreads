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

/* {{{ prepared property info ctor */
static void pthreads_preparation_property_info_ctor(zend_property_info *pi); /* }}} */
/* {{{ prepared property info dtor */
static void pthreads_preparation_property_info_dtor(zend_property_info *pi); /* }}} */

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

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(PTHREAD thread, zend_class_entry *candidate TSRMLS_DC) {
	zend_class_entry *prepared = NULL, **searched = NULL;
	
	if (candidate) {
#if PHP_VERSION_ID > 50399
		char *lcname = (char*) emalloc(candidate->name_length+1);
#else
		char *lcname = (char*) malloc(candidate->name_length+1);
#endif
		if (lcname != NULL) {
			/* lowercase name for lookup/insertion */
			zend_str_tolower_copy(lcname, candidate->name, candidate->name_length);
			/* perform lookup for existing class */
			if (zend_hash_find(CG(class_table), lcname, candidate->name_length+1, (void**)&searched)!=SUCCESS) {
				zval *tz;
				zend_function *tf;
				zend_property_info *ti;
	
				/* create a new user class for this context */
				prepared = (zend_class_entry*) emalloc(sizeof(zend_class_entry));
				prepared->name = estrndup(candidate->name, candidate->name_length);
				prepared->name_length = candidate->name_length;
				prepared->type = candidate->type;
				prepared->ce_flags = candidate->ce_flags;
				
				/* initialize class data */
				zend_initialize_class_data(prepared, 1 TSRMLS_CC);
				
				/* perform inheritance */
				if (candidate->parent)
					candidate->parent = pthreads_prepared_entry(thread, candidate->parent TSRMLS_CC);

				/* declare interfaces */
				if (candidate->num_interfaces) {
					uint interface;
					prepared->interfaces = emalloc(sizeof(zend_class_entry*) * candidate->num_interfaces);
					for(interface=0; interface<prepared->num_interfaces; interface++)
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
					void *usources[11] = {
						candidate->constructor,
						candidate->destructor,
						candidate->clone,
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
								case 3: zend_hash_update(&prepared->function_table, "__serialize", sizeof("__serialize"), &candidate->serialize_func, sizeof(zend_function), (void**) &prepared->serialize_func); break;
								case 4: zend_hash_update(&prepared->function_table, "__unserialize", sizeof("__unserialize"), &candidate->unserialize_func, sizeof(zend_function), (void**) &prepared->unserialize_func); break;
								/* handlers */
								case 5: prepared->create_object = candidate->create_object; break;
								case 6: prepared->serialize = candidate->serialize; break;
								case 7: prepared->unserialize = candidate->unserialize; break;
								case 8: {
									prepared->get_iterator = candidate->get_iterator;
									prepared->iterator_funcs = candidate->iterator_funcs;
								} break;
								case 9: prepared->interface_gets_implemented = candidate->interface_gets_implemented; break;
								case 10: prepared->get_static_method = candidate->get_static_method; break;
							}
						}
					} while(++umethod < 11);
				}
				
				/* copy function table */
				zend_hash_copy(&prepared->function_table, &candidate->function_table, (copy_ctor_func_t) function_add_ref, &tf, sizeof(zend_function));
				
				/* copy property info structures */
				zend_hash_copy(&prepared->properties_info, &candidate->properties_info, (copy_ctor_func_t) pthreads_preparation_property_info_ctor, &ti, sizeof(zend_property_info));
				prepared->properties_info.pDestructor = (dtor_func_t) pthreads_preparation_property_info_dtor;
				
				/* copy statics and defaults */
				{
#if PHP_VERSION_ID < 50400
					zend_hash_copy(&prepared->default_properties, &candidate->default_properties, (copy_ctor_func_t) pthreads_preparation_default_properties_ctor, &tz, sizeof(zval*));
					zend_hash_copy(&prepared->default_static_members, &candidate->default_static_members, (copy_ctor_func_t) pthreads_preparation_default_properties_ctor, &tz, sizeof(zval*));
					prepared->default_properties.pDestructor = (dtor_func_t) pthreads_preparation_default_properties_dtor;
					prepared->default_static_members.pDestructor = (dtor_func_t) pthreads_preparation_default_properties_dtor;
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
									1 TSRMLS_CC
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
									1 TSRMLS_CC
								);
							} else prepared->default_static_members_table[i]=NULL;
						}
						prepared->static_members_table = prepared->default_static_members_table;
					} else prepared->default_static_members_count = 0;
					
					/* copy user info struct */
					memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));
#endif
				}
				
				/* copy constants */
				zend_hash_copy(&prepared->constants_table, &candidate->constants_table, (copy_ctor_func_t) zval_add_ref, &tz, sizeof(zval*));

				/* update class table */
#if PHP_VERSION_ID > 50399
				lcname = (char*)zend_new_interned_string(lcname, prepared->name_length+1, 1 TSRMLS_CC);
				if (IS_INTERNED(lcname)) {
					 zend_hash_quick_update(CG(class_table), lcname, prepared->name_length+1, INTERNED_HASH(lcname), &prepared, sizeof(zend_class_entry*), NULL);
				} else zend_hash_update(CG(class_table), lcname, prepared->name_length+1, &prepared, sizeof(zend_class_entry*), NULL);
#else
				zend_hash_update(CG(class_table), lcname, prepared->name_length+1, &prepared, sizeof(zend_class_entry*), NULL);
#endif
			} else {
				prepared = *searched;
			}

			/* free lowercase name buffer */
#if PHP_VERSION_ID > 50399
			str_efree(lcname);
#else
			free(lcname);
#endif
		} else zend_error(E_ERROR, "pthreads has detected a memory error while attempting to prepare %s for execution in %s %lu", candidate->name, thread->std.ce->name, thread->tid);
	}
	return prepared;
} /* }}} */

/* {{{ prepares the current context to execute the referenced thread */
void pthreads_prepare(PTHREAD thread TSRMLS_DC){
	HashPosition position;
	
	/* inherit ini entries from parent ... */
	{
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
				if (((entry[0]->value && entry[1]->value) && (strcmp(entry[0]->value, entry[1]->value) != 0)) || entry[0]->value) {
					zend_bool modifiable = entry[1]->modifiable;
					zend_alter_ini_entry_ex(
						setting, slength, 
						entry[0]->value, entry[0]->value_length,
						ZEND_INI_STAGE_ACTIVATE, ZEND_INI_SYSTEM, 1 TSRMLS_CC
					);
					entry[1]->modifiable = modifiable;
				}
			}
		}
	}
	
	/* copy constants */
	{
		zend_constant *zconstant;
		HashTable *table[2] = {PTHREADS_EG(thread->cls, zend_constants), EG(zend_constants)};
		
		for(zend_hash_internal_pointer_reset_ex(table[0], &position);
			zend_hash_get_current_data_ex(table[0], (void**) &zconstant, &position)==SUCCESS;
			zend_hash_move_forward_ex(table[0], &position)) {
			if (strcmp(zconstant->name, "STDIN")!=0 &&
				strcmp(zconstant->name, "STDOUT")!=0 &&
				strcmp(zconstant->name, "STDERR")!=0 &&
				strcmp(zconstant->name, "TRUE")!=0 &&
				strcmp(zconstant->name, "FALSE")!=0 &&
				strcmp(zconstant->name, "NULL")!=0) {
				if (!zend_hash_exists(table[1], zconstant->name, zconstant->name_len)) {
					zend_constant constant;
					
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
	{
		zend_function function;
		zend_hash_merge(
			EG(function_table), 
			PTHREADS_EG(thread->cls, function_table), 
			(copy_ctor_func_t) function_add_ref, 
			&function, sizeof(zend_function), 0
		);
	}
	
	/* inherit class table from parent ... */
	{
		zend_class_entry **entry;
		HashTable *table[2] = {PTHREADS_CG(thread->cls, class_table), CG(class_table)};

		for(zend_hash_internal_pointer_reset_ex(table[0], &position);
			zend_hash_get_current_data_ex(table[0], (void**) &entry, &position)==SUCCESS;
			zend_hash_move_forward_ex(table[0], &position)) {
			char *lcname;
			uint lcnamel;
			ulong idx;
			
			if (zend_hash_get_current_key_ex(table[0], &lcname, &lcnamel, &idx, 0, &position)==HASH_KEY_IS_STRING) {
				if (!zend_hash_exists(table[1], lcname, lcnamel)){
					if (pthreads_prepared_entry(thread, *entry TSRMLS_CC)==NULL) {
						zend_error_noreturn(
							E_ERROR, "pthreads detected failure while preparing %s in %s", (*entry)->name, thread->std.ce->name, thread->tid
						);
						break;
					}
				}
			}
		}
	}
	
	/* merge included files with parent, more compatible than copying ( think apc ) */
	{
		int included;
		zend_hash_merge(
			&EG(included_files), 
			&PTHREADS_EG(thread->cls, included_files),
			NULL, &included, sizeof(int), 0
		);
	}
} /* }}} */

/* {{{ copy property info 
	@TODO possibly adjust scope here */
static void pthreads_preparation_property_info_ctor(zend_property_info *pi) {} /* }}} */

/* {{{ destroy property info 
	@TODO possibly undo/free for adjustments made above */
static void pthreads_preparation_property_info_dtor(zend_property_info *pi) {} /* }}} */

#if PHP_VERSION_ID < 50400
/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_ctor(zval **property) {
	
} /* }}} */

/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property) {
	zval_ptr_dtor(&property);
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
		copy->function = alias->function;
	}
	return copy;
} /* }}} */

/* {{{ trait precendence for 5.4 */
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
		copy->function = precedence->function;
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

#endif
