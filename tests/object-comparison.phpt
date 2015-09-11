--TEST--
Test object comparison
--DESCRIPTION--
Threaded object comparison was broken in pthreads v2, the handler was not implemented.

In pthreads v3 we implement the handler, but not in the way Zend is expecting us too.

To traverse (and reconstruct in most cases) the property store in order to perform a comparison is extremely inefficient.

In addition, because of the way pthreads works, it might be useful to determine if two references from two different contexts are infact references
to the same object from some other context.

So pthreads v3 implements the comparison handler and returns true when two references are for the same Threaded object, this differs from normal object
comparison which would return true if the property tables of two distinct objects are uniform.
--FILE--
<?php
class Handler {
	public function handle(Exception $ex) {
		var_dump(__METHOD__, $ex->getMessage());
	}
}

class Test extends Thread {
	public function __construct(Threaded ... $args) {
		$this->args = $args;
	}

	public function run () {
		var_dump($this->args[0] == $this->args[1]);
	}
}

$a = new Threaded();
$b = new Threaded();

$test = new Test($a, $b);	# bool(false)
$test->start();
$test->join();

$test = new Test($a, $a);	# bool(true)
$test->start();
$test->join();
?>
--EXPECT--
bool(false)
bool(true)

