--TEST--
Test pthreads Worker::unstack
--DESCRIPTION--
This test verifies that unstack functions as intended
--FILE--
<?php

$worker = new Worker();

$worker->stack(new class extends Threaded {
	public function run() {
		var_dump($this);
	}
});

var_dump($worker->unstack());

$worker->shutdown();
?>
--EXPECTF--
object(class@anonymous)#%d (%d) {
}
