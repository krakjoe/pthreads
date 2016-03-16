--TEST--
Test pthreads constant array copy
--FILE--
<?php
const CONSTANT_ARRAY = ['ok'];

class Test extends Thread {
        public function run() {
                echo CONSTANT_ARRAY[0];
        }
}

$test = new Test();
$test->start();
$test->join();
--EXPECT--
ok
