--TEST--
Testing merging members
--DESCRIPTION--
This tests that merging members works as expected
--FILE--
<?php
class Store extends Stackable {
        public function run(){}
}

$stores = array(new Store(), new Store());

$stores[0]->test = "one";
$stores[1]->other = "when";
$stores[1]->test = "two";
$stores[1]->next = 0.01;

$stores[0]->merge($stores[1]);

var_dump($stores[0]);

$stores[0]->test = "one";
$stores[1]->other = "when";
$stores[1]->test = "two";
$stores[1]->next = 0.01;

$stores[0]->merge($stores[1], false);

var_dump($stores[0]);
?>
--EXPECT--
object(Store)#1 (3) {
  ["test"]=>
  string(3) "two"
  ["other"]=>
  string(4) "when"
  ["next"]=>
  float(0.01)
}
object(Store)#1 (3) {
  ["test"]=>
  string(3) "one"
  ["other"]=>
  string(4) "when"
  ["next"]=>
  float(0.01)
}

