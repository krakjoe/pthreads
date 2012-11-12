--TEST--
Test constants (bug 19)
--DESCRIPTION--
This test verifies that constant members in declarations made outside of threads are available inside threads without error
--FILE--
<?php
class TestThread extends Thread {
	public function run() { printf("%s\n", DateTime::ISO8601 ); }
}

$thread = new TestThread();
$thread->start();
?>
--EXPECT--
Y-m-d\TH:i:sO
