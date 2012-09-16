<?php
class ScopeTest extends Thread {
	public function run(){
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		printf("%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
	}
}

class ScopeTest2 extends Thread {
	public function __construct($other){
		$this->other = $other;
	}
	
	public function run(){
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		if (($other = Thread::getThread($this->other))) {
			printf("%s: %lu discovering if %lu is running: %d\n", __CLASS__, $this->getThreadId(), $this->other, $other->isRunning());
			printf("%s: %lu discovering if %lu is joined: %d\n", __CLASS__, $this->getThreadId(), $this->other, $other->isJoined());
			printf("%s: %lu notifying %lu: %d\n", __CLASS__, $this->getThreadId(), $this->other, $other->notify());
		} else printf("%s: %lu failed to find %lu\n", __CLASS__, $this->getThreadId(), $this->other);
		printf("%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
	}
}

printf("Process: running\n");
$test = new ScopeTest();
$test->start();
$test2 = new ScopeTest2($test->getThreadId());
$test2->start();

usleep(100000);

printf("Process: notifying %lu: %d\n", $test->getThreadId(), $test2->notify());
?>