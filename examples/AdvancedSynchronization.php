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
	
	/*
	* This doesn't do anything useful, does a bit of read/writing and waiting ...
	* Uninteresting, move along ...
	*/ 
	public function run(){
		$stdout = fopen("php://stdout", "w");
		/* set a variable to test fetching from an imported thread */
		$this->my = "data";
		fprintf($stdout, "%s: %lu running\n", __CLASS__, $this->getThreadId());
		fprintf($stdout, "%s: %lu waiting\n", __CLASS__, $this->getThreadId());
		/* wait for someone to write some data if necessary */
		if (!$this->isFinished())
			fprintf($stdout, "%s: %lu notified: %d\n", __CLASS__, $this->getThreadId(), $this->wait());
		fprintf($stdout, "%s: %lu read %s\n", __CLASS__, $this->getThreadId(), $this->reading);
		fprintf($stdout, "%s: %lu leaving\n", __CLASS__, $this->getThreadId());
		fflush($stdout);
		$this->my = strrev($this->my);
	}
	
	/*
	* This is a good example of using a protected method:
	*	The nature of wait() means a condition has failed, ie 	
	*		if (conditionFailed()) 
				wait()
			else noNeedToWait();
	*	Using protected here gives the finished flag authority
	*	because it is the only method that read/writes the variable
	*	Calling notify from the getter/setter means that if another context
	*	calls isFinished(true) and this object is waiting at the time of the call
	*	it will get the notification it is waiting for
	*/
	protected function isFinished($flag = null) {
		if ($flag === null) {
			return $this->finished;
		} else {
			$this->finished = $flag;
			if ($this->isWaiting())
				$this->notify();
			return $this->finished;
		}
	}
}

class Tester extends Thread {
	public function __construct($tests){
		$this->tests = $tests;
	}
	
	public function run(){
		printf("%s: %lu running\n", __CLASS__, $this->getThreadId());
		foreach($this->tests as $test) {
			/* this tests protected method access on a nonsense method, and reads some data directly */
			printf("%s: %lu working %lu ... %s/%d\n", __CLASS__, $this->getThreadId(), $test->getThreadId(), $test->my, $test->scopeTestFunc());
			/* this is us directly writing another threads members, just for something to do */
			$test->reading = ((rand()*10)*microtime(true))/10;
			/* tell the test thread we are done and it can read what was written now */
			$test->isFinished(true);
		}
		/* the process is waiting to handle failed tests */
		printf("%s: %lu notifying process: %d\n", __CLASS__, $this->getThreadId(), $this->notify());
	}
}

printf("Process: running\n");
$tests = array(new SyncTest(), new SyncTest());
$tester = new Tester($tests);
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