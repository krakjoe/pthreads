--TEST--
Test pooling
--DESCRIPTION--
This test verifies the functionality of selective inheritance
--FILE--
<?php

class WebWorker extends Worker {
	public function __construct(SafeLog $logger) {
		$this->logger = $logger;
	}
	
	protected $loger;	
}

class WebWork extends Collectable {
	
	public function run() {
		$this->worker
			->logger
			->log("%s executing in Thread #%lu",
				  __CLASS__, $this->worker->getThreadId());
		$this->setGarbage();
	}
}

class SafeLog extends Threaded {
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args) {
			echo vsprintf("{$message}\n", ...$args);
		}, $message, $args);
	}
}

$pool = new Pool(8, 'WebWorker', array(new SafeLog()));

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
--EXPECTF--
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
WebWork executing in Thread #%d
object(Pool)#%d (%d) {
  ["size":protected]=>
  int(8)
  ["class":protected]=>
  string(9) "WebWorker"
  ["workers":protected]=>
  array(0) {
  }
  ["ctor":protected]=>
  array(1) {
    [0]=>
    object(SafeLog)#%d (0) {
    }
  }
  ["last":protected]=>
  int(6)
}

