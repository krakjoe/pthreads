--TEST--
Test pthreads workers rules (collect)
--DESCRIPTION--
This test verifies that workers cannot be misused (collect)
--FILE--
<?php
class Test extends Thread {
	public function __construct(Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() {
		$this->worker->collect(function($t){
			return $t->isGarbage();
		});
	}
}

$worker = new Worker();
$worker->start();
$test = new Test($worker);
$test->start();
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this Worker may collect from the stack in %s:10
Stack trace:
#0 %s(10): Worker->collect(Object(Closure))
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 10
