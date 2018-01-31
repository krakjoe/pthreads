--TEST--
Test the copying of child to parent thread
--DESCRIPTION--
When copying a class from a child thread to a parent thread, there are problems
with interned strings (since they were not being copied).
--FILE--
<?php

class Foo extends Thread
{
	public $running = true;
	private $shared;

	public function run() {
		require __DIR__ . '/child-to-parent-class-copying-helper.php';

		$this->shared['baseClass'] = new ExternalBaseClass();

		while($this->running);
	}
}

$foo = new Foo();
$foo->shared = new Threaded();
$foo->start();

while(!isset($foo->shared['baseClass']));

$baseClass = $foo->shared['baseClass']; // copy zend_class_entry
$foo->running = false;

$foo->join();
--EXPECT--