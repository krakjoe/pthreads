--TEST--
Test inherit none closures
--DESCRIPTION--
This test verifies that closures work when using PTHREADS_INHERIT_NONE
--FILE--
<?php
class Test extends Thread {
	public function run() {
		$this->synchronized(function(){
			echo "OK\n";
		});
	}
}

$test = new Test();
$test->start(PTHREADS_INHERIT_NONE);
$test->join();
--EXPECT--
OK
