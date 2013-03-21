--TEST--
Testing member count
--DESCRIPTION--
This test verifies that getting member counts works without effort
--FILE--
<?php
class Test extends Stackable {
	public function run() { 
	}
}

$t = new Test();
$t[] = "one";
$t[] = "two";
$t["three"] = "three";
var_dump(count($t));
?>
--EXPECT--
int(3)
