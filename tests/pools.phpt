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
	public function __construct(int $id) {
		$this->id = $id;
	}

	public function run() {
		$this->worker
			->logger
			->log("%s(%d) executing in Thread #%lu",
				  __CLASS__, $this->id, $this->worker->getThreadId());
	}

	protected $id;
}

class SafeLog extends Threaded {
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args) {
			echo vsprintf("{$message}\n", ...$args);
		}, $message, $args);
	}
}

$pool = new Pool(8, 'WebWorker', array(new SafeLog()));
while (@$i++<10)
	$pool->submit(new WebWork($i));

$pool->collect(function(WebWork $work){
	return $work->isGarbage();
});

$pool->shutdown();

var_dump($pool);
?>
--EXPECTF--
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
WebWork(%d) executing in Thread #%d
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
  int(%d)
}

