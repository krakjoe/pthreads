--TEST--
Test pthreads Worker::shutdown
--DESCRIPTION--
This test verifies shutdown of a Worker (or Thread) not yet started doesn't fault
--FILE--
<?php
$worker = new Worker();

$worker->stack(new class extends Threaded {
	public function run() {
		var_dump($this);
	}
});

$worker->shutdown();
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: Worker has not been started in %s:10
Stack trace:
#0 %s(10): Worker->shutdown()
#1 {main}
  thrown in %s on line 10
