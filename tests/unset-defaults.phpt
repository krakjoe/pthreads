--TEST--
Test unset defaults
--DESCRIPTION--
This test verifies that unset members do not cause a problem in pthreads objects
--FILE--
<?php
class TestThread extends Thread {
	public $default;
	
	public function run() { var_dump($this->default); }
}

$thread = new TestThread();
$thread->start();
?>
--EXPECT--
