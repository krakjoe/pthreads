<?php
class SyncTest extends Thread {
	/*
	* This protected method can only be called by one context at a time
	*/
	protected function scopeTestFunc(){
		return strlen($this->my)*rand();
	}
	
	public function __construct(){
		$this->start();
	}
	
	public function run(){
		$stdout = fopen("php://stdout", "w");
		/* set a variable to test fetching from an imported thread */
		$this->my = "data";
		fprintf($stdout, "%s: %lu running\n", __CLASS__, $this->getThreadId());
		fprintf($stdout, "%s: %lu waiting\n", __CLASS__, $this->getThreadId());
		fprintf($stdout, "%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
		fprintf($stdout, "%s: %lu read %s\n", __CLASS__, $this->getThreadId(), $this->reading);
		fprintf($stdout, "%s: %lu leaving\n", __CLASS__, $this->getThreadId());
		fflush($stdout);
		$this->my = strrev($this->my);
	}
}

class Tester extends Thread {
	public function __construct($tests){
		$this->tests = $tests;
	}
	
	public function run(){
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		
		foreach($this->tests as $tid) {
			if (($test = Thread::getThread($tid))) {
				printf("%s: %lu working %lu ... %s/%d\n", __CLASS__, $this->getThreadId(), $tid, $test->my, $test->scopeTestFunc());
				$test->reading = ((rand()*10)*microtime(true))/10;
				if ($test->isWaiting()) {
					printf("%s: %lu notifying %lu: %d\n", __CLASS__, $this->getThreadId(), $tid, $test->notify());
				} 
					
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
	printf("Process: read %lu: %s\n", $test->getThreadId(), $test->reading);
}
printf("Process: notifying %lu\n", $tester->getThreadId());
?>