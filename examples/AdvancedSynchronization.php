<?php
class SyncTest extends Thread {
	public function scopeTestFunc(){
		return strlen($this->my)*rand();
	}
	
	public function __construct(){
		$this->start();
	}
	
	public function run(){
		/* set a variable to test fetching from an imported thread */
		$this->my = "data";
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		printf("%s: %lu waiting\n", __CLASS__, $this->getThreadId());
		printf("%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
		printf("%s: %lu leaving\n", __CLASS__, $this->getThreadId());
		$this->my = strrev($this->my);
	}
}

class Tester extends Thread {
	public function __construct($tests){
		$this->tests = $tests;
	}
	
	public function run(){
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		print_R($this);
		foreach($this->tests as $tid) {
			if (($test = Thread::getThread($tid))) {
				printf("%s: %lu working %lu ... %s/%d\n", __CLASS__, $this->getThreadId(), $tid, $test->my, $test->scopeTestFunc());
				if ($test->isWaiting())
					printf("%s: %lu notifying %lu: %d\n", __CLASS__, $this->getThreadId(), $tid, $test->notify());
			} else printf("%s: %lu failed to find %s\n", __CLASS__, $this->getThreadId(), $tid);
		}
		printf("%s: %lu notifying process: %d\n", __CLASS__, $this->getThreadId(), $this->notify());
	}
}

printf("Process: running\n");
$tests = array(new SyncTest(), new SyncTest());
$tester = new Tester(array($tests[0]->getThreadId(), $tests[1]->getThreadId()));
$tester->start();
$tester->wait();
printf("Process: notified\n");
foreach($tests as $test){
	if(!$test->isJoined() && $test->isWaiting()) {
		printf("Process: notifying %lu\n", $test->getThreadId());
		printf("Process: notified %lu: %d\n", $test->getThreadId(), $test->notify());
	} else printf("Process: done with %lu\n", $test->getThreadId());
}
printf("Process: notifying %lu\n", $tester->getThreadId());
?>