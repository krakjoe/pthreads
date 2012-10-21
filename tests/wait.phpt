--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends Thread {
	public function run(){
		return $this->notify();
	}
}
$thread = new ThreadTest();
if($thread->start(true)) {
	var_dump($thread->join());	 /* should return int 1 */
}
?>
--EXPECT--
int(1)