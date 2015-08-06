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
	}
}

class SafeLog extends Threaded {
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args){
			echo vsprintf("{$message}\n", ... $args);
		}, $message, $args);
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
