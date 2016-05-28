--TEST--
Testing class statics properties (gh bug #570)
--DESCRIPTION--
Class static properties were messy, extremely messy in PHP 7 so far.

We are back to the original rules, that complex unsafe members will be removed from statics:

	normal PHP class: removed
	threaded class: remains
	closure: remains
	resource: removed
	array: remains, rules above applied
--FILE--
<?php
class Test extends Threaded {

   public static $prop1 = "one";
   public static $prop2 = [1, 2, 3];
   public static $prop3 = [[1,2,3], [4,5,6]];
   public static $prop4;
   public static $prop5;
   public static $prop6;
   public static $prop7;
   public static $prop8;
}

Test::$prop4 = new Threaded;
Test::$prop5 = new stdClass; /* will be null */
Test::$prop6 = function() { /* will be copied with voodoo */
	return [1,2,3];
};

Test::$prop7 = fopen("php://stdin", "r");
Test::$prop8 = [fopen("php://stdin", "r")];

$test = new class extends Thread {
	public function run() {
		var_dump(Test::$prop1,
				 Test::$prop2,
				 Test::$prop3,
				 Test::$prop4,
				 Test::$prop5,
				 Test::$prop6, (Test::$prop6)(),
				 Test::$prop7,
				 Test::$prop8);
	}
}; 

$test->start() && $test->join();
--EXPECT--
string(3) "one"
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}
array(2) {
  [0]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
  [1]=>
  array(3) {
    [0]=>
    int(4)
    [1]=>
    int(5)
    [2]=>
    int(6)
  }
}
object(Threaded)#1 (0) {
}
NULL
object(Closure)#2 (0) {
}
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}
NULL
array(0) {
}

