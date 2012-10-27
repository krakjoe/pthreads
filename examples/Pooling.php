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
*	pthreads includes a Worker Thread functionality, and this can be built upon in the user space to pool threads that you have created in order to reuse their contexts.
*	
*	I insist, give it a go ... this is less than 100 lines, and is mostly babbling ...
*/

/* a stackable piece of work */
class ExampleWork extends Stackable {
	public function __construct(){
		$this->worker = 0;
	}
	
	protected function setWorker($wid){ $this->worker = $wid; }
	protected function getWorker() { return $this->worker; }
	
	/* do anything simple */
	public function run(){
		$this->wait();
		if (($worker = Thread::getThread($this->getThreadId()))) {
			$worker->addAttempt();
			printf(
				"%s executing in %s on attempt %d\n",
				__CLASS__, $worker->getName(), $worker->getAttempts()
			);
		} else printf("Failed to find the Thread #%lu in the current context ...\n", $this->worker);
	}
}

/* a worker thread */
class ExampleWorker extends Worker {
	public function __construct($name){
		$this->name = $name;
		$this->init = false;
		$this->attempts = 0;
	}
	/* the only thing to do here is initialize an environment which is conducive to the stackables you'll be stacking ... */
	public function run(){
		if (!$this->init) {
			$this->init = true;
			if (isset($this->name))
				$this->name = sprintf("%s (%lu)", __CLASS__, $this->getThreadId());
		}
	}
	/* get name of the worker */
	public function getName() { return $this->name; }
	public function addAttempt(){ $this->attempts = ($this->attempts+1); }
	public function getAttempts() { return $this->attempts; }
}

/*
* Dead simple pthreads pool
*/
class Pool {
	/* to hold worker threads */
	public $workers;
	/* to hold exit statuses */
	public $status;
	/* prepare $size workers */
	public function __construct($size = 10) {
		$this->size = $size;
	}
	/* submit Stackable to Worker */
	public function submit(Stackable $stackable) {
		if (count($this->workers)<$this->size) {
			$this->workers[] = $worker = new ExampleWorker(sprintf("Worker [%d]", count($this->workers)-1));
			$worker->start();
			if ($worker->stack($stackable)) {
				$stackable->setWorker($worker->getThreadId());
				if($stackable->notify())
					return $stackable;
			} else trigger_error(sprintf("failed to push Stackable onto %s", $worker->getName()), E_USER_WARNING);
		}
		
		if (($select = $this->workers[array_rand($this->workers)])) {
			if ($select->stack($stackable)) {
				$stackable->setWorker($select->getThreadId());
					if ($stackable->notify())
						return $stackable;
					trigger_error(sprintf("failed to notify stackable in %s", $worker->getName()), E_USER_WARNING);
			} else trigger_error(sprintf("failed to stack onto selected worker %s", $worker->getName()), E_USER_WARNING);
		} else trigger_error(sprintf("failled to select a worker for Stackable"), E_USER_WARNING);
		
		return false;
	}
	
	/*
	* Shutdown the pool of threads cleanly, retaining exit status locally
	*/
	public function shutdown() {
		foreach($this->workers as $worker) {
			$this->status[$worker->getThreadId()]=$worker->join();
		}
	}
}

/*
* Create a pool of ten threads
*/
$pool = new Pool(10);

/*
* Create and submit an array of Stackables
*/
$work = array();
/*
* Submit 100 tasks to the pool
*/
while(++$target<100) {
	$work[]=$pool->submit(new ExampleWork(array_rand($_SERVER)));
}

/*
* Shutdown the pool
*/
$pool->shutdown();

/*
* Look inside ...
*/
foreach($pool->workers as $worker) {
	printf("Thread %lu made %d attempts ...\n", $worker->getThreadId(), $worker->getAttempts());
}
?>