--TEST--
Testing isset unset
--DESCRIPTION--
This tests that isset and unset are working properly
--FILE--
<?php
class Storage extends Stackable {
    public function run(){}
}

$storage = new Storage();
$storage->merge(range(1, 10));
var_dump($storage);

for($i = 0; $i < 10; $i ++) {
    var_dump(isset($storage[$i]));
    var_dump($storage[$i]);
    unset($storage[$i]);
    var_dump(isset($storage[$i]));
    printf("\n");
}
?>
--EXPECT--
object(Storage)#1 (10) {
  ["0"]=>
  int(1)
  ["1"]=>
  int(2)
  ["2"]=>
  int(3)
  ["3"]=>
  int(4)
  ["4"]=>
  int(5)
  ["5"]=>
  int(6)
  ["6"]=>
  int(7)
  ["7"]=>
  int(8)
  ["8"]=>
  int(9)
  ["9"]=>
  int(10)
}
bool(true)
int(1)
bool(false)

bool(true)
int(2)
bool(false)

bool(true)
int(3)
bool(false)

bool(true)
int(4)
bool(false)

bool(true)
int(5)
bool(false)

bool(true)
int(6)
bool(false)

bool(true)
int(7)
bool(false)

bool(true)
int(8)
bool(false)

bool(true)
int(9)
bool(false)

bool(true)
int(10)
bool(false)


