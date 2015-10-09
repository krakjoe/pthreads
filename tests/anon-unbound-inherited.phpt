--TEST--
Test anonymous classes (unbound inherited class)
--DESCRIPTION--mae
This test verifies that anonymous Threaded objects work as expected
--FILE--
<?php
$worker = new Worker();

$worker->start();

$collectable = new class extends Threaded implements Collectable {
	public function run() {
		var_dump($this instanceof Collectable);	
	}

	public function isGarbage() : bool { return true; }
};

$worker->stack($collectable);

$worker->shutdown();
--EXPECT--
bool(true)
