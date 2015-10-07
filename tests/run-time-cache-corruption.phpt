--TEST--
Test run time cache bug #494
--DESCRIPTION--
Runtime Caches were being incorrectly used (shared!), this caused strange behaviour for various things
--FILE--
<?php
interface TestInterface {}

class DebugClass implements TestInterface {}

class Some extends Threaded {
    public static function staticNess() {
        $closure = function() : DebugClass {
            return (new \DebugClass());
        };

        $objCreated = $closure();
        var_dump((new \ReflectionClass($objCreated))->getInterfaceNames());
		var_dump($objCreated instanceof \TestInterface);
    }
}

class Test extends Thread {
    public function run(){
        Some::staticNess();
    }
}

Some::staticNess();

$test = new class extends Thread {
	public function run() {
		Some::staticNess();
	}
};

$test->start(PTHREADS_INHERIT_NONE | 
			 PTHREADS_INHERIT_INI | 
			 PTHREADS_ALLOW_HEADERS | 
			 PTHREADS_INHERIT_COMMENTS | 
			 PTHREADS_INHERIT_INCLUDES | 
			 PTHREADS_INHERIT_FUNCTIONS | 
			 PTHREADS_INHERIT_CLASSES) && $test->join();
?>
--EXPECT--
array(1) {
  [0]=>
  string(13) "TestInterface"
}
bool(true)
array(1) {
  [0]=>
  string(13) "TestInterface"
}
bool(true)



