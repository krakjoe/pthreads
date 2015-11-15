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

$pool->collect(function(Collectable $work) {
	return $work->isGarbage();
});

var_dump($pool);
?>
--EXPECTF--
object(Work)#%d (%d) {
  ["worker"]=>
  object(Worker)#%d (0) {
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
  ["ctor":protected]=>
  NULL
  ["last":protected]=>
  int(1)
}



