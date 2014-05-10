--TEST--
Test function table inheritance
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
        set_exception_handler(
        	array("ExceptionHandler", "handle"));

        $this->traceable();
    }
}

$t = new ExceptionThread();
$t->start();
$t->join();
?>
--EXPECTF--
object(Exception)#2 (7) {
  ["message":protected]=>
  string(0) ""
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(%d) "%s"
  ["line":protected]=>
  int(%d)
  ["trace":"Exception":private]=>
  array(1) {
    [0]=>
    array(6) {
      ["file"]=>
      string(%s) "%s"
      ["line"]=>
      int(%d)
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
  }
  ["previous":"Exception":private]=>
  NULL
}

