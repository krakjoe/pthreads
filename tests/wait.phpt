--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends Thread {
	public function run(){
		return $this->wait();
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	usleep(1000*1000); /* simulate some work */
	var_dump($thread->notify()); /* should return boolean true */
	var_dump($thread->join());	 /* should return int 1 */
}
?>
--EXPECT--
boolean(true)
int(1)