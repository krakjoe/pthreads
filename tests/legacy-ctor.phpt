--TEST--
Test legacy constructor (issue #336)
--DESCRIPTION--
This test verifies that legacy ctors do not induce failure
--FILE--
<?php
class test2 {
        function test2() {
            echo "ctor test2\n";
        }
}

class test1 extends Thread {
        function run() {
                $x = new test2();
        }
}

$t = new test1();
$t->start();
$t->join();
--EXPECT--
ctor test2
