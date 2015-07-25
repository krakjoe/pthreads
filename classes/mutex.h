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
#ifndef HAVE_PTHREADS_CLASS_MUTEX_H
#define HAVE_PTHREADS_CLASS_MUTEX_H

PHP_METHOD(Mutex, __construct);
PHP_METHOD(Mutex, create);
PHP_METHOD(Mutex, lock);
PHP_METHOD(Mutex, trylock);
PHP_METHOD(Mutex, unlock);
PHP_METHOD(Mutex, destroy);

ZEND_BEGIN_ARG_INFO_EX(Mutex_construct, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_create, 0, 0, 0)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_lock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_trylock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_unlock, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
	ZEND_ARG_INFO(0, destroy)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Mutex_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, mutex)
ZEND_END_ARG_INFO()

extern zend_function_entry pthreads_mutex_methods[];
#else
#	ifndef HAVE_PTHREADS_CLASS_MUTEX
#	define HAVE_PTHREADS_CLASS_MUTEX
zend_function_entry pthreads_mutex_methods[] = {
	PHP_ME(Mutex, __construct, Mutex_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR|ZEND_ACC_FINAL)
	PHP_ME(Mutex, create, Mutex_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, lock, Mutex_lock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, trylock, Mutex_trylock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, unlock, Mutex_unlock, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Mutex, destroy, Mutex_destroy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};

/* {{{ proto void Mutex::__construct() */
PHP_METHOD(Mutex, __construct) 
{
	zend_throw_exception_ex(
		spl_ce_RuntimeException, 0, 
		"pthreads has detected an attempt to incorrectly create a Mutex, please refer to the PHP manual");
} /* }}} */

/* {{{ proto long Mutex::create([boolean lock]) 
	Will create a new mutex and return it's handle. If lock is true it will lock the mutex before returning the handle to the calling thread.
	Because of their nature you need to destroy these explicitly with Mutex::destroy */
PHP_METHOD(Mutex, create)
{
	zend_bool lock;
	pthread_mutex_t *mutex;
	int rc = SUCCESS;
	
	if ((mutex=(pthread_mutex_t*) calloc(1, sizeof(pthread_mutex_t)))!=NULL) {
		switch((rc = pthread_mutex_init(mutex, NULL))){
			case SUCCESS: 
				if (ZEND_NUM_ARGS()) {		
					if (zend_parse_parameters(ZEND_NUM_ARGS(), "b", &lock)==SUCCESS) {
						if (lock) {
							switch((rc = pthread_mutex_lock(mutex))){
								case SUCCESS: RETURN_LONG((zend_long)mutex); break;
								case EDEADLK: RETURN_LONG((zend_long)mutex); break;
								
								default:
									zend_throw_exception_ex(
										spl_ce_RuntimeException, rc,
										"pthreads detected an internal error while attempting to lock new mutex");
									pthread_mutex_destroy(mutex);
									free(mutex);
							}
						} else { RETURN_LONG((zend_long)mutex); }
					}
				} else { RETURN_LONG((zend_long)mutex); }
			break;
		
			case EAGAIN:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EAGAIN, 
					"pthreads detected that the system lacks the necessary resources (other than memory) to initialise another mutex");
				free(mutex);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_throw_exception_ex(
					spl_ce_RuntimeException, ENOMEM, 
					"pthreads detected that the system lacks the necessary memory to initialise another mutex");
				free(mutex);
			break;
			
			case EPERM:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EPERM, 
					"pthreads detected that the caller does not have permission to initialize mutex");
				free(mutex);
			break;
			
			default: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, rc, 
					"pthreads detected an internal error while attempting to initialize mutex");
				free(mutex);
		}
	} else {
		zend_throw_exception_ex(
			spl_ce_RuntimeException, 0,
			"pthreads failed to allocate memory for new mutex"); 
	}
} /* }}} */

/* {{{ proto boolean Mutex::lock(long mutex) 
		Locks the mutex referenced so that the calling thread owns the mutex */
PHP_METHOD(Mutex, lock)
{
	pthread_mutex_t *mutex;
	int rc = SUCCESS;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mutex)==SUCCESS) {
		switch ((rc = pthread_mutex_lock(mutex))) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EINVAL, 
					"pthreads has detected that the variable passed is not a valid mutex");
			break;
			
			case EAGAIN: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EAGAIN,
					"The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded");
			break;
			
			default: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, rc,
					"pthreads detected an internal error while locking mutex");
		}
	}
} /* }}} */

/* {{{ proto boolean Mutex::trylock(long mutex) 
		Will attempt to lock the mutex without causing the calling thread to block */
PHP_METHOD(Mutex, trylock)
{
	pthread_mutex_t *mutex;
	int rc = SUCCESS;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mutex)==SUCCESS && mutex) {
		switch ((rc = pthread_mutex_trylock(mutex))) {
			case SUCCESS: RETURN_TRUE; break;
			case EBUSY: RETURN_FALSE; break;
			
			case EINVAL: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EINVAL, 
					"pthreads has detected that the variable passed is not a valid mutex");
			break;
			
			case EAGAIN: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EAGAIN, 
					"pthreads detected that the mutex could not be acquired because the maximum number of recursive locks has been exceeded");
			break;
			
			default:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, rc, 
					"pthreads detected an internal error while trying to lock mutex");
		}
	}
} /* }}} */

/* {{{ proto boolean Mutex::unlock(long mutex, [bool destroy]) 
		Will unlock the mutex referenced, and optionally destroy it */
PHP_METHOD(Mutex, unlock)
{
	pthread_mutex_t *mutex;
	zend_bool destroy = 0;
	int rc = SUCCESS;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|b", &mutex, &destroy)==SUCCESS && mutex) {
		switch ((rc = pthread_mutex_unlock(mutex))) {
			case SUCCESS: 
				if (destroy > 0) {
					pthread_mutex_destroy(mutex);
					free(mutex);
				}
				
				RETURN_TRUE; 
			break;
			
			case EINVAL: 
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EINVAL, 
					"pthreads has detected that the variable passed is not a valid mutex");
			break;
			
			case EPERM:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EPERM, 
					"pthreads has detected that the calling thread does not own the mutex");
			break;
			
			default:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, rc, 
					"pthreads detected an internal error while unlocking mutex");
		}
	}
} /* }}} */

/* {{{ proto boolean Mutex::destroy(long mutex)
		Will destroy the mutex referenced and free memory associated */
PHP_METHOD(Mutex, destroy)
{
	pthread_mutex_t *mutex;
	int rc = SUCCESS;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mutex)==SUCCESS && mutex) {
		switch ((rc = pthread_mutex_destroy(mutex))) {
			case SUCCESS: 
				free(mutex);
				RETURN_TRUE;
			break;
			
			case EINVAL:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EINVAL, 
					"pthreads has detected that the variable passed is not a valid mutex");
			break;
			
			case EBUSY:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, EBUSY, 
					"pthreads has detected an attempt to destroy mutex while it is locked or referenced");
			break;
			
			default:
				zend_throw_exception_ex(
					spl_ce_RuntimeException, rc, 
					"pthreads detected an internal error while attempting to destroy mutex");
		}
	}
} /* }}} */

#	endif
#endif
