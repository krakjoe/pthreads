--TEST--
Test anonymous classes (unbound inherited class)
--DESCRIPTION--mae
This test verifies that anonymous Threaded objects work as expected
--FILE--
<?php
$worker = new Worker();

$worker->start();

$collectable = new class extends Threaded {
	public function run() {
		var_dump($this instanceof Collectable);	
	}
};

$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
