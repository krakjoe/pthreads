--TEST--
Test Thread::globally
--DESCRIPTION--
Test globally voodoo
--FILE--
<?php
class Test extends Thread {
	public function run() {
		/* this will be created in the global scope */
		global $var;
		
		Thread::globally(function() {
			/* we are in the global scope */
			$var = new stdClass;
		});
		
		Thread::globally(Closure::bind(function(){
			/* we are both in the thread */
			var_dump($this);
			/* an in the global scope */
			var_dump($var);
		}, $this));
		
		/* we now have this variable */
		var_dump($var);
	}
}

$test = new Test();
$test->start();
$test->join();
?>
--EXPECTF--
object(Test)#%d (%d) {
}
object(stdClass)#%d (%d) {
}
object(stdClass)#%d (%d) {
}

