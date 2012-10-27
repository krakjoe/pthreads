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

/* {{{ attach appropriate handlers to candidate for origin in thread */
static void pthreads_preparation_handlers(PTHREAD thread, zend_class_entry *candidate, zend_class_entry *attach TSRMLS_DC); /* }}} */

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

/* {{{ empty method entries */
zend_function_entry	pthreads_empty_methods[] = {
	{NULL, NULL, NULL}
}; /* }}} */

/* {{{ initialize prepared class entry storage */
void pthreads_prepare_classes_init(PTHREAD thread TSRMLS_DC) {
	zend_llist_init(&(thread->preparation.classes), sizeof(zend_class_entry*), (llist_dtor_func_t) pthreads_preparation_classes_dtor, 1);
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
				pthreads_preparation_handlers(thread, candidate, &ce TSRMLS_CC);
				
				/*
				* Registration
				*/
				prepared=zend_register_internal_class(&ce TSRMLS_CC);
				
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
				* Copy Defaults and Statics
				*/
				{
					/*
					* Default Properties
					*/
#if PHP_VERSION_ID > 50399
					if (candidate->default_properties_count) {
						int i;
						prepared->default_properties_table = emalloc(sizeof(zval*) * candidate->default_properties_count);
						for (i = 0; i < candidate->default_properties_count; i++) {
							prepared->default_properties_table[i] = candidate->default_properties_table[i];
							if (candidate->default_properties_table[i]) {
								ALLOC_INIT_ZVAL(prepared->default_properties_table[i]);
								MAKE_COPY_ZVAL(
									&candidate->default_properties_table[i], prepared->default_properties_table[i]
								);
							}
						}
						prepared->default_properties_count = candidate->default_properties_count;
					}
#else
					{
						zval *tp;
						zend_hash_copy
						(
							&prepared->default_properties,
							&candidate->default_properties,
							(copy_ctor_func_t) zval_add_ref,
							&tp, sizeof(zval*)
						);
					}
#endif
					
					/*
					* Static Properties
					*/
#if PHP_VERSION_ID > 50399
					if (candidate->default_static_members_count) {
						int i;
						prepared->default_static_members_table = perealloc(
							prepared->default_static_members_table, 
							sizeof(zval*) * candidate->default_static_members_count, 1
						);
						for (i = 0; i < candidate->default_static_members_count; i++) {
							prepared->default_static_members_table[i] = candidate->default_static_members_table[i];
							if (candidate->default_static_members_table[i]) {
								SEPARATE_ZVAL_TO_MAKE_IS_REF(&candidate->default_static_members_table[i]);
								prepared->default_static_members_table[i] = candidate->default_static_members_table[i];
								Z_ADDREF_P(prepared->default_static_members_table[i]);
							}
						}
						prepared->default_static_members_count = candidate->default_static_members_count;
					}
#else
					{
						zval *ts;
						zend_hash_copy
						(
							&prepared->default_static_members,
							&candidate->default_static_members,
							(copy_ctor_func_t) zval_add_ref,
							&ts, sizeof(zval*)
						);
					}
#endif
				}
				
				/*
				* Copy Constants Table
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
				prepared->refcount+=2;
			} else prepared = *searched;
			
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
						E_ERROR, "pthreads detected failure while preparing %s in %s", (*entry)->name, thread->std.ce->name, thread->tid
					);
					break;
				}
			}
		}
	}
} /* }}} */

/* {{{ attach appropriate object handlers for candidate with respect to scope of thread */
static void pthreads_preparation_handlers(PTHREAD thread, zend_class_entry *candidate, zend_class_entry *attach TSRMLS_DC) {
	int scope = pthreads_scope_detect(thread, candidate TSRMLS_CC);
	zend_bool connect = PTHREADS_IS_NOT_CONNECTION(thread);
	
	if (scope) {
		if (((scope & PTHREADS_SCOPE_THREAD)==PTHREADS_SCOPE_THREAD)) {
			if (connect) {
				attach->create_object = pthreads_connection_thread_ctor;
			} else attach->create_object = pthreads_object_thread_ctor;
		} else if (((scope & PTHREADS_SCOPE_WORKER)==PTHREADS_SCOPE_WORKER)) {
			if (connect) {
				attach->create_object = pthreads_connection_worker_ctor;
			} else attach->create_object = pthreads_object_worker_ctor;
		} else if (((scope & PTHREADS_SCOPE_STACKABLE)==PTHREADS_SCOPE_STACKABLE)) {
			if (connect) {
				attach->create_object = pthreads_connection_stackable_ctor;
			} else attach->create_object = pthreads_object_stackable_ctor;
		}
		
		if (attach->create_object) {
			attach->serialize = zend_class_serialize_deny;
			attach->unserialize = zend_class_unserialize_deny;
			attach->clone = NULL;
		}
	} else if (candidate->create_object) {
		attach->create_object = candidate->create_object;
		attach->clone = candidate->clone;
		attach->serialize = candidate->serialize;
		attach->unserialize = candidate->unserialize;
	}
} /* }}} */

/* {{{ copy property info 
	@TODO possibly adjust scope here */
static void pthreads_preparation_property_info_ctor(zend_property_info *pi) {} /* }}} */

/* {{{ destroy property info 
	@TODO possibly undo/free for adjustments made above */
static void pthreads_preparation_property_info_dtor(zend_property_info *pi) {} /* }}} */

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
			}
#endif
		}
	}
} /* }}} */

/* {{{ destroy prepared function
	@TODO statics */
static void pthreads_preparation_function_dtor(zend_function *pfe) {
	
} /* }}} */

/* {{{ destroy prepared classes */
static void pthreads_preparation_classes_dtor(void **ppce) {
	zend_class_entry *pce = (zend_class_entry*) *ppce;
	if(pce) {
		if (--pce->refcount == 1) {
#if PHP_VERSION_ID > 50399
			
#else
			zend_hash_destroy(&pce->default_properties);
#endif

#if PHP_VERSION_ID > 50399
			if (pce->default_static_members_count) {
				int i;
				for (i=0; i<pce->default_static_members_count; i++) {
					if (pce->default_static_members_table[i]) {
						zval_ptr_dtor(&pce->default_static_members_table[i]);
					}
				}
				free(pce->default_static_members_table);
			}
#else
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
