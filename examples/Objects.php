<?php
/*
	You can pass this around ... watch ...
	
	A Stackable can be thought of ( and should be used as ) as threaded stdClass
	PHP's stdClass was never meant to be used in this kind of environment:
		User declared classes where possible should extend Stackable, the object is MUCH better suited for threading, as you shall see ...
	
	When you pass around a reference to a Thread/Worker/Stackable to any context ( that's Thread/Stackable/Worker ) ...
	
	I don't have words to explain it, read on ...
*/
class GlobalStorage extends Stackable {
	public $id;
	
	public function __construct(){
		$this->id = 1;
	}
	
	public function addToObject($key, $data) {
		$this["{$key}"]=$data;
	}
	
	public function fetch($key) {
		return $this["{$key}"];
	}
	
	/* note that members must be strings */
	protected function getUniqueId(){
		$id = $this->id++;
		if (!$id)
			return "unknown";
		printf("Returning %d\n", $id);
		return "{$id}";
	}
	
	public function run(){
		/* we won't do anything to keep the example simple */
		/* 	
			 in the real world this kind of object would be executed:
				for example, at the beginning of the request
				to retrieve
				and at the end
				to save
		*/
	}
}
class MyThread extends Thread {
	public $stored;
	public $storage;
	public $worker;
	
	public function __construct($storage, $worker = null) {
		$this->storage = $storage;
		$this->worker = $worker;
		$this->stored = 0;
	}
	
	public function run(){
		/* it's possible to reference the stackable directly from the object scope for every call */
		/* but this will incur unecessary locking and allocation */
		/* if logic requires reuse of a reference then pull it into the method scope */
		/* a stackable does NOT need to be written back to the method scope for changes to data to be visible in other contexts */
		/* ANY other type of object DOES */
		if ($this->storage) {
			printf("%s (%lu) STORAGE AVAILABLE\n", __METHOD__, $this->getThreadId());
			$this->stored = $this->storage->getUniqueId();
			$this->storage->addToObject(
				$this->stored,
				array(rand()*120)
			);
			if ($this->worker) {
				printf("%s (%lu) WORKER AVAILABLE\n", __METHOD__, $this->getThreadId());
				if (!$this->worker->isShutdown()){
					$work = new MyWork($this->storage);
					$this->worker->stack($work);
					print_r($work);
				} else printf("%s (%lu) WORKER SHUTDOWN\n", __METHOD__, $this->getThreadId());
			} else printf("NO WORKER !!\n");
			printf("%s (%lu) STORED %s@%s\n", __METHOD__, $this->getThreadId(), $this->storage->fetch($this->stored), $this->getStorageId());
		} else printf("%s (%lu) NO STORAGE\n", __METHOD__, $this->getThreadId());
	}
	
	public function getStorageId() {
		return $this->stored;
	}
}


class GlobalWorker extends Worker {
	public function __construct($storage){
		$this->storage = $storage;
	}
	
	public function run() {
		/* setup some connections and whatever */
		while(++$o<20)
			$a[$o]=rand()*1009;
		/* this works, incase anyone cares */
		$this["storage"][__CLASS__]=$a;
	}
	
	/* some useful methods for stackables to fetch connections and whatever here */
}

class MyWork extends Stackable {
	public $stored;
	public $thread;
	public $storage;
	public $tstored;
	
	public function __construct($storage) {
		$this->storage = $storage;
		$this->tstored = null;
	}
	
	public function run(){
		/* read once, see above ... */
		if (($storage = $this->storage)) {
			$this->stored = $storage->getUniqueId();
			$storage->addToObject(
				$this->stored,
				array(rand()*120)
			);
			/*
			* This stackable, being executed by a worker to manipulate a global object, created in another thread ...
			* Is now going to create a thread and pass it a reference to the global object
			* The thread 
				created by the stackable 
				executing in the worker 
				can reference the same objects 
			* that all the things I just said can ...
			*/
			$thread = new MyThread($storage, $this->worker);
			if ($thread->start()) {
				$thread->join();
				$this->tstored = $thread->getStorageId();
				/* @NOTE doesnt matter what reference to the stackable is passed to the thread */
				
			}
		}
	}
	
	public function getStorageId() { return $this->stored; }
	public function getThreadStorageId() { return $this->tstored; }
}
/* this object we will pass everywhere we can */
$storage = new GlobalStorage();
/* this might be an SQLWorker, or just general purpose executor */
$worker = new GlobalWorker($storage);
$worker->start();

/* in the real world where you're executing the storage: */
/* $worker->stack($storage); */
/* while(!$storage->isReady()) */
/*	$storage->wait(); */

/* random array of work to populate some storage from the worker */
$work = array();
while(++$o<10) {
	/* items stacked could be using resources available in worker */
	$work[]=new MyWork($storage);
}
foreach($work as $w)
	$worker->stack($w);
/*
	In this example your application would now be loaded, with access to a runnng background worker
	and all the other goodies we just created/initiated/whatever
	It's worth noting that now any part of your normal application code can stack to the worker, and reference the global storage
	or whatever else you created...
	It does not matter what reference you pass around.
*/
/* done with worker, execute everything */
$worker->shutdown();

printf("set by stackables...:\n");
foreach($work as $w)
	var_dump($storage->fetch($w->getStorageId()));
printf("set by threads executed by stackables executing in a worker: ...\n");
foreach($work as $w) {
	if ($w->getThreadStorageId())
		var_dump($storage->fetch($w->getThreadStorageId()));
}
printf("set by the worker (alt syntax):\n");
var_dump($storage->fetch("GlobalWorker"));

/* 
	@NOTE pretty cool, right ?
*/
?>
