--TEST--
Test the guard usage in unbound anonymous classes
--DESCRIPTION--
This test verifies that class entry flags are properly copied when completing a
class copy (it is only relevant to unbound anonymous classes).
--FILE--
<?php

class T2 extends Threaded
{
    public function __get($p){}

	public function run()
	{
        var_dump($this->prop);
	}
}

$w = new Worker();
$w->start();
$w->stack(new class extends T2 {});
$w->shutdown();
--EXPECT--
NULL
