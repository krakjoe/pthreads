<?php
/*
	Rules:
		1. only ever wait FOR something
		2. only ever wait FOR something
		3. only ever wait FOR something
	
	Read the code ...
*/
$thread = new class extends Thread {
	
	public function run() {
		
		$this->synchronized(function(){
			$this->awake = true;
			$this->notify();
		});
	}
};

$thread->start();
$thread->synchronized(function() use($thread) {
	while (!$thread->awake) {
		/*
			If there was no precondition above and the Thread
			managed to send notification before we entered this synchronized block
			we would wait forever!

			We check the precondition in a loop because a Thread can be awoken
			by signals other than the one you are waiting for.
		*/
		$thread->wait();
	}
});
$thread->join();
?>
