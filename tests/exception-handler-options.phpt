--TEST--
Test exception handler inheritance (options)
--DESCRIPTION--
PTHREADS_INHERIT_CLASSES and PTHREADS_INHERIT_FUNCTIONS effect the handlers that can be copied.

This test ensures that the various options are sensibly covered, and hopefully nothing crashes.
--FILE--
<?php
class Handler {
	public function handle(Exception $ex) {
		var_dump(__METHOD__, $ex->getMessage());
	}
}

class Test extends Thread {
	public function run () {
		throw new Exception("oh dear");
	}
}

$handler = new Handler();

set_exception_handler([$handler, "handle"]);	# needs classes

$test = new Test();
$test->start(PTHREADS_INHERIT_FUNCTIONS);		# uncaught
$test->join();

$test = new Test();
$test->start(PTHREADS_INHERIT_CLASSES);			# caught		
$test->join();

restore_exception_handler();

set_exception_handler(function($ex){
	var_dump(__FUNCTION__, $ex->getMessage());
});

$test = new Test();
$test->start(PTHREADS_INHERIT_FUNCTIONS); 		# caught
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught Exception: oh dear in %s:10
Stack trace:
#0 [internal function]: Test->run()
#1 {main}
  thrown in %s on line 10
string(15) "Handler::handle"
string(7) "oh dear"
string(9) "{closure}"
string(7) "oh dear"


