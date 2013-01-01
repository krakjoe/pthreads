<?php
/*
* New Synchronization exposed in userland ... here's a brief
*
* You can now enter a block of code synchronized, (almost) like you can in java.
* When you enter a synchronized block, you acquire the mutex associated with the objects monitor.
* Because of the way Posix Conditions work, this allows calls to notify/wait to be truly synchronized.
* The explanation of this is long, boring and not required to operate pthreads.
* You may still call notify/wait on any object unsynchronized, however in a lot of cases this can be undesireable
* The old way of locking the monitor for you lead to restricted functionality, and a high chance of a programming error in PHP.
*
* Because of differences in PHP 5.4 and 5.3, I will give the most compatible example of syntax, which you should emulate, or use some other
* compatible method if you are not specifically targetting 5.4
*
*/

class Test extends Thread {	
	public $done = false;
	
	public function run(){
		/* some code here */

		/* in 5.4, referencing $this is possible */
		/* in 5.3, you cannot "use" $this */
		/* making the following the most compatible synchronized block */
		$this->synchronized(function($self){
			printf("%s(%lu) going to wait ...\n", __CLASS__, $self->getThreadId());
			$self->wait();
			printf("%s(%lu) woken\n", __CLASS__, $self->getThreadId());
			$self->done = true;
		}, $this);

		/* some more here */
	}
}

$test = new Test();
$test->start();

/* notice that I have not "used" $self */
/* this is to show that every parameter passed after the block
	is passed directly to the block */
/* you can equally make use of "use" and this functionality for added flexibility */
$test->synchronized(function($self){
	printf("Process: synchronized.\n");
	if (!$self->done) {
		$self->notify();
	} else printf("Process: error\n");
}, $test);
?>
