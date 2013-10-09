--TEST--
Test NULLing ressources in arrays
--FILE--
<?php
class file {
	public static $fps;

	public static function __callstatic($method, $args) {
		$tid = Thread::getCurrentThreadId();
		if (isset(self::$fps[$tid])) {
			return call_user_func_array(array("file", "_{$method}"), array_merge($args, array($tid)));
		} else {
			self::$fps[$tid] = fopen(__FILE__, "r+");
			if (isset(self::$fps[$tid]))
				return call_user_func_array(array("file", "_{$method}"), array_merge($args, array($tid)));
		}
	}
	
	public static function _get ($arg, $tid) {
		printf("%s: %s\n", __METHOD__, $arg);
		var_dump(self::$fps);
	}
}

class UserThread extends Thread {
	public function run () {
		/* execute calls */
		$i = 2;
		file::get("something".(++$i));
		file::get("something".(++$i));
		
	}
}

$i = 0;

file::get("something".(++$i));
file::get("something".(++$i));

$thread = new UserThread();
$thread->start();
?>
--EXPECTF--
file::_get: something1
array(1) {
  [%i]=>
  resource(5) of type (stream)
}
file::_get: something2
array(1) {
  [%i]=>
  resource(5) of type (stream)
}
file::_get: something3
array(2) {
  [%i]=>
  NULL
  [%i]=>
  resource(2) of type (stream)
}
file::_get: something4
array(2) {
  [%i]=>
  NULL
  [%i]=>
  resource(2) of type (stream)
}
