--TEST--
Test pthreads Worker::unstack
--DESCRIPTION--
This test verifies that unstack functions as intended
--FILE--
<?php

$worker = new Worker();

$worker->stack(new class extends Collectable {
	public function run() {
		var_dump($this);
	}
});

var_dump($worker->unstack());
?>
--EXPECTF--
object(class@anonymous)#%d (1) {
  ["garbage"]=>
  bool(false)
}
