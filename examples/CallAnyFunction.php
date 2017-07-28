<?php
/*
	This isn't built in as it's a pretty simple task to achieve for your self
	I can't really think of any functions that are labourious enough that you
	would want to execute them by themselves in a thread of their own. Maybe
	you have functions within your own code that could be called in parallel without
	refactoring for multi-threading capabilities ...

	I personally think you shouldn't try to make chocolate from cheese and you
	would be better of refactoring your code to be multi-threaded ...

	But here's an example of how you would achieve such a task:
*/
class Caller extends Thread {
	/**
	* Provide a passthrough to call_user_func_array
	**/
	public function __construct(callable $method, ...$params){
		$this->method = $method;
		$this->params = $params;
		$this->result = null;
		$this->joined = false;
	}

	/**
	* The smallest thread in the world
	**/
	public function run() {
		$this->result = 
			($this->method)(...$this->params); /* gotta love php7 :) */
	}

	/**
	* Static method to create your threads from functions ...
	**/
	public static function call($method, ...$params){
		$thread = new Caller($method, ...$params);
		if($thread->start()){
			return $thread;
		}
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


	private $method;
	private $params;
	private $result;
	private $joined;
}

/* here's us calling file_get_contents in a thread of it's own */
$future = Caller::call("file_get_contents", "http://www.php.net");
/* here's us counting the bytes out, note, __toString() magic joined so no need to join explicitly */
printf("Got %d bytes from php.net\n", strlen((string)$future));
/* you can reference again as a string because you cached the result, YOU CANNOT JOIN TWICE */
printf("First 16 chars: %s\n", substr((string)$future, 0, 16));

/* here's us calling a closure in a thread of it's own */
$future = Caller::call(function(...$params) {
	printf("and how about this: %s, %s %s %s %s!\n", ...$params);
}, "also", "you", "can", "use", "closures");
?>
