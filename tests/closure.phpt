--TEST--
Test closures
--DESCRIPTION--
This test verifies that closures can be used
--FILE--
<?php
class TestThread extends Thread {
  public function __construct(Closure $closure) {
    $this->closure = $closure;
  }
  public function run() { 
    $f = $this->closure;
    printf("%s\n", $f()); 
  }
}

$thread = new TestThread(function() { return "Closure"; });
$thread->start();
?>
--EXPECT--
Closure
