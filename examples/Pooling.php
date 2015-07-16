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
		try {
			$this->worker
			->logger
			->log("%s executing in Thread #%lu",
				  __CLASS__, $this->worker->getThreadId());
		} catch(Throwable $thrown) { var_dump($thrown); }
		$this->setGarbage();
	}
}

class SafeLog extends Threaded {
	protected function log($message, ... $args) {
		echo vsprintf("{$message}\n", $args);
	}
}

$pool = new Pool(8, 'WebWorker', [new SafeLog()]);
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
$pool->submit(new WebWork());
$pool->shutdown();

$pool->collect(function($work){
	return $work->isGarbage();
});

var_dump($pool);
?>
