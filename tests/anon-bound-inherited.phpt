--TEST--
Test anonymous classes (bound inherited class)
--DESCRIPTION--
This test verifies that anonymous Threaded objects work as expected
--FILE--
<?php
$worker = new Worker();

$collectable = new class extends Threaded {
	public function run() {
		var_dump($this instanceof Threaded);	
	}
};

$worker->start();
$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
