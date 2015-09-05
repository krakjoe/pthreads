--TEST--
Check override of sleep and usleep
--DESCRIPTION--
sleep() and usleep() are not suitable for use in multi-threaded applications
therefore pthreads disallows their invokation.

proper synchronization should *always* be used, *no exceptions*
--FILE--
<?php
try {
	usleep(1);
} catch(Exception $ex) {
	var_dump($ex->getMessage());
}

try {
	sleep(1);
} catch(Exception $ex) {
	var_dump($ex->getMessage());
}
--EXPECT--
string(94) "usleep is not suitable for use in multi threaded applications, use synchronized Threaded::wait"
string(93) "sleep is not suitable for use in multi threaded applications, use synchronized Threaded::wait"

