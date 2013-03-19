<?php
/*
* Method Scope 101
*
* protected:
*	A protected method may only be called by one thread at a time
*	If Thread A is calling a protected method and Thread B attempts to call the same method Thread B will block until Thread A has left the method
*
* private:
*	private with respect to the scope of the Thread, ie: you can only call private methods from within the threading context
*/
class ExampleThread extends Thread {
	private $mine = "mine";
	
	/*
	* This private method can only be called within the threading context
	*/
	private function noaccess(){
		return sprintf("%lu: ran %s\n", $this->getThreadId(), __METHOD__);
	}
	
	/*
	* This protected method can only be called by one thread at a time
	*/
	protected function exclusive($arg = null){
		printf("IN->%s: %s\n", __METHOD__, microtime(true));
		if ($arg)
			$result = sprintf("%s: got \"%s\"", __METHOD__, $arg);
		else $result = sprintf("%s: got nothing", __METHOD__);
		printf("%s: %s\n", __METHOD__, microtime(true));
		usleep(1000000);
		printf("OUT->%s: %s\n", __METHOD__, microtime(true));
		return $result;
	}
	
	/*
	* Nothing special from here on in ...
	*/
	public function __construct($data){
		$this->data = $data;
		$this->mine = strrev($this->data);
	}
	
	public function run(){
		printf("IN->%s: %s\n", __METHOD__, microtime(true));
		printf("%s: %s\n", __METHOD__, $this->exclusive(strrev($this->data)));
		printf("%s: %s\n", __METHOD__, microtime(true));
		printf("%s: %s\n", __METHOD__, $this->noaccess());
		printf("OUT->%s: %s\n", __METHOD__, microtime(true));
	}
}

/*
* This comment is not in use.
*/
$thread = new ExampleThread(rand()*10);
$thread->start();

/*
* You can see that this call is blocked until the threading context returns from the method
*/
printf("Process: %s\n", $thread->exclusive());

/*
* Passing an argument on the command line will show you what happens when you call a private method from here
*/
if ($argv[1])
	printf("Process: %s\n", $thread->noaccess());
?>
