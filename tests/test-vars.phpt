
--TEST--
Testing object variable in declaration
--DESCRIPTION--
Test for object variables (array)
--FILE--
<?php
class MY {
        private $test = array();

        public function getTest(){
                return $this->test;
        }
}

class TEST extends Thread {
        public function __construct() {

        }

        public function run(){
		$MY = new MY();
              var_dump($MY->getTest());
        }
}

$test = new TEST();
$test->start();
?>
--EXPECT--
array(0) {
}

