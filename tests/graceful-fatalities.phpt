--TEST--
Test graceful fatalities
--DESCRIPTION--
This test verifies that fatalities are graceful with regard to state
--FILE--
<?php
class TestThread extends Thread {
	public function run(){
		/* silent fatal error */
		echo @MY::$FATAL;
	}
}
$test = new TestThread();
$test->start();
usleep(100000);
var_dump($test->isRunning());
?>
--EXPECTF--
bool(false)
