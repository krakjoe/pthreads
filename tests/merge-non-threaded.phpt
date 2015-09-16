--TEST--
Test merge non-threaded
--DESCRIPTION--
This test verifies that merging with non-threaded classes works as expected
--FILE--
<?php
$array = [];

while (count($array) < 10)
    $array[] = count($array);

$stdClass = new stdClass();
$stdClass->foo = "foo";
$stdClass->bar = "bar";
$stdClass->baz = "baz";

$safe = new Threaded();
$safe->merge($array);

$safe->foo = "bar";
$safe->merge($stdClass, false);

var_dump($safe);
?>
--EXPECTF--
object(Threaded)#%d (%d) {
  [0]=>
  int(0)
  [1]=>
  int(1)
  [2]=>
  int(2)
  [3]=>
  int(3)
  [4]=>
  int(4)
  [5]=>
  int(5)
  [6]=>
  int(6)
  [7]=>
  int(7)
  [8]=>
  int(8)
  [9]=>
  int(9)
  ["foo"]=>
  string(3) "bar"
  ["bar"]=>
  string(3) "bar"
  ["baz"]=>
  string(3) "baz"
}
