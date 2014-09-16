--TEST--
Testing closure to object conversion
--DESCRIPTION--
This test verifies Threaded objects can be created from Closures
--FILE--
<?php
$threaded = Thread::from(function(){
    $this->test = "hello";
    var_dump($this);
});

$threaded->start();
$threaded->join();

$pool = new Pool(1);
$pool->submit(Collectable::from(function(){
    var_dump($this);
}));
$pool->shutdown();

$test = new Threaded();

$threaded = Thread::from(function() {
    var_dump($this->test);
}, function($test) {
    $this->test = $test;
}, [$test]);

$threaded->start();
$threaded->join();

--EXPECTF--
object(ThreadClosure@%s)#%d (1) {
  ["test"]=>
  string(5) "hello"
}
object(CollectableClosure@%s)#%d (2) {
  ["garbage"]=>
  bool(false)
  ["worker"]=>
  object(Worker)#3 (0) {
  }
}
object(Threaded)#%d (0) {
}

