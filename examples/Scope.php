<?php
/*
* pthreads overrides the functionality of the protected and private access modifiers ( for methods )
*/
class ExampleThread extends Thread {
	/*
	* A private method can only be accessed from within the thread
	* A call to a protected method from any other context will result in an E_WARNING being raised and NULL being returned
	* The reasons it does not result in an E_ERROR ( fatality ) is on account of synchronization ( wait/notify ), an E_ERROR will suffice
	* Code that tries to execute a private method from another context is WRONG and must be changed so you should never see the error in production
	* but it may help in the case of porting existing objects in a codebase to be multithreaded
	*/
	private function noaccess(){
		return __METHOD__;
	}
	
	/*
	* A protected method synchronizes the thread with the following effects:
	*	No other contexts can read members ( until the method has returned )
	*	No other contexts can execute protected methods ( all methods )
	*
	* Other contexts can still read the state of a thread ( isWaiting/isJoined )
	*
	* If context A is executing a protected method, and context B calls that method, context B will block until context A leaves the method
	* If context A is executing a protected method, and context B calls a public method the executing of the method will go ahead without blocking
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
	//	printf("%s: %s\n", __METHOD__, $this->noaccess());
		printf("%s: %s\n", __METHOD__, $this->data);
	}
}

/*
* This comment is not in use.
*/
$thread = new ExampleThread(rand()*10);
$thread->start();

?>