--TEST--
New class after thread creation
--DESCRIPTION--
Fixes bug #711. This requires a class to be declared after a new thread has been
created, where the new class implements at least one interface
--FILE--
<?php

$worker = new \Worker();
$worker->start();

interface A {}
class task extends Threaded implements A {}

$worker->stack(new task());
while($worker->collect());
$worker->shutdown();
--EXPECT--