#ifndef HAVE_PTHREADS_OBJECT_H
#define HAVE_PTHREADS_OBJECT_H
extern zend_class_entry *	pthreads_class_entry;

static zend_object_value 	pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC);
static void 				pthreads_detach_from_instance(void * child TSRMLS_DC);

static zend_object_value pthreads_attach_to_instance(zend_class_entry *entry TSRMLS_DC){
	zend_object_value attach;
	/** allocate memory for thread object **/
	PTHREAD thread = calloc(1, sizeof(*thread));
	/** flag to indicate thread was previously joined **/
	thread->joined = 0;
	/** flag to indicate thread was previously started **/
	thread->running = 0;
	/** parent threads local storage **/
	thread->pls = tsrm_ls;
	/** calling thread to wait for this before it continues **/
	thread->started = pthreads_create_event();
	/** initialize standard entry **/
	zend_object_std_init(&thread->std, entry TSRMLS_CC);
	/** copy standard properties **/
	zval *temp;
	zend_hash_copy(
		thread->std.properties,
		&entry->default_properties,
		ZVAL_COPY_CTOR,
		&temp, sizeof(zval*)
	);
	/** store definition **/
	attach.handle = zend_objects_store_put(
		thread,  
		(zend_objects_store_dtor_t) zend_objects_destroy_object, 
		pthreads_detach_from_instance, 
		zend_objects_store_clone_obj TSRMLS_CC
	);
	/** override get/set read/write property handlers to use mutex **/
	attach.handlers = (zend_object_handlers *) zend_get_std_object_handlers();
	/** return pthread **/
	return attach;
}
static void pthreads_detach_from_instance(void * child TSRMLS_DC){
	PTHREAD thread = (PTHREAD) child;
	if(thread){
		if(	thread->thread ){
			if(thread->running){
				if(!thread->joined)
					pthread_join(thread->thread, NULL);
				if(thread->started)
					pthreads_destroy_event(thread->started);
			}
		}
		if(thread->std.properties){
			zend_hash_destroy(
				thread->std.properties
			);
			FREE_HASHTABLE(thread->std.properties);
		}
		free(thread);
	}
}

void * PHP_PTHREAD_ROUTINE(void *arg){
	PTHREAD 	thread = (PTHREAD) arg;
	/* I can't imagine a way for this condition to be false ... */
	if(thread){
		/* this should always be true, can't hurt to check */
		if(thread->runnable){
			/* copy symbols from class to thread scope while it is locked on event */
			HashTable symbols; {
				zval *tz;
				zend_hash_init(&symbols, 100, NULL, ZVAL_PTR_DTOR, 0);
				zend_hash_copy(
					&symbols, thread->std.properties, NULL, &tz, sizeof(zval*)
				);
			}
			/* allow parent thread to continue */
			pthreads_fire_event(thread->started); {
				void *oldctx;
				/* allocate new interpreter context */
				void ***newctx = tsrm_new_interpreter_context(); {
					/* set context */
					oldctx = tsrm_set_interpreter_context(
						newctx
					);
					/* switch to new context local storage */
					TSRMLS_FETCH_FROM_CTX(newctx);
					/* activate zend for execution of Thread::run */
					zend_activate(TSRMLS_C);
					/* activate extensions */
					zend_activate_modules(TSRMLS_C);
					zend_first_try {
						/* prepare method for execution */
						zend_op_array *op_array = (zend_op_array*) thread->runnable;
						/* return value for method */
						zval *returned;
						/* allocate return value, it is not used but the VM likes it */
						ALLOC_ZVAL(returned);
						/* set execution flag */
						EG(in_execution) = 1;
						/* set pointer to the pointer to the return value */
						EG(return_value_ptr_ptr)=&returned;
						/* set active op array ( Thread::run ) */
						EG(active_op_array)=op_array;
						/* no current execute data */
						EG(current_execute_data)=NULL;
						/* set active symbol table to scoped symbols */
						EG(active_symbol_table)=&symbols;
						/* have another go at getting better scope */
						EG(called_scope)=pthreads_class_entry;
						EG(scope)=pthreads_class_entry;
						/* do it */
						zend_execute(EG(active_op_array) TSRMLS_CC);
						/* destroy return value */
						if(returned){
							INIT_ZVAL(thread->result);
							thread->result = *returned;
							zval_copy_ctor(
								&thread->result
							);
							zval_ptr_dtor(
								&returned
							);
						}
					} zend_catch {
						/* report some error somewhere */
					} zend_end_try();
					zend_deactivate_modules(TSRMLS_C);
					/* shutdown modules */
					zend_deactivate(TSRMLS_C);
					/* free interpreter context */
					tsrm_free_interpreter_context(newctx);
				}
			}
		} else pthreads_fire_event(thread->started);
	} else pthreads_fire_event(thread->started);
	/* leave thread */
	pthread_exit((void*)0);
}
#endif
