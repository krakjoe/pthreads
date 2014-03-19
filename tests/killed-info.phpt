--TEST--
Test kill termination info
--DESCRIPTION--
This test verifies that ::kill sets state and error information
--FILE--
<?php
class TestThread extends Thread
{
    public function run()
    {
        sleep(5);
    }
}

$t = new TestThread();
$t->start();
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

