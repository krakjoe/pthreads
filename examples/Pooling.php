<?php

class WebWorker extends Worker {

	public function __construct(SafeLog $logger) {
		$this->logger = $logger;
	}
	
	protected $logger;	
}

class WebWork extends Stackable {
	
	public function isComplete() {
		return $this->complete;
	}
	
	public function run() {
		$this->worker
			->logger
			->log("%s executing in Thread #%lu",
				  __CLASS__, $this->worker->getThreadId());
		$this->complete = true;
	}
	
	protected $complete;
}

class SafeLog extends Stackable {
	
	protected function log($message, $args = []) {
		$args = func_get_args();	
		
		if (($message = array_shift($args))) {
			echo vsprintf(
				"{$message}\n", $args);
		}
	}
}


$pool = new Pool(8, \WebWorker::class, [new SafeLog()]);

$pool->submit($w=new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->shutdown();

$pool->collect(function($work){
	return $work->isComplete();
});

var_dump($pool);
?>
