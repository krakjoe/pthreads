<?php
/*
* pthreads overrides the functionality of the protected and private access modifiers ( for methods )
*/
class ExampleThread extends Thread {
	
	private function noaccess(){
		return __METHOD__;
	}
	
	/*
	* 
	*/
	protected function synchronized($arg = null){
		if ($arg)
			return sprintf("%s: got \"%s\"", __METHOD__, $arg);
		return sprintf("%s: got nothing", __METHOD__);
	}
	
	/*
	* Nothing special from here on in ...
	*/
	public function __construct($data){
		$this->data = $data;
	}
	
	public function run(){
		printf("%s: %s\n", __METHOD__, $this->data);
		printf("%s: %s\n", __METHOD__, $this->synchronized(strrev($this->data)));
		//printf("%s: %s\n", __METHOD__, $this->noaccess());
		printf("%s: %s\n", __METHOD__, $this->data);
	}
}

/*
* This comment is not in use.
*/
$thread = new ExampleThread(rand()*10);
$thread->start();

?>