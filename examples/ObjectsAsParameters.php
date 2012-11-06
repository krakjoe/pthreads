<?php
/*
* Because we plan for two contexts to manipulate this object we extend the stackable declaration
* This connects automatically the reference in the creating context and the reference in the threading context
* Think of stackable as a threaded stdClass
*/
class Response extends Stackable {
	
	public function __construct($url){
		$this->url = $url;
		$this->start = 0;
		$this->finish = 0;
		$this->data = null;
		$this->length = 0;
	}
	
	public function setStart($start)	{ $this->start = $start; }
	public function setFinish($finish)	{ $this->finish = $finish; }
	public function setData($data)		{ 
		$this->data = $data; 
		$this->length = strlen($this->data);
	}
	
	public function getUrl()		{ return $this->url; }
	public function getDuration()	{ return $this->finish - $this->start; }
	public function getLength()		{ return $this->length; }
	public function getData()		{ return $this->data; }
	public function getStart()		{ return $this->start; }
	public function getFinish()		{ return $this->finish; }
	
	public function run(){
		/* this particular object won't run */
	}
	
}

class Request extends Thread {
	/*
	* You could have more parameters but an object can hold everything ...
	*/
	public function __construct($response){
		$this->response = $response;
	}
	
	/*
	* Populate the response object for creating context
	*/
	public function run(){
		
		/*
		* NOTE:
		* Referencing threaded objects inside a thread:
		*	If you plan to heavily manipulate any object then retain a reference in the method scope
		*	Anytime you read and write the object context of a threaded object locking occurs
		*	Retaining method scope references avoids some locking and allocation
		*	$response retains the connection to the reference in the thread that created the request
		* If you have a threaded member that you only plan to execute ONE time:
		*	(isWaiting/isRunning for example) then it is acceptable to reference the member via the object context
		* Referencing the object context on every call will have no adverse affects ( no leaks/errors )
		* The hints above are best practices but not required.
		*/
		if(($response = $this->response)){
			if($response->getUrl()){
				$response->setStart(microtime(true));
				$response->setData(file_get_contents($response->url));
				$response->setFinish(microtime(true));
			}
		}
	}
}

/*
* We can retain the reference to the response object !!
*/
$response = new Response(
	sprintf("http://www.google.com/?q=%s", md5(rand()*time()))
);

/*
* Initialize a new threaded request with a response object as the only parameter
*/
$request = new Request($response);

/*
* Tell you all about it ...
*/
printf("Fetching: %s ", $response->getUrl());
if($request->start()){
	/* do something during runtime */
	while($request->isRunning()) {
		echo ".";
		usleep(100);
	}
	/* 
		you do not need to join:
			when a thread returns false for isRunning then your code is no longer being executed
			pthreads will cleanup the variable when it goes out of scope like any other variable in php
	*/
	printf(" got %d bytes in %f seconds\n", $response->getLength(), $response->getDuration());
}
?>