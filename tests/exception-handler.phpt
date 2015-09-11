--TEST--
Test exception handler inheritance (ok)
--DESCRIPTION--
This test verifies that user exception handler is invoked by pthreads if set
--FILE--
<?php
class ExceptionHandler
{
    static public function handle(Exception $e)
    {
        var_dump($e);
    }
}

class ExceptionThread extends Thread
{
	public function traceable() {
		throw new Exception();
	}
    public function run()
    {
        $this->traceable();
    }
}

/* this is now copied from parent, as you would expect it to be */
set_exception_handler(array("ExceptionHandler", "handle"));

$t = new ExceptionThread();
$t->start();
$t->join();
?>
--EXPECTF--
object(Exception)#2 (%d) {
  ["message":protected]=>
  string(0) ""
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(%d) "%s"
  ["line":protected]=>
  int(13)
  ["trace":"Exception":private]=>
  array(2) {
    [0]=>
    array(6) {
      ["file"]=>
      string(%d) "%s"
      ["line"]=>
      int(17)
      ["function"]=>
      string(9) "traceable"
      ["class"]=>
      string(15) "ExceptionThread"
      ["type"]=>
      string(2) "->"
      ["args"]=>
      array(0) {
      }
    }
    [1]=>
    array(4) {
      ["function"]=>
      string(3) "run"
      ["class"]=>
      string(15) "ExceptionThread"
      ["type"]=>
      string(2) "->"
      ["args"]=>
      array(0) {
      }
    }
  }
  ["previous":"Exception":private]=>
  NULL
}

