--TEST--
Test exception handler (nested caught) bug #498
--DESCRIPTION--
More mishandling of exceptions, regression test
--FILE--
<?php
class Test extends Thread {
    public function run() {
        set_exception_handler(function($message) {
            var_dump('Uncaught', $message);
        });

        try {
            self::processClassMethods();
        } catch(\Throwable $e) {
            var_dump('Caught', $e);
        }
    }

    public static function processClassMethods() {
        throw new \Exception();
    }
}
$test = new Test();
$test->start();
$test->join();
--EXPECTF--
string(6) "Caught"
object(Exception)#3 (7) {
  ["message":protected]=>
  string(0) ""
  ["string":"Exception":private]=>
  string(0) ""
  ["code":protected]=>
  int(0)
  ["file":protected]=>
  string(%d) "%s"
  ["line":protected]=>
  int(16)
  ["trace":"Exception":private]=>
  array(2) {
    [0]=>
    array(6) {
      ["file"]=>
      string(%d) "%s"
      ["line"]=>
      int(9)
      ["function"]=>
      string(19) "processClassMethods"
      ["class"]=>
      string(4) "Test"
      ["type"]=>
      string(2) "::"
      ["args"]=>
      array(0) {
      }
    }
    [1]=>
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

