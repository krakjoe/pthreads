<?php
/* 
	This isn't built in as it's a pretty simple task to achieve for yourself
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
	}
	/**
	* The smallest thread in the world
	* @NOTE: the function you're calling had better had serializable results
	**/
	public function run(){ return call_user_func_array($this->method, $this->params); }
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
	* You could join the result, or if you know the response is a string then do the following:
	**/
	public function __toString(){ 
		if(!$this->result)
			$this->result = $this->join();
		return (string) $this->result;
	}
}

/* here's us calling file_get_contents in a thread of it's own */
$future = Async::call("file_get_contents", array("http://www.php.net"));
/* here's us counting the bytes out, note, __toString() magic joined so no need to join explicitly */
printf("Got %d bytes from php.net\n", strlen($future));
/* you can reference again as a string because you cached the result, YOU CANNOT JOIN TWICE */
printf("First 16 chars: %s\n", substr($future, 0, 16));
/* if you have no __toString(): */
/* $response = $future->join(); */
/* you could also not use a reference to the thread, 
	if the threads job was to say, log stats and you do not need to know when or how it returns, 
	then you could just Async::call at the beginning of the request and by the end it would be finished */
?>