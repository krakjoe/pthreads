--TEST--
Test stacking by reference ONLY
--DESCRIPTION--
Stacking without a reference is not allowed, so the API should enforce the requirement
--FILE--
<?php
class T extends Stackable { 
	public function run() {

	}
}

class W extends Worker {
	public function run() {}
}

$t = new W();
$t->start();
$t->stack(new T());
$t->shutdown();
?>
--EXPECTF--
Fatal error: Uncaught exception 'InvalidArgumentException' with message 'Worker::stack expects $work to be a reference' in %s:%d
Stack trace:
#0 %s(%d): Worker->stack(Object(T))
#1 {main}
  thrown in %s on line %d

