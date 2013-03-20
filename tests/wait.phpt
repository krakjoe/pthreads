--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends Thread {
	public $sent;
	public function run(){
		$this->isSent(true);
	}
	
	/* good use of protection ! kudos, me !! */
	protected function isSent($flag = null) {
		if ($flag === null) {
			return $this->sent;
		} else {
		    $this->sent = $flag;
		    $this->synchronized(function($me){
		         $me->notify();
		    }, $this);
		    return $flag;
		}
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	/* 
		you only ever wait FOR something !
	*/
	$thread->synchronized(function($me){
	    if (!$me->isSent()) {
		    var_dump($me->wait());
	    } else printf("bool(true)\n");
	}, $thread);
	
	/* note that: this works because of protection */
	/* without protection, notify in the other thread can cause the process to continue */
	/* without the other thread having set the value of "sent" */
	/* the call to notify is made in the same protected method that writes the variable */
	/* and this thread uses that same method to access the variable */
	var_dump($thread->isSent());
} else printf("bool(false)\n");
?>
--EXPECT--
bool(true)
bool(true)
