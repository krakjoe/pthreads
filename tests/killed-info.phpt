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
    public $started = false;

    public function run()
    {
        $this->synchronized(function($that) {
            $that->started = true;
            $that->notify();
        }, $this);

        sleep(5);
    }
}

$t = new Test();
$t->start();

$t->synchronized(function($that) {
    if (!$that->started)
        $that->wait();
}, $t);

$t->kill();
$t->join();

var_dump($t->isTerminated());
?>
--EXPECT--
bool(true)

