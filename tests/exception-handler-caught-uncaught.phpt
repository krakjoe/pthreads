--TEST--
Test exception handler (caught and uncaught) bug #493
--DESCRIPTION--
Mishandling of exceptions caused catch() blocks to be ignored in some cases
--FILE--
<?php
class Test extends Thread {

    public function run() {
		set_exception_handler(function($ex) {
			var_dump('Uncaught', $ex);
		});

		try {
			throw new \Exception();
		} catch(Throwable $ex) {
			var_dump('Caught', $ex);
		}

		throw new \Exception();
    }
}

$test = new Test();
$test->start();
$test->join();
--EXPECTF--
string(6) "Caught"
object(Exception)#%d (%d) {
  ["message":protected]=>
  string(0) ""
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(%d) "%s"
  ["line":protected]=>
  int(10)
  ["trace":"Exception":private]=>
  array(1) {
    [0]=>
    array(4) {
      ["function"]=>
      string(3) "run"
      ["class"]=>
      string(4) "Test"
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
string(8) "Uncaught"
object(Exception)#%d (%d) {
  ["message":protected]=>
  string(0) ""
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(%d) "%s"
  ["line":protected]=>
  int(15)
  ["trace":"Exception":private]=>
  array(1) {
    [0]=>
    array(4) {
      ["function"]=>
      string(3) "run"
      ["class"]=>
      string(4) "Test"
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


