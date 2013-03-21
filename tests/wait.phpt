--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends Thread {
	public $sent;
	
	public function run(){
	    sleep(1);
		$this->sent = true;
		$this->synchronized(function(){
		    $this->notify();
		});
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	$thread->lock();
	$thread->synchronized(function($me){
	    if (!$me->sent) {
		    $me->unlock();
		    var_dump($me->wait());
	    } else $me->unlock();
	}, $thread);
	
} else printf("bool(false)\n");
?>
--EXPECT--
bool(true)
