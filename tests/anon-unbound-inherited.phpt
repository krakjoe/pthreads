--TEST--
Test anonymous classes (unbound inherited class)
--DESCRIPTION--mae
This test verifies that anonymous Threaded objects work as expected
--FILE--
<?php
$worker = new Worker();

$worker->start();

$collectable = new class extends Threaded {
	/** z */
	const Z = 1;
	/** a */
	static $a = 2;
	/** c */
	public $c = false;

	public function run() {
		var_dump(
			$this instanceof Collectable,
			self::Z,
			self::$a,
			$this->c
		);
	}
};

$worker->stack($collectable);
$worker->shutdown();
--EXPECT--
bool(true)
int(1)
int(2)
bool(false)
