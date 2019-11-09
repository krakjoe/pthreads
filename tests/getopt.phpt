--TEST--
Test getopt() works as desired on threads
--ARGS--
--arg value --arg=value -avalue -a=value -a value
--INI--
register_argc_argv=On
variables_order=GPS
--FILE--
<?php
var_dump(getopt("a:", array("arg:")));

$t = new class extends Thread{
	public function run() : void{
		var_dump(getopt("a:", array("arg:")));
	}
};
$t->start() && $t->join();
?>
--EXPECT--
array(2) {
  ["arg"]=>
  array(2) {
    [0]=>
    string(5) "value"
    [1]=>
    string(5) "value"
  }
  ["a"]=>
  array(3) {
    [0]=>
    string(5) "value"
    [1]=>
    string(5) "value"
    [2]=>
    string(5) "value"
  }
}
array(2) {
  ["arg"]=>
  array(2) {
    [0]=>
    string(5) "value"
    [1]=>
    string(5) "value"
  }
  ["a"]=>
  array(3) {
    [0]=>
    string(5) "value"
    [1]=>
    string(5) "value"
    [2]=>
    string(5) "value"
  }
}
