--TEST--
Test class defaults
--DESCRIPTION--
Class defaults should now initialize defaults properly
--FILE--
<?php
class Test extends Thread {

	public function run(){
		var_dump($this->test);
	}
	
	protected $test = "hello world";
}

$test =new Test();
$test->start();
?>
--EXPECT--
string(11) "hello world"


