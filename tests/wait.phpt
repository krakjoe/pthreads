--TEST--
Test wait/notify
--DESCRIPTION--
This test will verify wait/notify functionality
--FILE--
<?php
class ThreadTest extends Thread {
	public function run(){
		$this->sent = $this->notify();
		var_dump($this->sent);
	}
}
$thread = new ThreadTest();
if($thread->start()) {
	/* 
		you only ever wait FOR something !
	*/
	if (!$thread->sent) {
		var_dump($thread->wait());
	} else printf("bool(true)\n");
	
	/* 
		note that:
			joining is necessary to check the value of sent
			by this point in the execution we have been notified
			or sent is set ( so no need to wait )
			but zend may not have yet assigned the resutlt of the call
			to notify to the variable in the thread
			joining here ensures that you get the value set in the thread
			and is only necessary because we are checking that value
	*/
	$thread->join();
	
	/* test that notification was sent by checking 
		the result of the call to notify from the threading context */
	var_dump($thread->sent);
} else printf("bool(false)\n");
?>
--EXPECT--
bool(true)
bool(true)
bool(true)