/*
  +----------------------------------------------------------------------+
  | pthreads                                                             |
  +----------------------------------------------------------------------+
  | Copyright (c) Joe Watkins 2012                               		 |
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
#ifndef HAVE_PTHREADS_CLASS_COND_H
#define HAVE_PTHREADS_CLASS_COND_H
PHP_METHOD(Cond, create);
PHP_METHOD(Cond, signal);
PHP_METHOD(Cond, broadcast);
PHP_METHOD(Cond, wait);
PHP_METHOD(Cond, destroy);

ZEND_BEGIN_ARG_INFO_EX(Cond_create, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_signal, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_wait, 0, 0, 2)
	ZEND_ARG_INFO(0, condition)
	ZEND_ARG_INFO(0, mutex)
	ZEND_ARG_INFO(0, timeout)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_broadcast, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Cond_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, condition)
ZEND_END_ARG_INFO()

zend_function_entry pthreads_condition_methods[] = {
	PHP_ME(Cond, create, Cond_create, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, signal, Cond_signal, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, wait, Cond_wait, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, broadcast, Cond_broadcast, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(Cond, destroy, Cond_destroy, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	{NULL, NULL, NULL}
};
#else
#	ifndef HAVE_PTHREADS_CLASS_COND
#	define HAVE_PTHREADS_CLASS_COND
/* {{{ proto long Cond::create() 
		This will create a condition, because of their nature you need to destroy these explicitly with Cond::destroy */
PHP_METHOD(Cond, create)
{
	pthread_cond_t *condition;
	
	if ((condition=(pthread_cond_t*) calloc(1, sizeof(pthread_cond_t)))!=NULL) {
		switch (pthread_cond_init(condition, NULL)) {
			case SUCCESS: 
				RETURN_LONG((ulong)condition); 
			break;
			
			case EAGAIN:
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary resources (other than memory) to initialise another condition"); 
				free(condition);
			break;
			
			case ENOMEM: /* I would imagine we would fail to write this message to output if we are really out of memory */
				zend_error(E_ERROR, "pthreads detected that the system lacks the necessary memory to initialise another condition"); 
				free(condition);
			break;
			
			default: 
				zend_error(E_ERROR, "pthreads detected an internal error while initializing new condition and cannot conitnue");
				free(condition);
		}
	} else zend_error(E_ERROR, "pthreads failed to allocate memory for new condition and cannot continue");
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::signal(long condition) 
		This will signal a condition */
PHP_METHOD(Cond, signal)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition) == SUCCESS && condition) {
		switch (pthread_cond_signal(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while signaling condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::broadcast(long condition) 
		This will broadcast a condition */
PHP_METHOD(Cond, broadcast)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition)==SUCCESS && condition) {
		switch (pthread_cond_broadcast(condition)) {
			case SUCCESS: RETURN_TRUE; break;
			
			case EINVAL:
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while broadcasting condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::wait(long condition, long mutex, [long timeout]) 
		This will wait for a signal or broadcast on condition, you must have mutex locked by the calling thread
		Timeout should be expressed in microseconds ( millionths ) 
		@NOTE timeouts dodgy in windows because of a lack of proper gettimeofday implementation, dispite it being defined, somewhere ... */
PHP_METHOD(Cond, wait)
{
	pthread_cond_t *condition;
	pthread_mutex_t *mutex;
	long timeout = 0L;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &condition, &mutex, &timeout)==SUCCESS && condition && mutex) {
		if (timeout > 0L) {
			struct timeval now;
			if (gettimeofday(&now, NULL)==SUCCESS) {
				struct timespec until;
				long	nsec = timeout * 1000;
				if (nsec > 1000000000L) {
					until.tv_sec = now.tv_sec + (nsec / 1000000000L);
					until.tv_nsec = (now.tv_usec * 1000) + (nsec % 1000000000L);
				} else {
					until.tv_sec = now.tv_sec;
					until.tv_nsec = (now.tv_usec * 1000) + timeout;	
				}
				
				switch(pthread_cond_timedwait(condition, mutex, &until)){
					case SUCCESS: RETURN_TRUE; break;
					case EINVAL: 
						zend_error(E_WARNING, "pthreads has detected that the condition you are attempting to wait on does not refer to a valid condition variable");
						RETURN_FALSE;
					break;
					
					case ETIMEDOUT: 
						zend_error(E_WARNING, "pthreads detected a timeout while waiting for condition"); 
						RETURN_FALSE;
					break;
					
					default: RETURN_FALSE;
				}
			} else {
				zend_error(E_ERROR, "pthreads has detected a failure while attempting to get the time from the system");
				RETURN_FALSE;
			}
		} else {
			switch (pthread_cond_wait(condition, mutex)) {
				case SUCCESS:  RETURN_TRUE; break;
				
				case EINVAL: 
					zend_error(E_WARNING, "pthreads has detected that the condition you are attempting to wait on does not refer to a valid condition variable");
					RETURN_FALSE;
				break;
				
				default: RETURN_FALSE;
			}
		}
	}
	RETURN_FALSE;
} /* }}} */

/* {{{ proto boolean Cond::destroy()
		This will destroy a condition and free the memory associated with it */
PHP_METHOD(Cond, destroy)
{
	pthread_cond_t *condition;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &condition)==SUCCESS && condition) {
		switch (pthread_cond_destroy(condition)) {
			case SUCCESS: 
				free(condition);
				RETURN_TRUE; 
			break;
			
			case EINVAL: 
				zend_error(E_WARNING, "pthreads has detected the condition referenced does not refer to a valid condition variable"); 
			break;
			
			case EBUSY:
				zend_error(E_WARNING, "pthreads has detected an attempt to destroy the object referenced by condition while it is referenced by another thread"); 
			break;
			
			default:
				zend_error(E_ERROR, "pthreads detected an internal error while destroying condition and cannot continue");
		}
	} 
	RETURN_FALSE;
} /* }}} */
#	endif
#endif
