--TEST--
Check chunking
--DESCRIPTION--
This test verifies functionality of ::chunk
--FILE--
<?php
class S extends Stackable {
    public function run(){}
}

$s = new S();
$s->merge(array_fill(0, 10000, true));

var_dump($s->chunk(10));
?>
--EXPECT--
array(10) {
  [0]=>
  bool(true)
  [1]=>
  bool(true)
  [2]=>
  bool(true)
  [3]=>
  bool(true)
  [4]=>
  bool(true)
  [5]=>
  bool(true)
  [6]=>
  bool(true)
  [7]=>
  bool(true)
  [8]=>
  bool(true)
  [9]=>
  bool(true)
}

