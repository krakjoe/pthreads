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
	}
	
	/* we are just ensuring the correct order of output */
	/* else the tests fail */
	protected function isDone($done = null) {
		if (!$done)
			return $this->done;
		return ($this->done = $this->notify());
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
	}
	/* we are just ensuring the correct order of output */
	/* else the tests fail */
	protected function isDone($done = null) {
		if (!$done)
			return $this->done;
		return ($this->done = $this->notify());
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
