--TEST--
Test kill termination info
--DESCRIPTION--
This test verifies that ::kill sets state and error information
--FILE--
<?php
class TestThread extends Thread
{
    public $started = false;

    public function run()
    {
        $this->synchronized(function($that) {
            $that->started = true;
            $that->notify();
        }, $this);

        sleep(5);
    }
}

$t = new TestThread();
$t->start();

$t->synchronized(function($that) {
    while (! $that->started)
        $that->wait();
}, $t);

$t->kill();
$t->join();

var_dump($t->isTerminated());
var_dump($t->getTerminationInfo());
?>
--EXPECTF--
bool(true)
array(4) {
  ["scope"]=>
  string(10) "TestThread"
  ["function"]=>
  string(3) "run"
  ["file"]=>
  string(%d) "%s"
  ["line"]=>
  int(%d)
}

