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
--EXPECTF--
Deprecated: Methods with the same name as their class will not be constructors in a future version of PHP; test2 has a deprecated constructor in %s on line 4
ctor test2
