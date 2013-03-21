--TEST--
Null member crash
--DESCRIPTION--
This test verifies that null members do not crash php
--FILE--
<?php
class Test extends Stackable {
    public function run(){}
}
$test = new Test();
@$test[$undefined]="what";
var_dump($test);
?>
--EXPECTF--
object(Test)#1 (1) {
  ["0"]=>
  string(4) "what"
}
