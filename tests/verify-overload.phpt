--TEST--
Test VERIFY_RETURN_TYPE overload
--DESCRIPTION--
When using the explicit cast of objects, return type hinting can be broken.

The class in the current context doesn't match the class of the object.

pthreads overrides ZEND_VERIFY_RETURN_TYPE to fix the problem, nothing else is affected.
--FILE--
<?php
class Custom extends Threaded {}

class Test extends Thread {

	public function __construct(Custom $custom) {
		$this->custom = $custom;
	}

	public function method() : Custom {
		return (object) $this->custom;
	}

	public function checkReturnType():int {
		return 1;
	}

	public function wrongReturnType():string {
		return 1;
	}

	public function checkOptionalNull():?Threaded {
		return ($var = null);
	}

	public function run() {
		var_dump($this->method());
		var_dump($this->checkReturnType());
		var_dump($this->wrongReturnType());
		var_dump($this->checkOptionalNull());
	}
}

$custom = new Custom();
$test = new Test($custom);
$test->start() && $test->join();
?>
--EXPECTF--
object(Custom)#%d (0) {
}
int(1)
string(1) "1"
NULL

