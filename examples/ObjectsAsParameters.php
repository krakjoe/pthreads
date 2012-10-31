<?php
class Response {
	
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
		* Read/Write objects as little as possible
		*/
		$response = $this->response;
		
		if($response){
			if($response->url){
				$response->setStart(microtime(true));
				$response->setData(file_get_contents($response->url));
				$response->setFinish(microtime(true));
			}
		}
		
		/*
		* Write the object back to the thread now you're done manipulating it
		*/
		$this->response = $response;
		
		/*
		* Tell whoever is listening how much we got ...
		*/
		return $response->getLength();
	}
}

/*
* Initialize a new threaded request with a response object as the only parameter
*/
$request = new Request(
	new Response(
		sprintf("http://www.google.com/?q=%s", md5(rand()*time()))
	)
);

/*
* Tell you all about it ...
*/
printf("Fetching: %s ", $request->response->getUrl());
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
	printf(" got %d bytes in %f seconds\n", $request->response->getLength(), $request->response->getDuration());
}
?>