<?php
/*
* 	Pooling is the clever thing to do with threads ...
*	
*	Just like CallAnyFunction this is an extremely easy thing to implement in userspace, and control is retained over how the pool works ...
*	for this reason, pthreads doesn't include pooling in the API ...
*
*	We often think of a language as high or low level, PHP doesn't fit into either of those two categories very well
*	Lots of functions we're used to using in PHP are strikingly similar, if not exactly the same as their C counterparts, which gives it a low level feel ...
*	The threading API is similar to __insert_high_level_language__, but infact we are still in the middle. In both high and low level languages your threads share memory
*	The architecture we are executing in prohibits that from occuring, agressively ( it forms the thread safety used to execute in some SAPI's and indeed execute pthreads )
*	When a thread is started it is allocated a context, to allocate that context and throw it away after a file_get_contents or a few instructions is wasteful, especially
*	if the programmer knows they will soon need to execute another Thread.
*	pthreads includes a Worker Thread like functionality, and this can be built upon in the user space to pool threads that you have created in order to reuse their contexts.
*	
*	I insist, give it a go ... this is less than 100 lines, and is mostly babbling ...
*/

class MyWorker extends Thread {
	public function run(){
		if($this->attempt>1)
			usleep(rand(10000, 20000));
		printf("%s: %lu running: %d/%d ...\n", __CLASS__, $this->getThreadId(), $this->attempt++, $this->getStacked());
	}
}

class MyWorkerPool {
	public $size = 0;
	public $workers = array();
	public $ids = array();
	
	/*
	* Will create a pool of $size threads
	*/
	public function __construct($size) {
		$this->size = $size;
		while($start++<$this->size){
			$this->workers[]=new MyWorker();
		}
	}
	
	/*
	* Will stack on the first thread not running, or waiting
	* If all threads are busy it is stacked at random
	* In the real world the logic will be more complicated
	*/
	public function submit($object){
		$working = 0;
		foreach($this->workers as $id => $worker) {
			if (!$worker->isStarted() || $worker->isWaiting()){
				if ($worker->stack($object)) {
					if ($worker->isStarted() || $worker->start()) {
						$this->ids[($working=$id)]=$worker->getThreadId();
						break;
					}
				}
			}
		}
		if (!$working)
			$this->workers[$working=array_rand($this->workers, 1)]->stack($object);
		return $working;
	}
	
	/*
	* Will shutdown the pool gracefully
	*/
	public function shutdown(){
		foreach($this->workers as $id => $worker) {
			if ($worker->isStarted())
				$worker->join();
		}
	}
}

/*
* Create a pool of ten threads
*/
$pool = new MyWorkerPool(10);

/*
* Submit 100 tasks to the pool
*/
while(++$target<100) {
	$pool->submit(new MyWorker());
}

/*
* Shutdown the pool
*/
$pool->shutdown();

/*
* Look inside ...
*/
foreach($pool->workers as $worker) {
	printf("Thread %lu made %d attempts ...\n", $worker->getThreadId(), $worker->attempt);
}
?>