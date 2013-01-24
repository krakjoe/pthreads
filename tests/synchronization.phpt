--TEST--
Test synchronized blocks
--DESCRIPTION--
This test verifies that syncronization is working
--FILE--
<?php
class T extends Thread {
        public $data;
        public function run() {
            $this->synchronized(function($thread){
				$thread->data = true;
				$thread->notify();
			}, $this);               
        }
}
$t = new T;
$t->start();
/* will return boolean false the data should not yet be set ( the timeout was reached ) */
$t->synchronized(function($thread){
	if (!$thread->data) {
		var_dump($thread->wait());
	} else var_dump($thread->data);
}, $t);
?>
--EXPECT--
bool(true)
