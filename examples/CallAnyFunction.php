<?php
/* 
	This isn't built in as it's a pretty simple task to achieve for your self
	I can't really think of any functions that are labourious enough that you
	would want to execute them by themselves in a thread of their own. Maybe
	you have functions within your own code that could be called async without
	refactoring for multi-threading capabilities ...
	
	I personally think you shouldn't try to make chocolate from cheese and you
	would be better of refactoring your code to be multi-threaded ...
	
	But here's an example of how you would achieve such a task:
*/
class Async extends Thread {
	/**
	* Provide a passthrough to call_user_func_array
	**/
	public function __construct($method, $params){
		$this->method = $method;
		$this->params = $params;
		$this->result = null;
		$this->joined = false;
	}
	
	/**
	* The smallest thread in the world
	**/
	public function run(){
		if (($this->result=call_user_func_array($this->method, $this->params))) {
			return true;
		} else return false;
	}
	
	/**
	* Static method to create your threads from functions ...
	**/
	public static function call($method, $params){
		$thread = new Async($method, $params);
		if($thread->start()){
			return $thread;
		} /** else throw Nastyness **/
	}
	
	/**
	* Do whatever, result stored in $this->result, don't try to join twice
	**/
	public function __toString(){ 
		if(!$this->joined) {
			$this->joined = true;
			$this->join();
		}
		
		return $this->result;
	}
}

/* here's us calling file_get_contents in a thread of it's own */
$future = Async::call("file_get_contents", array("http://www.php.net"));
/* here's us counting the bytes out, note, __toString() magic joined so no need to join explicitly */
printf("Got %d bytes from php.net\n", strlen($future));
/* you can reference again as a string because you cached the result, YOU CANNOT JOIN TWICE */
printf("First 16 chars: %s\n", substr($future, 0, 16));
/* if you have no __toString(): */
/* $response = $future->result; */
/* you could also not use a reference to the thread, 
	if the threads job was to say, log stats and you do not need to know when or how it returns, 
	then you could just Async::call at the beginning of the request and by the end it would be finished */
?>