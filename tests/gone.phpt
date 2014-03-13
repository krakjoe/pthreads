--TEST--
Test objects that have gone away
--DESCRIPTION--
This test verifies that objects that have gone away do not cause segfaults
--FILE--
<?php
class O extends Threaded { 
	public function run() {

	}
}

class T extends Thread {
	public $o;

	public function run() {
		$this->o = new O();
		/* this will disappear */
		$this->o["data"] = true;
	}
}

$t = new T();
$t->start();
$t->join();

var_dump($t->o);
?>
--EXPECTF--
Fatal error: Uncaught exception 'RuntimeException' with message 'pthreads detected an attempt to connect to a O which has already been destroyed' in %s:%d
Stack trace:
#0 %s(%d): unknown()
#1 {main}
  thrown in %s on line %d

