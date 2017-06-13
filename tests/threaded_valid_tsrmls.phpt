--TEST--
Test correct TSRMLS per threaded object
--DESCRIPTION--
This test verifies that TSRMLS is set properly to threaded objects
--FILE--
<?php
class Test extends Threaded
{
	private $prop;
}

$thread = new class extends Thread {
	public function __construct()
	{
		$this->test = new Test();
		$this->alive = true;
	}

	public function run()
	{
		$this[] = $this->test;
		while ($this->alive);
	}
};
$thread->start();

while (true) {
	if (isset($thread[0])) {
		$thread[0][] = new Threaded();

		if(count($thread[0]) > 32) break;
	}
}
$thread->alive = false;
$thread->join();
--EXPECTF--