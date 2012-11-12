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
		} else return ($this->sent = $this->notify());
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	/* 
		you only ever wait FOR something !
	*/
	if (!$thread->isSent()) {
		var_dump($thread->wait());
	} else printf("bool(true)\n");
	
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