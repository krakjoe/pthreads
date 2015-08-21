--TEST--
Test managing refcount in userland (super advanced voodoo shit, leave it alone)
--DESCRIPTION--
You are responsible for the objects you create; setting a member of a Threaded object
to another Threaded object with no other references is problematic since a Threaded
object does not touch reference counts for member data. Exposing addRef/delRef/getRefCount
allows the programmer to work around this problem.

This is an advanced feature, if you don't know what you're doing, *don't touch it*.

*I will be EXTREMELY reluctant to help with issues caused by the programmer trying to deal with refcounts*
--FILE--
<?php
class Managed extends Threaded {
	private $owner;
	
	public function __construct() {
		$this->addRef();
		$this->owner = 
			Thread::getCurrentThreadId();
	}

	public function __destruct() {
		if (Thread::getCurrentThreadId() == $this->owner) {
			$this->delRef();
		}
			
	}
}

class Test extends Thread {
	public function __construct() {
		$this->inctor = new Managed();
	}

	public function run() {
		$this->inrun = new Managed();

		var_dump($this);
	}
}

$thread = new Test();
$thread->start();
$thread->join();
--EXPECTF--
object(Test)#1 (2) {
  ["inctor"]=>
  object(Managed)#3 (1) {
    ["owner"]=>
    int(%d)
  }
  ["inrun"]=>
  object(Managed)#2 (1) {
    ["owner"]=>
    int(%d)
  }
}

