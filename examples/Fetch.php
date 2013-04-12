<?php
/*
* Sharing symbols 101
* @NOTE Thread::fetch was never included in a release and was superceeded by object handlers
* 	pthreads allows read access to thread data from any context
	pthreads allows write access to thread data from any context
	carry on reading ...
	work in progress ...
*/
class TestObject {
	public $val;
}

class Fetching extends Thread {
	public function run(){
		/*
		* of course ...
		*/
		$this->sym = 10245;
		$this->arr = array(
			"1", "2", "3"
		);
		
		/*
		* objects do work, no preparation needed ...
		* read/write objects isn't finalized ..
		* so do the dance to make it work ...
		*/
		$obj = new TestObject();
		$obj->val = "testval";
		$this->obj = $obj;
		
		/*
		* will always work
		*/
		$this->objs = serialize($this->obj);
		
		/*
		* nooooooo
		*/
		$this->res = fopen("php://stdout", "w");
		
		/*
		* tell the waiting process we have created symbols and fetch will succeed
		*/
		$this->synchronized(function(){
		    $this->notify();
		});
		
		/* wait for the process to be finished with the stream */
		$this->synchronized(function(){
		    $this->wait();
		});
	}
}

$thread = new Fetching();

$thread->start();

$thread->synchronized(function($me){
    $me->wait();
}, $thread);

/*
* we just got notified that there are symbols waiting
*/
foreach(array("sym", "arr", "obj", "objs", "res") as $symbol){
	printf("\$thread->%s: ", $symbol);	
	$fetched = $thread->$symbol;
	if ($fetched) {
		switch($symbol){
			/*
			* manual unserialize
			*/
			case "objs":
				var_dump(unserialize($fetched));
			break;
			
			default: var_dump($fetched);
		}
	}
	printf("\n");
}

/* notify the thread so it can destroy resource */
$thread->synchronized(function($me){
    $me->notify();
}, $thread);
?>
