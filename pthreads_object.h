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
#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H
static 	zend_object_value 		pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
static 	void 					pthreads_detach_from_instance(void * child TSRMLS_DC);

		zend_function_entry		pthreads_runnable_methods[] = {{NULL, NULL, NULL}};
static	zend_object_value 		pthreads_attach_to_runnable(zend_class_entry *entry TSRMLS_DC);
static 	void 					pthreads_detach_from_runnable(void * child TSRMLS_DC);

extern	char *					pthreads_serialize(zval *unserial TSRMLS_DC);
extern	zval *					pthreads_unserialize(char *serial TSRMLS_DC);
extern	int 					pthreads_unserialize_into(char *serial, zval *result TSRMLS_DC);

		void * 					PHP_PTHREAD_ROUTINE(void *);
		
typedef struct _pthread_construct {						/* Thread internal structure */
	zend_object 			std;						/* standard entry */
	pthread_t				thread;						/* internal thread identifier */
	PEVENT					started;					/* started event */
	PEVENT					finished;					/* finished event */
	zend_class_entry		*ice;					/* the threading version of the user class */
	void					***pls;						/* local storage of parent thread */
	void					***ls;						/* local storage for thread */
	int						running;					/* flag to indicate thread was started */
	int						joined;						/* flag to indicate thread was joined */
	char					*serial;					/* serialized symbols */
} THREAD, *PTHREAD;

#define PTHREADS_FETCH_FROM_EX(object, ls)				(PTHREAD) zend_object_store_get_object(object, ls)
#define PTHREADS_FETCH_FROM(object)						(PTHREAD) zend_object_store_get_object(object TSRMLS_CC)
#define PTHREADS_FETCH									(PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC)

/**
* This is the object constructor for the parent thread, we construct a different objects for instances
**/
static zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	zend_object_value attach;
	
	PTHREAD thread = calloc(1, sizeof(*thread));								/* allocate memory for thread */	
	thread->joined = 		0;													/* set flags */
	thread->running = 		0;													
	thread->ls	=			NULL;												/* context for new thread */
	thread->pls = 			tsrm_ls;											/* context for parent */
	thread->started = 		pthreads_create_event();							/* fired when the parent thread can continue */
	thread->finished = 		pthreads_create_event();							/* fired when the method has returned a result */
	
	zend_object_std_init(&thread->std, entry TSRMLS_CC);						/* initialize standard entry */
	
#if PHP_VERSION_ID < 50399
	 zval *temp;
	 
	 zend_hash_copy(															/* copy standard properties, as yet unused */
		thread->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
#else
	object_properties_init(&(thread->std), entry);								/* compatible with 5.4 */
#endif

	attach.handle = zend_objects_store_put(										/* standard stuff, store definition */
		thread,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_instance, 
		NULL TSRMLS_CC
	);
	
	attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();

	return attach;																/* return attached */
}

/**
* This is the object constructor for instances
**/
static zend_object_value pthreads_attach_to_runnable(zend_class_entry *entry TSRMLS_DC) {
	zend_object_value	attach;
	PTHREAD thread		= calloc(1, sizeof(*thread));
	if (thread) {
		thread->thread		= pthread_self();
		thread->joined		= -1;
		thread->running		= -1;
		thread->ls			= tsrm_ls;
		thread->pls			= NULL;
		
		zend_object_std_init(&thread->std, entry TSRMLS_CC);						/* initialize standard entry */
		
#if PHP_VERSION_ID < 50399
		 zval *temp;
		 
		 zend_hash_copy(															/* copy standard properties, as yet unused */
			thread->std.properties,
			&entry->default_properties,
			ZVAL_COPY_CTOR,
			&temp, sizeof(zval*)
		);
#else
		object_properties_init(&(thread->std), entry);								/* compatible with 5.4 */
#endif

		attach.handle = zend_objects_store_put(										/* standard stuff, store definition */
			thread,  
			(zend_objects_store_dtor_t) zend_objects_destroy_object, 
			pthreads_detach_from_runnable, 
			NULL TSRMLS_CC
		);
		
		attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();
	}

	return attach;
}

/**
* This is the real destructor for a thread
**/
static void pthreads_detach_from_instance(void * child TSRMLS_DC){
	PTHREAD thread = (PTHREAD) child;
	char *result = NULL;												/* we need a reference to this to free it for the child */
	
	if (thread) {
		if (thread->thread) {											/* be certain we reference an actual thread */
			if (thread->running) {										/* if the thread is still running we will join with it */
				if (!thread->joined) {									/* if it wasn't already joined */
					pthread_join(thread->thread, (void*) &result);
					if (result)											/* check for result and free it's memory */
						free(result);
				}
			}
			
			if (thread->started) {
				pthreads_destroy_event(thread->started); 				/* destroy the started event and release it's resources */
			}
			
			if (thread->finished) {
				pthreads_destroy_event(thread->finished);				/* destroy the finished event and release it's resources */
			}
		}
		
		if (thread->std.properties){
			zend_hash_destroy(thread->std.properties);
			FREE_HASHTABLE(thread->std.properties);						/* free the table they were stored in */
		}
		
		free(thread);													/* free memory associated with thread */
	}
}

