--TEST--
Test anonymous classes (bound inherited class)
--DESCRIPTION--
This test verifies that anonymous Threaded objects work as expected
--FILE--
<?php
$worker = new Worker();

$collectable = new class extends Threaded implements Collectable {
	public function run() {
		var_dump($this instanceof Threaded);	
	}

	public function isGarbage() : bool { return true; }
	public function setGarbage() {}
};

$worker->start();

$worker->stack($collectable);

$worker->shutdown();
--EXPECT--
bool(true)
