--TEST--
Test members (typeof object) with no other references
--DESCRIPTION--
This test verifies that members of an object type that have no other references in the engine can be set as members of threaded objects
--FILE--
<?php
class O extends Threaded {
	public function run(){}
}

class T extends Thread {
	public function __construct() {
		$this->t = new O();
	}
	
    public function run(){}
}

var_dump(new T());
?>
--EXPECTF--
object(T)#1 (0) {
}

Fatal error: Uncaught %s: pthreads detected an attempt to connect to an object which has already been destroyed in %s:%d
Stack trace:
#0 %s(%d): var_dump(Object(T))
#1 {main}
  thrown in %s on line %d


