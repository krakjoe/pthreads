<?php
/*
* pthreads overrides the functionality of the protected and private access modifiers ( for methods )
*/
class ExampleThread extends Thread {
	/*
	* This private method can only be called within the threading context
	* @NOTE there is a leak when a private method is called from outside the threading context
	*/
	private function noaccess(){
		printf("%lu: ran %s\n", $this->getThreadId(), __METHOD__);
	}
	
	/*
	* This protected method can only be called by one context at a time
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
		$this->noaccess();
		printf("%s: %s\n", __METHOD__, $this->data);
		return $this->data;
	}
}

/*
* This comment is not in use.
*/
$thread = new ExampleThread(rand()*10);
$thread->start();
if ($argv[1])
	$thread->noaccess();
?>