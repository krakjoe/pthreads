--TEST--
Test pool defaults
--DESCRIPTION--
This test verifies pool defaults
--FILE--
<?php
class Work extends Threaded {
	public function run() {
		var_dump($this);
	}
}

$pool = new Pool(1);
$pool->submit(new Work());
$pool->shutdown();

var_dump($pool);
?>
--EXPECTF--
object(Work)#%d (%d) {
  ["worker"]=>
  object(Worker)#%d (%d) {
  }
}
object(Pool)#%d (%d) {
  ["size":protected]=>
  int(1)
  ["class":protected]=>
  string(6) "Worker"
  ["workers":protected]=>
  array(0) {
  }
  ["work":protected]=>
  array(1) {
    [0]=>
    object(Work)#%d (%d) {
      ["worker"]=>
      object(Worker)#%d (%d) {
      }
    }
  }
  ["ctor":protected]=>
  NULL
  ["last":protected]=>
  int(1)
}

