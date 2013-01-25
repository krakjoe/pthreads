--TEST--
Test members (typeof object) with no other references
--DESCRIPTION--
This test verifies that members of an object type that have no other references in the engine can be set as members of threaded objects
--FILE--
<?php
class O extends Stackable {
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
--EXPECT--
object(T)#1 (1) {
  ["t"]=>
  object(O)#3 (0) {
  }
}

