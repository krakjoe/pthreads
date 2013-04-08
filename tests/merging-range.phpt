--TEST--
Testing merging members (long keys)
--DESCRIPTION--
This tests that merging ranges works as expected (long keys)
--FILE--
<?php
class Storage extends Stackable {
    public function run() {}
}

$storage = new Storage();
$storage->merge(range("0", "3"));
var_dump($storage);
?>
--EXPECT--
object(Storage)#1 (4) {
  ["0"]=>
  int(0)
  ["1"]=>
  int(1)
  ["2"]=>
  int(2)
  ["3"]=>
  int(3)
}
