--TEST--
Test extending abstract
--DESCRIPTION--
This test is regression test for #409
--FILE--
<?php
var_dump(Threaded::extend(ReflectionFunctionAbstract::class));
?>
--EXPECT--
bool(true)
