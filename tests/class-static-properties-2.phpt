--TEST--
Testing class statics properties
--DESCRIPTION--
Ensure that the static properties of the current thread is copied and available.
--FILE--
<?php

class T extends Thread {
	private static $autoloadPath = 'def';

	public function __construct() {
		self::$autoloadPath = 'abc';
    }

	public function run() {
		var_dump(self::$autoloadPath);
	}
}

$thread = new T();
$thread->start(PTHREADS_INHERIT_NONE) && $thread->join();

--EXPECT--
string(3) "abc"
