--TEST--
Test objects that have gone away
--DESCRIPTION--
This test verifies that objects that have gone away do not cause segfaults
--FILE--
<?php
class O extends Stackable { 
	public function run() {

	}
}

class T extends Thread {
	public $o;

	public function run() {
		/* this is created without intent of sharing */
		/* with any context created before this one */
		$this->o = new O();	
	}
}

$t = new T();
$t->start();
$t->join();
/* we expect nothing */
var_dump($t->o);
?>
--EXPECT--
NULL

