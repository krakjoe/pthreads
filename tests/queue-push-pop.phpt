--TEST--
Queue test push and pop
--DESCRIPTION--
This test verifies that queue push and pop functionality are working
--FILE--
<?php

$q = new Queue();
$q->push('1');
$q->push('2');
$q->push('3');
var_dump($q->size());
var_dump($q->pop());
var_dump($q->pop());
var_dump($q->pop());
var_dump($q->pop());

?>
--EXPECT--
int(3)
string(1) "3"
string(1) "2"
string(1) "1"
NULL
