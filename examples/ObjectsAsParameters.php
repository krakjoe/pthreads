<?php
class OOWebRequest extends Thread {
	/*
	* You could have more parameters but an object can hold everything ...
	*/
	public function __construct($request){
		$this->response = $request;
	}
	/*
	* You must include the user declared class here 
	*/
	public function __prepare()	{ 
		include(sprintf(
			"%s/Request.inc.php", dirname(__FILE__)
		));
		if(class_exists("Request")){
			printf("Prepared Thread #%lu\n", $this->getThreadId());
		} else printf("Cannot Prepare Thread #%lu\n", $this->getThreadId());
		return true;
	}
	/*
	* Manipulate the object and return it as the result so the parent can have it back
	*/
	public function run(){
		printf("Running Thread #%lu\n", $this->getThreadId());
		if($this->response){
			if($this->response->url){
				$this->response->setStart(microtime(true));
				$this->response->setData(file_get_contents($this->response->url));
				$this->response->setFinish(microtime(true));
			}
		}
		return $this->response->getLength();
	}
}

/* include your declared class */
include(sprintf(
	"%s/Request.inc.php", dirname(__FILE__)
));

/* keeping a reference is a bit pointless ... though you could and replace it */
$request = new OOWebRequest(new Request("http://www.google.com"));
/* go */
if($request->start()){
	/* do some heavy lifting here perhaps in the current thread */
	while($request->isBusy()) {
		echo ".";
		usleep(100);
	}
	/* then join to get the result */
	if($request->join()){
		printf("\nGot %d bytes from %s in %f seconds\n", $request->response->getLength(), $request->response->getUrl(), $request->response->getDuration());
	}
}
?>
