--TEST--
Test VERIFY_RETURN_TYPE overload - PHP 71 compliant
--DESCRIPTION--
When using the explicit cast of objects, return type hinting can be broken.

The class in the current context doesn't match the class of the object.

pthreads overrides ZEND_VERIFY_RETURN_TYPE to fix the problem, nothing else is affected.
--SKIPIF--
<?php
if(PHP_VERSION_ID < 70100) die('skip this test is for php 7.1+');
?>
--FILE--
<?php

class Test extends Thread {
        
        public function checkOptionalNull():?Threaded {
            return ($var = null);
        }

	public function run() {
                var_dump($this->checkOptionalNull());
	}
}

$test = new Test();
$test->start() && $test->join();
?>
--EXPECT--
NULL