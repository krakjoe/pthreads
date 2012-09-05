--TEST--
Test access to user defined methods in the object context
--DESCRIPTION--
User methods are now imported from your declared class into the thread
--FILE--
<?php
class ThreadTest extends Thread {
	public function objectTest(){
		return 1;
	}
	
	public function run(){
		return $this->objectTest();
	}
}
$thread = new ThreadTest();
if($thread->start())
	var_dump($thread->join());
?>
--EXPECT--
int(1)