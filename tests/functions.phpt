--TEST--
Test function table inheritance
--DESCRIPTION--
This test verifies that the function table is inherited by pthreads correctly
--FILE--
<?php
function TestFunction(){
	return __FUNCTION__;
}

class TestThread extends Thread {
	public function run() { 
		printf("%s\n", TestFunction()); 
	}
}

$thread = new TestThread();
$thread->start();
?>
--EXPECT--
TestFunction
