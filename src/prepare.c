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

#ifdef AG
#	undef AG
#endif

#define AG(v) TSRMG(alloc_globals_id, zend_alloc_globals *, v)

/* {{{ prepared function ctor */
static void pthreads_preparation_function_ctor(zend_function *pfe); /* }}} */

/* {{{ prepared property info ctor */
static void pthreads_preparation_property_info_ctor(zend_property_info *pi); /* }}} */

/* {{{ prepared classes dtor */
static void pthreads_preparation_classes_dtor(void **ppce); /* }}} */

/* {{{ prepared function ctor */
static void pthreads_preparation_function_dtor(zend_function *pfe); /* }}} */

/* {{{ prepared property info dtor */
static void pthreads_preparation_property_info_dtor(zend_property_info *pi); /* }}} */

#if PHP_VERSION_ID < 50400
/* {{{ default property ctor for 5.3 */
static void pthreads_preparation_default_properties_ctor(zval **property); /* }}} */
/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property); /* }}} */
#endif

/* {{{ empty method entries */
zend_function_entry	pthreads_empty_methods[] = {
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ initialize prepared class entry storage */
void pthreads_prepare_classes_init(PTHREAD thread TSRMLS_DC) {
	zend_llist *classes = &thread->preparation.classes;
	if (!classes || !classes->dtor)
		zend_llist_init(classes, sizeof(zend_class_entry*), (llist_dtor_func_t) pthreads_preparation_classes_dtor, 1);
} /* }}} */

/* {{{ free prepared class storage */
void pthreads_prepare_classes_free(PTHREAD thread TSRMLS_DC) {
	zend_llist *classes = &thread->preparation.classes;
	if (classes) {
		zend_llist_destroy(classes);
	}
} /* }}} */

/* {{{ fetch prepared class entry */
zend_class_entry* pthreads_prepared_entry(PTHREAD thread, zend_class_entry *candidate TSRMLS_DC) {
	zend_class_entry *prepared = NULL, **searched = NULL;
	
	if (candidate) {
		char *lcname = (char*) malloc(candidate->name_length+1);
		
		if (lcname != NULL) {
			zend_str_tolower_copy(lcname, candidate->name, candidate->name_length);
			
			/*
			* look for class entry in current context
			*/
			if (zend_hash_find(CG(class_table), lcname, candidate->name_length+1, (void**)&searched)!=SUCCESS) {
				/*
				* create class entry in current context
				*/
				zend_class_entry ce;
				
				/*
				* Initialize an interned class entry
				*/
				INIT_CLASS_ENTRY_EX(
					ce, 
					candidate->name, 
					candidate->name_length,
					pthreads_empty_methods
				);
				
				/*
				* Attach appropriate handlers
				*/
				if (candidate->create_object)
					ce.create_object = candidate->create_object;
				if (candidate->serialize)
					ce.serialize = candidate->serialize;
				if (candidate->unserialize)
					ce.unserialize = candidate->unserialize;
				if (candidate->clone)
					ce.clone = candidate->clone;
				
				/*
					_zend_function *constructor;
					_zend_function *destructor;
					_zend_function *clone;
					_zend_function *__get;
					_zend_function *__set;
					_zend_function *__unset;
					_zend_function *__isset;
					_zend_function *__call;
					_zend_function *__callstatic;
					_zend_function *__tostring;
					_zend_function *serialize_func;
					_zend_function *unserialize_func;
				*/
				
				/*
				* Registration
				*/
				if (candidate->parent && candidate != candidate->parent) {
					prepared=zend_register_internal_class_ex(&ce, pthreads_prepared_entry(
						thread, candidate->parent TSRMLS_CC
					), NULL TSRMLS_CC);
				} else prepared=zend_register_internal_class(&ce TSRMLS_CC);
				prepared->ce_flags = candidate->ce_flags;
				
				{
					zend_uint umethod = 0;
					zend_function *usources[13] = {
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
						NULL
					};
					
					do {
						zend_function **utarget;
						if (usources[umethod]) {
							switch(umethod){
								case 0: utarget = &prepared->constructor; break;
								case 1: utarget = &prepared->destructor; break;
								case 2: utarget = &prepared->clone; break;
								case 3: utarget = &prepared->__get; break;
								case 4: utarget = &prepared->__set; break;
								case 5: utarget = &prepared->__unset; break;
								case 6: utarget = &prepared->__isset; break;
								case 7: utarget = &prepared->__call; break;
								case 8: utarget = &prepared->__callstatic; break;
								case 9: utarget = &prepared->__tostring; break;
								case 10: utarget = &prepared->serialize_func; break;
								case 11: utarget = &prepared->unserialize_func; break;
							}
							if (utarget) {
								*utarget = (zend_function*) emalloc(sizeof(zend_function));
								if ((*utarget)) {
									memcpy(
										(*utarget), usources[umethod], sizeof(zend_function)
									);
								}
							}
						}
					} while(++umethod < 13);
				}
				
				/*
				* Copy Function Table
				*/
				{
					zend_function *tf;
					zend_hash_copy
					(
						&prepared->function_table, 
						&candidate->function_table,
						(copy_ctor_func_t) pthreads_preparation_function_ctor,
						&tf, sizeof(zend_function)
					);
					prepared->function_table.pDestructor = (dtor_func_t) pthreads_preparation_function_dtor;
				}
				
				/*
				* Copy Property Info
				*/
				{
					zend_property_info *ti;
					zend_hash_copy
					(
						&prepared->properties_info, 
						&candidate->properties_info,
						(copy_ctor_func_t) pthreads_preparation_property_info_ctor,
						&ti, sizeof(zend_property_info)
					);
					prepared->properties_info.pDestructor = (dtor_func_t) pthreads_preparation_property_info_dtor;
				}
				
				/*
				* Copy Defaults
				*/
				{
					/*
					* Default Properties
					*/
#if PHP_VERSION_ID < 50400
					{
						zval *tp;
						zend_hash_copy
						(
							&prepared->default_properties,
							&candidate->default_properties,
							(copy_ctor_func_t) pthreads_preparation_default_properties_ctor,
							&tp, sizeof(zval*)
						);
						prepared->default_properties.pDestructor = (dtor_func_t) pthreads_preparation_default_properties_dtor;
					}
#else
					if (candidate->default_properties_count) {
						int i;
						prepared->default_properties_table = malloc(
							sizeof(zval*) * candidate->default_properties_count
						);
						for (i=0; i<candidate->default_properties_count; i++) {
							prepared->default_properties_table[i]=candidate->default_properties_table[i];
							if (candidate->default_properties_table[i]) {
								ALLOC_ZVAL(prepared->default_properties_table[i]);
								MAKE_COPY_ZVAL(&candidate->default_properties_table[i], prepared->default_properties_table[i]);
								INIT_PZVAL(prepared->default_properties_table[i]);
							}
						}
						prepared->default_properties_count = candidate->default_properties_count;
					}
					
					if (candidate->default_static_members_count) {
						prepared->default_static_members_count = 0;
					}
#endif
					/*
					* Static Properties are pointless and cannot be static in the context of threading
					*/
				}
				
				/*
				* Copy Constants
				*/
				{
					zval *tc;
					zend_hash_copy
					(
						&prepared->constants_table,
						&candidate->constants_table,
						(copy_ctor_func_t) zval_add_ref,
						&tc, sizeof(zval*)
					);
				}
				
#if PHP_VERSION_ID > 50399
				/*
				* Copy user info struct
				*/
				memcpy(&prepared->info.user, &candidate->info.user, sizeof(candidate->info.user));
#endif

				/*
				* Adjust refcount such that pthreads is responsible for freeing this entry
				*/
				(prepared->refcount)++;
			} else {
				prepared = *searched;
			}
			
			/* free lowercase name buffer */
			free(lcname);
		} else zend_error(E_ERROR, "pthreads has detected a memory error while attempting to prepare %s for execution in %s %lu", candidate->name, thread->std.ce->name, thread->tid);
	}
	return prepared;
} /* }}} */

/* {{{ prepares the current context to execute the referenced thread */
void pthreads_prepare(PTHREAD thread TSRMLS_DC){
	HashPosition position;
	zend_class_entry **entry;
	HashTable *source = PTHREADS_CG(thread->cls, class_table);
	HashTable *destination = CG(class_table);
	
	for(zend_hash_internal_pointer_reset_ex(source, &position);
		zend_hash_get_current_data_ex(source, (void**) &entry, &position)==SUCCESS;
		zend_hash_move_forward_ex(source, &position)) {
		char *lcname;
		uint lcnamel;
		ulong idx;
		zend_class_entry *prepared;
		
		if (zend_hash_get_current_key_ex(source, &lcname, &lcnamel, &idx, 0, &position)==HASH_KEY_IS_STRING) {
			if (!zend_hash_exists(destination, lcname, lcnamel)){
				if ((prepared=pthreads_prepared_entry(thread, *entry TSRMLS_CC))!=NULL) {
					zend_llist_add_element(&(thread->preparation.classes), (void**) &prepared);
				} else {
					zend_error_noreturn(
						E_ERROR, "pthreads detected failure while preparing %s in %s thread: %lu", (*entry)->name, thread->std.ce->name, thread->tid
					);
					break;
				}
			}
		}
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
	ALLOC_ZVAL(*property);
	MAKE_COPY_ZVAL(property, *property);
	INIT_PZVAL(*property);
} /* }}} */

/* {{{ default property dtor for 5.3 */
static void pthreads_preparation_default_properties_dtor(zval *property) {
	zval_ptr_dtor(&property);
} /* }}} */
#endif

/* {{{ construct prepared function 
	@TODO statics */
static void pthreads_preparation_function_ctor(zend_function *pfe) {
	zend_function *function = (zend_function*) pfe;
	
	if (function) {
		if (function->type == ZEND_USER_FUNCTION) {
#if PHP_VERSION_ID > 50399
			zend_op_array *ops = &function->op_array;
			if (ops) {
				if (ops->run_time_cache) {
					ops->run_time_cache = NULL;
				}
				(*ops->refcount)++;
			}
#endif
		}
	}
} /* }}} */

/* {{{ destroy prepared function
	@TODO statics */
static void pthreads_preparation_function_dtor(zend_function *pfe) {
	zend_function *function = (zend_function*) pfe;
	
	if (function) {
		if (function->type == ZEND_USER_FUNCTION) {
#if PHP_VERSION_ID > 50399
			zend_op_array *ops = &function->op_array;		
			if (ops) {
				if (--(*ops->refcount)>0) {
					return;
				}
				
				if (ops->run_time_cache) {
					efree(ops->run_time_cache);
					ops->run_time_cache = NULL;
				}
				
			}
#endif
		}
	}
} /* }}} */

/* {{{ destroy prepared classes */
static void pthreads_preparation_classes_dtor(void **ppce) {
	zend_class_entry *pce = (zend_class_entry*) *ppce;
	if(pce) {
		if (--pce->refcount == 0) {
#if PHP_VERSION_ID > 50399
			if (pce->default_properties_count) {
				int i;
				for(i=0; i<pce->default_properties_count; i++) {
					if (pce->default_properties_table[i]) {
						//zval_dtor(pce->default_properties_table[i]);
					}
				}
				free(pce->default_properties_table);
			}
#else
			zend_hash_destroy(&pce->default_properties);
#endif

#if PHP_VERSION_ID > 50399
			if (pce->default_static_members_count) {
				int i;
				for(i=0; i<pce->default_static_members_count; i++) {
					if (pce->default_static_members_table[i]) {
						zval_ptr_dtor(&pce->default_static_members_table[i]);
					}
				}
				free(pce->default_static_members_table);
			}
#else
			/* we don't use statics but the table is still initialized so must be destroyed */
			zend_hash_destroy(&pce->default_static_members);
#endif

			zend_hash_destroy(&pce->properties_info);
			zend_hash_destroy(&pce->constants_table);
			zend_hash_destroy(&pce->function_table);
			
			if (pce->name) {
				free((char*)pce->name);
			}
			
			free(pce);
		}
	}
} /* }}} */

#endif
