--TEST--
Testing interfaces inheritance 
--DESCRIPTION--
Test if interfaces are stored in thread
--FILE--
<?php
interface iMY{
        function getTest();
}

class MY implements iMY{
        private $test = "Hello World";

        public function getTest(){
                return $this->test;
        }
}

class TEST extends Thread {
        public function __construct() {

        }

        public function run(){
		$MY = new MY();
               var_dump(in_array("iMY",get_declared_interfaces()));
		var_dump(in_array("iMY",class_implements($MY)));
        }
}

$test = new TEST();
$test->start();
?>
--EXPECT--
bool(true)
bool(true)

