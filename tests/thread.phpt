--TEST--
Test basic threading
--DESCRIPTION--
This test will create and join a simple thread
--FILE--
<?php
class ThreadTest extends Thread {
	public function run(){
		$i=0;
		while(++$i<100){
			continue;
		}
		return $i;
	}
}
$thread = new ThreadTest();
if($thread->start())
	var_dump($thread->join());
?>
--EXPECT--
int(100)