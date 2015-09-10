--TEST--
Test managing refcount in userland (super advanced voodoo shit, leave it alone)
--DESCRIPTION--
pthreads v3 does allow the programmer to store references to other Threaded objects as members without references.
It still might be useful in a complex system to manipulate the refcount of Threaded objects, so this stays.

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
    int(%s)
  }
  ["inrun"]=>
  object(Managed)#2 (1) {
    ["owner"]=>
    int(%s)
  }
}

