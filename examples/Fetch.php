<?php
/*
* Sharing symbols 101
*/
class Fetching extends Thread {
	public function run(){
		/*
		* all basic types supported all the time
		*/ 
		$this->sym = 10245;
		$this->arr = array(
			"1", "2", "3"
		);
		/*
		* objects supported at join time ( when it's finished with, and no concern for the thread safety of the underlying object is required )
		*/
		$this->obj = new stdClass();
		$this->obj->val = "testval";
		/*
		* serialized objects supported all the time
		*/
		$this->objs = serialize($this->obj);
		/*
		* resources never supported
		*/
		$this->res = fopen("php://stdout", "w");
		
		/*
		* tell the waiting process we have created symbols and fetch will succeed
		*/
		$this->notify();
		
		/*
		* wait for waiting process to fetch symbols
		*/
		$this->wait();
	}
}
$thread = new Fetching();
$thread->start(true);
/*
* we just got notified that there are symbols waiting
*/
foreach(array("sym", "arr", "obj", "objs", "res") as $symbol){
	printf("Thread::fetch(%s): ", $symbol);	
	$fetched = $thread->fetch($symbol);
	if ($fetched) {
		switch($symbol){
			/*
			* we need to manually serialize objects here
			*/
			case "objs":
				print_r(unserialize($fetched));
			break;
			
			default: var_dump($fetched);
		}
	}
}
/*
* Tell the thread it can continue
*/
$thread->notify();

/*
* Join the thread ( and it's symbols )
*/
$thread->join();

/*
* Show thread structure after join ( will contain all symbols created in the thread )
* @NOTE members of a user declared class should be available in both contexts, else joining symbols will fail ( gracefully )
*/
print_r($thread);
?>