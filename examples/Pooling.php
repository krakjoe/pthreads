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
	public function __construct($data) {
		$this->local = $data;
	}	
	public function run() {
		$this->worker->addAttempt();
		$this->worker->addData(
			$this->local
		);
	} 
	public function getData() 				{ return $this->local; }
}
class ExampleWorker extends Worker {
	
	public function __construct($name) {
		$this->name = $name;
		$this->data = array();
		$this->setup = false;
		$this->attempts = 0;
	}
	public function run(){
		$this->setName(sprintf("%s (%lu)", $this->getName(), $this->getThreadId()));
	}
	public function setSetup($setup)	{ $this->setup = $setup; }
	public function getName() 			{ return $this->name; }
	public function setName($name)		{ $this->name = $name; }
	public function addAttempt() 		{ $this->attempts++; }
	public function getAttempts()		{ return $this->attempts; }
	public function setData($data)		{ $this->data = $data; }
	public function addData($data)		{ $this->data = array_merge($this->data, array($data)); }
	public function getData()			{ return $this->data; }
}
/* Dead simple pthreads pool */
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
			$id = count($this->workers);
			$this->workers[$id] = new ExampleWorker(sprintf("Worker [%d]", $id));
			$this->workers[$id]->start(PTHREADS_INHERIT_NONE);

			if ($this->workers[$id]->stack($stackable)) {
				return $stackable;
			} else trigger_error(sprintf("failed to push Stackable onto %s", $this->workers[$id]->getName()), E_USER_WARNING);
		}
		if (($select = $this->workers[array_rand($this->workers)])) {

			if ($select->stack($stackable)) {
				return $stackable;
			} else trigger_error(sprintf("failed to stack onto selected worker %s", $select->getName()), E_USER_WARNING);
		} else trigger_error(sprintf("failled to select a worker for Stackable"), E_USER_WARNING);
		
		return false;
	}
	/* Shutdown the pool of threads cleanly, retaining exit status locally */
	public function shutdown() {
		foreach($this->workers as $worker) {
			$this->status[$worker->getThreadId()]=$worker->shutdown();
		}
	}
}
$start = microtime(true);
/* Create a pool of ten threads */
$pool = new Pool(10);
/* Create and submit an array of Stackables */
$work = array();
while(++$target<100)
	$work[]=$pool->submit(new ExampleWork(array_rand($_SERVER)));
$pool->shutdown();
/*
* Look inside
*/
$runtime = (microtime(true)-$start);
if ($_SERVER["HTTP_HOST"]) echo "<pre>";
printf("---------------------------------------------------------\n");
printf("Executed %d tasks in %f seconds in %d threads\n", count($work), $runtime, 10);
printf("---------------------------------------------------------\n");
if ($_SERVER["HTTP_HOST"]) 
	printf("%s | %.3fMB RAM\n", $_SERVER["SERVER_SOFTWARE"], memory_get_peak_usage(true)/1048576);
else printf("%.3fMB RAM\n", memory_get_peak_usage(true)/1048576);
printf("---------------------------------------------------------\n");	
$attempts = 0;
foreach($pool->workers as $worker) {
	printf("%s made %d attempts ...\n", $worker->getName(), $worker->getAttempts());
	print_r($worker->getData());
	$attempts+=$worker->getAttempts();
}
printf("---------------------------------------------------------\n");
printf("Average processing time of %f seconds per task\n", $runtime/$attempts);
printf("---------------------------------------------------------\n");
if ($_SERVER["HTTP_HOST"]) echo "</pre>";
?>
