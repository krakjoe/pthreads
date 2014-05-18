<?php
class WebWorker extends Worker {

	public function __construct(SafeLog $logger) {
		$this->logger = $logger;
	}
	
	protected $logger;	
}

/* the collectable class implements machinery for Pool::collect */
class WebWork extends Collectable {
	public function run() {
		$this->worker
			->logger
			->log("%s executing in Thread #%lu",
				  __CLASS__, $this->worker->getThreadId());
		$this->setGarbage();
	}
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

$pool = new Pool(8, 'WebWorker', [new SafeLog()]);

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
	return $work->isGarbage();
});

var_dump($pool);
?>
