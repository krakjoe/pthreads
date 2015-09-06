--TEST--
Test members (typeof object) with no other references
--DESCRIPTION--
In pthreads v2, you were charged with responsability of the objects you create, however in pthreads v3, a Threaded object
is able to retain a reference to another Threaded object, this makes programming Thread objects a lot simpler, 
the code in this test would fail with pthreads v2 and will work as expected with pthreads v3, which is a win for everyone ...
--FILE--
<?php
class O extends Threaded {
	public function run(){}
}

class T extends Thread {

	public function __construct() {
		$this->t = new O();
		$this->t->set = true;
		$this->s = new stdClass();
	}
	
    public function run(){
		var_dump($this->t, $this->s, $this);
	}
}

$t = new T();
var_dump($t);
$t->start();
$t->join();
?>
--EXPECTF--
object(T)#%d (%d) {
  ["t"]=>
  object(O)#%d (%d) {
    ["set"]=>
    bool(true)
  }
  ["s"]=>
  object(stdClass)#%d (%d) {
  }
}
object(O)#2 (%d) {
  ["set"]=>
  bool(true)
}
object(stdClass)#%d (%d) {
}
object(T)#%d (%d) {
  ["t"]=>
  object(O)#%d (%d) {
    ["set"]=>
    bool(true)
  }
  ["s"]=>
  object(stdClass)#%d (%d) {
  }
}
