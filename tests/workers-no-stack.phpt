--TEST--
Test pthreads workers rules (stack)
--DESCRIPTION--
This test verifies that workers cannot be misused (stack)
--FILE--
<?php
class Work extends Threaded implements Collectable {
	public function isGarbage() : bool { return true; }
}

class Test extends Thread {
	public function __construct(Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() {
		$c = new Work();
		$this->worker
			->stack($c);
	}
}

$worker = new Worker();
$worker->start();
$test = new Test($worker);
$test->start();
$test->join();
$worker->shutdown();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this Worker may call stack in %s:%d
Stack trace:
#0 %s(%d): Worker->stack(Object(Work))
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line %d

