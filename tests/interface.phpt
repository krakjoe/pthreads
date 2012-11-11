--TEST--
Test interface functionality
--DESCRIPTION--
This test verifies that interfaces are handled properly by pthreads
--FILE--
<?php
interface INamedThread {
	function setName($name);
	function getName();
}

class TestThread extends Thread implements INamedThread {
	public function setName($name) {
		$this->name = $name;
	}
	public function getName() {
		return $this->name;
	}
	
	public function run() { printf("%s\n", $this->getName()); }
}

$thread = new TestThread();
$thread->setName("InterfaceTest");
$thread->start();
?>
--EXPECT--
InterfaceTest
