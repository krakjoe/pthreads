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

	public function run() {
		var_dump($this->method());
	}
}

$custom = new Custom();
$test = new Test($custom);
$test->start() && $test->join();
?>
--EXPECT--
object(Custom)#1 (0) {
}





