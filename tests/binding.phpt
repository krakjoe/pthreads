--TEST--
Test pthreads connections
--DESCRIPTION--
This test verifies that variables are bound properly by pthreads
--FILE--
<?php
class ThreadTesting extends Thread {
	public $other;
	public $done;
	
	public function setOther($other){
		$this->other = $other;
	}
	public function run(){
		if ($this->other) {
			printf("%s\n", __METHOD__);
		}
		$this->isDone(true);
		$this->notify();
	}
	
	protected function isDone($done = null) {
		if ($done === null)
			return $this->done;
		return ($this->done = $done);
	}
}

class ThreadTest extends Thread {
	public $other;
	public $done;
	
	public function setOther($other){
		$this->other = $other;
	}
	public function run(){
		if ($this->other) {
			printf("%s\n", __METHOD__);
		}
		$this->isDone(true);
		$this->notify();
	}
	
	protected function isDone($done = null) {
		if ($done === null)
			return $this->done;
		return ($this->done = $done);
	}
}

$threads[0]=new ThreadTesting();
$threads[1]=new ThreadTest();
$threads[0]->setOther($threads[1]);
$threads[1]->setOther($threads[0]);
foreach($threads as $thread)
	$thread->start();
foreach($threads as $thread) {
	if (!$thread->isDone())
		$thread->wait();
}	
?>
--EXPECT--
ThreadTesting::run
ThreadTest::run
