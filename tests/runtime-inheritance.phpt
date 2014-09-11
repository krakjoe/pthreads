--TEST--
Test runtime extension
--DESCRIPTION--
This test verifies functionality of runtime extension
--FILE--
<?php
class Other {}

class Test extends Other {
    public function one() {}
}

/* force zend to declare Other extends Threaded */
Threaded::extend("Other");

$test = new Test();

var_dump($test instanceof Threaded,
         $test instanceof Other);
?>
--EXPECT--
bool(true)
bool(true)
