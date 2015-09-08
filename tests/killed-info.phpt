--TEST--
Test kill
--DESCRIPTION--
This test verifies that ::kill sets state
--SKIPIF--
<?php if (defined('PHP_WINDOWS_VERSION_MAJOR')) die("skip: no support for this on windows"); ?>
--FILE--
<?php
class Test extends Thread
{
    public function run()
    {
        while(1);
    }
}

$t = new Test();
$t->start();
$t->kill();
$t->join();
var_dump($t->isTerminated());
?>
--EXPECT--
bool(true)

