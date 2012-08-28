#ifndef HAVE_PHP_PTHREADS_H
#define HAVE_PHP_PTHREADS_H
#define PHP_PTHREADS_EXTNAME "pthreads"
#define PHP_PTHREADS_VERSION "0.01"

extern zend_module_entry pthreads_module_entry;
#define phpext_pthreads_ptr &pthreads_module_entry

PHP_MINIT_FUNCTION(pthreads);							/* Initialize class entries and default attributes */
PHP_MSHUTDOWN_FUNCTION(pthreads);						/* Destroy default attributes */

PHP_METHOD(Thread, start);								/* Userland method to start a Thread */
PHP_METHOD(Thread, self);								/* Userland method to get current Thread identifier */
PHP_METHOD(Thread, wait);								/* Userland method to wait for a Thread */

PHP_METHOD(Mutex, create);								/* Userland mutex constructor */
PHP_METHOD(Mutex, lock);								/* Userland method to lock mutex */
PHP_METHOD(Mutex, trylock);								/* Userland method to try mutex lock */
PHP_METHOD(Mutex, unlock);								/* Userland method to unlock mutex */
PHP_METHOD(Mutex, destroy);								/* Userland method to destroy mutex */					

typedef struct _pthread_event {							/* Event internal structure */
	pthread_mutex_t			*lock;						/* mutex for condition */
	pthread_cond_t			*cond;						/* event condition */
	int						fired;						/* flag fired for destruction */
} EVENT, *PEVENT;

typedef struct _pthread_construct {						/* Thread internal structure */
	zend_object 			std;						/* standard entry */
	pthread_t				thread;						/* internal thread identifier */
	PEVENT					started;					/* started event */
	zend_function			*runnable;					/* the run method as defined by user */
	zval					result;						/* result from thread */
	void					***pls;						/* the local storage of the thread/process creating this thread */
	int						running;					/* flag to indicate thread was started */
	int						joined;						/* flag to indicate thread was joined */
} THREAD, *PTHREAD;

PEVENT 	pthreads_create_event();						/* Creates an Event for Thread */
void 	pthreads_wait_event(PEVENT event);				/* Waits for Event to be fired */
void 	pthreads_fire_event(PEVENT event);				/* Fires an Event */
void 	pthreads_destroy_event(PEVENT event);			/* Destroys an Event */

void * PHP_PTHREAD_ROUTINE(void *);						/* Is pretty ruddy obvious .. */


#define PTHREADS_FETCH_FROM_EX(object, ls)				(PTHREAD) zend_object_store_get_object(object, ls)
#define PTHREADS_FETCH_FROM(object)						(PTHREAD) zend_object_store_get_object(object TSRMLS_CC)
#define PTHREADS_FETCH									(PTHREAD) zend_object_store_get_object(this_ptr TSRMLS_CC)

#define PTHREADS_FETCH_CTX(ls, id, type, element)		(((type) (*((void ***) ls))[TSRM_UNSHUFFLE_RSRC_ID(id)])->element)
#define PTHREADS_CG(ls, v)								PTHREADS_FETCH_CTX(ls, compiler_globals_id, zend_compiler_globals*, v)
#define PTHREADS_EG(ls, v)								PTHREADS_FETCH_CTX(ls, executor_globals_id, zend_executor_globals*, v)
#endif
