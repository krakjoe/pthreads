--TEST--
Test pthreads workers rules (stack)
--DESCRIPTION--
This test verifies that workers cannot be misused (stack)
--FILE--
<?php
class Test extends Thread {
	public function __construct(Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() {
		$c = new Collectable();
		$this->worker
			->stack($c);
	}
}

$worker = new Worker();
$worker->start();
$test = new Test($worker);
$test->start();
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this Worker may call stack in %s:10
Stack trace:
#0 %s(10): Worker->stack(Object(Collectable))
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 10

