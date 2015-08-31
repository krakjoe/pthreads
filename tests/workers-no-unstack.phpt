--TEST--
Test pthreads workers rules (unstack)
--DESCRIPTION--
This test verifies that workers cannot be misused (unstack)
--FILE--
<?php
class Test extends Thread {
	public function __construct(Worker $worker) {
		$this->worker = $worker;
	}
	
	public function run() {
		$this->worker->unstack();
	}
}

$worker = new Worker();
$worker->start();

$test = new Test($worker);
$test->start();
$test->join();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: only the creator of this Worker may call unstack in %s:8
Stack trace:
#0 %s(8): Worker->unstack()
#1 [internal function]: Test->run()
#2 {main}
  thrown in %s on line 8


