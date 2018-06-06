--TEST--
Test runtime extension of Volatile
--DESCRIPTION--
This test verifies that extend() applies the scope it was called in, not executed in
--FILE--
<?php
class Other {}

class Test extends Other {
    public function one() {}
}

/* force zend to declare Other extends Volatile */
Volatile::extend("Other");

$test = new Test();

var_dump($test instanceof Volatile,
         $test instanceof Other);
?>
--EXPECT--
bool(true)
bool(true)
