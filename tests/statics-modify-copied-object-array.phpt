--TEST--
Test static object arrays work correctly when modified after thread start
--FILE--
<?php

class Test{
	public static $array = [];

	public static function init(int $i){
		while(--$i > 0){
			self::$array[] = new stdClass();
		}
	}
}

Test::init(3);

$thread = new Worker;
$thread->start(PTHREADS_INHERIT_CLASSES);

var_dump(Test::$array);
Test::$array = [];
var_dump(Test::$array);

echo "gc start\n";
gc_collect_cycles();
echo "gc end\n";
$thread->shutdown();
echo "script end\n";

?>
--EXPECTF--
array(2) {
  [0]=>
  object(stdClass)#1 (0) {
  }
  [1]=>
  object(stdClass)#2 (0) {
  }
}
array(0) {
}
gc start
gc end
script end