--TEST--
Test return types
--DESCRIPTION--
This test verifies that functions with a typed return are copied and execute properly
--FILE--
<?php
function some() : string {
	return __FUNCTION__;
}

$thread = new class extends Thread {
	public function run () {
		var_dump(some());
	}
};

$thread->start();
$thread->join();
--EXPECT--
string(4) "some"
