--TEST--
Test RECV overload
--DESCRIPTION--
When using the explicit cast of objects, type hinting can be broken.

The class in the current context doesn't match the class of the object.

pthreads overrides ZEND_RECV to fix the problem, nothing else is affected.
--FILE--
<?php
class Test extends Thread {
	
	public function __construct(Custom $member) {
		$this->member = $member;
	}

	public function run() {
		$this->method(
			(object)$this->member);
	}

	public function method(Custom $member) {}

	private $member;
}

class Custom extends Threaded {}

$member = new Custom();

$test = new Test($member);

$test->start() && $test->join();

var_dump($test);
?>
--EXPECTF--
object(Test)#%d (1) {
  ["member"]=>
  object(Custom)#%d (0) {
  }
}




