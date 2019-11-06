--TEST--
Test JIT superglobals
--DESCRIPTION--
This test verifies that superglobals which are JIT-inited are properly registered
--INI--
auto_globals_jit=1
variables_order=ES
--FILE--
<?php

class TestThread extends Thread {
	public function run() {
		var_dump(is_array($GLOBALS));
		var_dump(is_array($_SERVER));
		var_dump(is_array($_ENV));
	}
}

$thread = new TestThread();
$thread->start();
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