static void pthreads_detach_from_runnable(void *child TSRMLS_DC)
{
	PTHREAD thread = (PTHREAD) child;
	
	if (thread) {
		if (thread->std.properties){
			zend_hash_destroy(thread->std.properties);
			FREE_HASHTABLE(thread->std.properties);						/* free the table they were stored in */
		}

#if PHP_VERSION_ID > 50399
		if (thread->std.properties_table) {
			zend_hash_destroy(
				&thread->std.properties_table
			);
			free(thread->std.properties_table);
		}
#endif

		free(thread);													/* free memory associated with thread */
	}
}

#if PHP_VERSION_ID > 50399
static void zend_extension_op_array_dtor_handler(zend_extension *extension, zend_op_array *op_array TSRMLS_DC)
{
	if (extension->op_array_dtor) {
		extension->op_array_dtor(op_array);
	}
}

/**
* The only difference between this and the release of zend_function_dtor 
* is we free the run_time_cache if there are no more references to it and the release version free's it regardless
* Unsure of what this affects and how ...
**/
static void pthreads_method_del_ref(zend_function *function);
static void pthreads_method_del_ref(zend_function *function){
	if (function && function->type == ZEND_USER_FUNCTION ) {
		TSRMLS_FETCH();
		
		zend_op_array *op_array = (zend_op_array*) function;
		
		if (op_array) {
			zend_literal *literal = op_array->literals;
			zend_literal *end;
			zend_uint i;

			if (op_array->static_variables) {
				zend_hash_destroy(op_array->static_variables);
				FREE_HASHTABLE(op_array->static_variables);
			}
		 
			if (--(*op_array->refcount)>0) {
				return;
			}

			if (op_array->run_time_cache) {	
				free(
					op_array->run_time_cache
				);
			}
			
			free(op_array->refcount);

			if (op_array->vars) {
				i = op_array->last_var;
				while (i > 0) {
					i--;
					str_free(op_array->vars[i].name);
				}
				free(op_array->vars);
			}

			if (literal) {
				end = literal + op_array->last_literal;
				while (literal < end) {
					zval_dtor(&literal->constant);
					literal++;
				}
				free(op_array->literals);
			}
			free(op_array->opcodes);

			if (op_array->function_name) {
				free((char*)op_array->function_name);
			}
			if (op_array->doc_comment) {
				free((char*)op_array->doc_comment);
			}
			if (op_array->brk_cont_array) {
				free(op_array->brk_cont_array);
			}
			if (op_array->try_catch_array) {
				free(op_array->try_catch_array);
			}
			if (op_array->fn_flags & ZEND_ACC_DONE_PASS_TWO) {
				zend_llist_apply_with_argument(&zend_extensions, (llist_apply_with_arg_func_t) zend_extension_op_array_dtor_handler, op_array TSRMLS_CC);
			}
			if (op_array->arg_info) {
				for (i=0; i<op_array->num_args; i++) {
					str_free(op_array->arg_info[i].name);
					if (op_array->arg_info[i].class_name) {
						str_free(op_array->arg_info[i].class_name);
					}
				}
				free(op_array->arg_info);
			}
		}
	}
}
#endif

