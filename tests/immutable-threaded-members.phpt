--TEST--
Test immutability
--DESCRIPTION--
To provide the best performance and and predictability, members of Threaded objects that are set to
other Threaded objects are immutable, that is you cannot unset or write a member of a Threaded object
if the member itself is a Threaded object.

This test ensures that immutability works as intended with no strange side effects.
--FILE--
<?php
class T extends Thread {

	public function __construct() {
		$this->t = new Threaded(); 	# now immutable
		$this->t->set = true; 		# fine
		$this->s = new stdClass(); 	# fine, not immutable
	}
	
    public function run(){
		$this->synchronized(function(){
			while (!$this->printed) {
				$this->wait(); 			# wait for object to be printed in main context
			}
		});
		
		try {
			$this->t = new Threaded(); 	# can't do it, immutable member
		} catch(Exception $ex) {
			printf("%s in %s on line %d\n", $ex->getMessage(), $ex->getFile(), $ex->getLine());
		}

		try {
			unset($this->t); 			# can't do it, immutable member
		} catch(Exception $ex) {
			printf("%s in %s on line %d\n", $ex->getMessage(), $ex->getFile(), $ex->getLine());
		}
	}
}

$t = new T();
var_dump($t->t, $t->s);
$t->start();

$t->synchronized(function() use($t) {
	var_dump($t->t, $t->s);
	$t->printed = true;	
	$t->notify();
});

$t->join();

try {
	$t->t = new Threaded(); 		# can't do it, immutable member
} catch(Exception $ex) {
	printf("%s in %s on line %d\n", $ex->getMessage(), $ex->getFile(), $ex->getLine());
}

try {
	unset($t->t);					# can't do it, immutable member
} catch(Exception $ex) {
	printf("%s in %s on line %d\n", $ex->getMessage(), $ex->getFile(), $ex->getLine());
}

var_dump($t->t);					# not mutated
--EXPECTF--
object(Threaded)#%d (%d) {
  ["set"]=>
  bool(true)
}
object(stdClass)#%d (%d) {
}
object(Threaded)#%d (%d) {
  ["set"]=>
  bool(true)
}
object(stdClass)#%d (%d) {
}
Threaded members previously set to Threaded objects are immutable, cannot overwrite t in %s on line 18
Threaded members previously set to Threaded objects are immutable, cannot overwrite t in %s on line 24
Threaded members previously set to Threaded objects are immutable, cannot overwrite t in %s on line 44
Threaded members previously set to Threaded objects are immutable, cannot overwrite t in %s on line 50
object(Threaded)#%d (%d) {
  ["set"]=>
  bool(true)
}

