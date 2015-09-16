--TEST--
Test Volatile objects
--DESCRIPTION--
Threaded members of Threaded objects are immutable, for the sake of performance and ease of use.

Volatile objects' members are excempt from immutability, but can still be an immutable property.

This test both demonstrates and tests volatility and immutability.
--FILE--
<?php
class Member extends Volatile {

	public function method(array $thing) {
		return $this->thing = $thing; 			# fine, coerced to volatile ...
	}

	public function setThreaded() {
		$this->threaded = new Threaded();		# fine, volatile, threaded without refs, still works
		$this->threaded->merge($this->thing); 	# fine, but expensive
	}

	public function condition() {
		return true;
	}
}

class Test extends Thread {

	public function __construct(Member $member) {
		$this->member = $member; 				# immutable, Volatile is still Threaded and this is a normal
												# Threaded object (Thread extends Threaded)
	}

	public function run() {
		$members = [];
		if ($this->member->condition()) {		# calls to Volatile object methods still fast for this object
			$this->member
				->method([1,2,3,4,5]);
			var_dump($this->member->thing); 	# fine will display ^Threaded
			$this->member->setThreaded(); 		# fine not immutable because volatile

			$this->member = new Threaded(); 	# not fine, immutable		
		}
	}
}

$member = new Member();
$test = new Test($member);
$test->start();
$test->join();
?>
--EXPECTF--
object(Volatile)#%d (%d) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
  [4]=>
  int(5)
}

Fatal error: Uncaught RuntimeException: Threaded members previously set to Threaded objects are immutable, cannot overwrite member in %s:33
Stack trace:
#0 [internal function]: Test->run()
#1 {main}
  thrown in %s on line 33
