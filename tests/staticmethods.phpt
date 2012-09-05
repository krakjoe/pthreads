--TEST--
Test access to static methods from within user threads
--DESCRIPTION--
Static methods as declared in the users implementation of Thread should now be available for calling in the thread scope
--FILE--
<?php
class ThreadTest extends Thread {
	public static function staticTest(){
		return 1;
	}
	
	public function run(){
		return self::staticTest();
	}
}
$thread = new ThreadTest();
if($thread->start())
	var_dump($thread->join());
?>
--EXPECT--
int(1)