void * PHP_PTHREAD_ROUTINE(void *arg){
	PTHREAD 	thread = (PTHREAD) arg;
	PTHREAD		child = NULL;
	char		*result = NULL;
	
	if (thread) {
	
		/**
		* 
		**/
		zend_class_entry 	runnable;								
		zval 				*runtime;								
		zend_function		*prepare;						
		zend_function		*run;							
		zval 				*retval = NULL;								
		zval 				*symbols = NULL;				
		HashTable			*properties = NULL;
		int					preparation = 0;
		
		/**
		* Allocate and Initialize an interpreter context for this Thread
		**/
		void ***ctx = thread->ls = tsrm_new_interpreter_context(); { 
	
			/**
			* Set Context/TSRMLS
			**/
			tsrm_set_interpreter_context(ctx);
			TSRMLS_FETCH_FROM_CTX(ctx);		
			
			/**
			* Activate Zend
			**/
			zend_activate(TSRMLS_C);							
			zend_activate_modules(TSRMLS_C);					
			
			/**
			* This is where the instance of Runnable is stored and referenced in this thread
			*/ 
			MAKE_STD_ZVAL(runtime);

			/**
			* First some basic executor setup, using zend_first_try sets a sensible bailout address incase of error
			**/
			zend_first_try {									
				EG(in_execution) = 1;							
				EG(current_execute_data)=NULL;					
				EG(current_module)=&pthreads_module_entry;
				
				/**
				* This part of the code initializes a new instance of the Runnable object
				**/
				INIT_CLASS_ENTRY(
					runnable, 									
					"Runnable", 
					pthreads_runnable_methods
				);
				runnable.create_object = pthreads_attach_to_runnable;
				runnable.serialize = zend_class_serialize_deny;
				runnable.unserialize = zend_class_unserialize_deny;
				thread->ice=zend_register_internal_class(
					&runnable TSRMLS_CC
				);
				object_init_ex(runtime, thread->ice);
				child = PTHREADS_FETCH_FROM(runtime);
				child->ice	= thread->std.ce;
				
				/**
				* We can now set the executor and scope to reference a thread safe version of $this
				**/
				EG(called_scope)=		thread->ice;
				EG(scope)=				thread->ice;
				EG(This)=				runtime;
				
				/**
				* This next part will import the users methods into the Runnable instance
				**/
				zend_function *tf; {
					zend_hash_copy(
						&Z_OBJCE_P(runtime)->function_table, 
						&thread->std.ce->function_table,
						(copy_ctor_func_t) function_add_ref,
						&tf, sizeof(zend_function)
					);
				}
				
				/**
				* From here in, do not attempt to write the parent context
				**/
				pthreads_fire_event(thread->started); {	
					
					/**
					* Find methods for execution
					**/
					if(zend_hash_find(								
						&Z_OBJCE_P(runtime)->function_table, 
						"__prepare", sizeof("__prepare"), 
						(void**) &prepare
					)==SUCCESS) {
						preparation=1;
					}
					
					zend_hash_find(
						&Z_OBJCE_P(runtime)->function_table, 
						"run", sizeof("run"), 
						(void**) &run
					);
					
#if PHP_VERSION_ID > 50399
					/**
					* NOTE@ run_time_cache
					*	This has to be handled differently here in 5.4 series PHP because of the addition of run_time_cache
					**/
					Z_OBJCE_P(runtime)->function_table.pDestructor = (dtor_func_t) pthreads_method_del_ref;
#endif

					/**
					* Even if the user does not supply symbols, they may create some
					**/
					if (!EG(active_symbol_table)) {
						EG(active_symbol_table)=&EG(symbol_table);	
					}
					
					/**
					* Execution Time
					**/
					if (preparation) {				
						/**
						* The result of the call to Thread::__prepare is ignored
						**/
						zval *discard;
						EG(return_value_ptr_ptr)=&discard;				
						EG(active_op_array)=(zend_op_array*)prepare;
						
						zend_execute(EG(active_op_array) TSRMLS_CC);
						
						if (discard && Z_TYPE_P(discard) != IS_NULL) {
							FREE_ZVAL(discard);
						}
					}
					
					/**
					* Deserialize symbols from parent context
					**/
					if (thread->serial)	{											
						symbols = pthreads_unserialize(thread->serial TSRMLS_CC); 	
						if (symbols) {			
							/**
							* Now set the properties of the Runnable to the TS symbols
							**/
							properties = child->std.properties;
							child->std.properties=Z_ARRVAL_P(
								symbols
							);				
						}
						free(thread->serial);
					}
					
					ALLOC_INIT_ZVAL(retval);
					/**
					* Now time to execute Thread::run
					**/
					EG(return_value_ptr_ptr)=&retval;					
					EG(active_op_array)=(zend_op_array*)run;
					
					zend_execute(EG(active_op_array) TSRMLS_CC);
					
					/**
					* Results are serialized too, because it's COMPATIBLE and SAFE
					**/
					if (retval && Z_TYPE_P(retval) != IS_NULL) {
						result = pthreads_serialize(
							retval TSRMLS_CC
						);
						FREE_ZVAL(retval);
					}
					FREE_ZVAL(runtime);
				}
			} zend_catch {	
				/**
				* If anything has gone wrong we need to make sure all memory is released
				* @NOTE errors in users code cause zend to throw
				**/
				if (symbols && Z_TYPE_P(symbols) != IS_NULL) {
					FREE_ZVAL(symbols);
				}
				
				if (retval && Z_TYPE_P(retval) != IS_NULL){
					FREE_ZVAL(retval);
				}
			} zend_end_try();
			
			/**
			* This ensures original property HashTable is freed
			**/
			child->std.properties = properties;
			/**
			* Deactivate Zend
			**/
			zend_deactivate_modules(TSRMLS_C);					/* shutdown modules */
			zend_deactivate(TSRMLS_C);							/* shutdown zend */
			
			/**
			* Free Context
			**/
			tsrm_free_interpreter_context(ctx);					/* free interpreter context */	
		}
		
		if (thread->started && !thread->started->fired)			/* firing started event in case of failure */
			pthreads_fire_event(thread->started);
		if (thread->finished)
			pthreads_fire_event(thread->finished);				/* more accurate down here, makes busy() more useful */
	}

	pthread_exit((void*)result);								/* leave thread */
} 
#endif
