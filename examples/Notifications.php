<?php
/*
* Note: in 5.4 series the output is handled differently from multiple threads on the command line, sometimes giving the idea that this sort of thing is executing the opposite to how it should...
* It isn't, but there's not much to be done about the new output handling in 5.4 ... we'll just have to live with it ...
*/
class ExampleThread extends Thread {
	public function run(){
			printf("The creating thread will not budge until I:");
			$this->notify();
	}
}

$t = new ExampleThread();

if ($t->start(true)) {
	printf(" ... I'm back from start ...");
}
?>