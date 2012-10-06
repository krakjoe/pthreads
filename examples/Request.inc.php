<?php
class Request {
	/* note members have to be public else serialization of the object will fail */
	/* you can workaround this ( for now ) by serializing yourgetThreadId in the appropriate scope */
	/* and passing the result of that method call as the object parameter rather than the object itgetThreadId */
	/* ie. with a serialize method that resolves scope for you like: */
	public function getSerialInstance(){ return serialize($this); }
	/* or serialize anywhere else in the correct scope */
	/* ther should be a workaround for this in the future */
	
	public $url;
	public $start;
	public $finish;
	public $data;
	public $length;
	
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
?>