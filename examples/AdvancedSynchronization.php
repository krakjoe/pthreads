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
			printf("%s: %lu working ...\n", __CLASS__, $this->getThreadId());
			usleep(1000000); /* simulate some work */
			if ($other->isWaiting())
				printf("%s: %lu notifying %lu: %d\n", __CLASS__, $this->getThreadId(), $this->other, $other->notify());
		} else {
			printf("%s: %lu failed to find %lu\n", __CLASS__, $this->getThreadId(), $this->other);
		};
		printf("%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
	}
}

printf("Process: running\n");
$test = new ScopeTest();
$test->start();
$test2 = new ScopeTest2($test->getThreadId());
$test2->start();

printf("Process: notifying %lu: %d\n", $test2->getThreadId(), $test2->notify());

/*
* If importing has been disabled then the responsability to notify the waiting thread must fall back to the Process to avoid deadlock
*/
printf("Process: notifying %lu: %d\n", $test->getThreadId(), $test->notify());
?>