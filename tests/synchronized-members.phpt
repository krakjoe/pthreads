--TEST--
Test member sync ( 10 second test )
--DESCRIPTION--
This test verifies that syncronization on a member is working
--FILE--
<?php
class T extends Thread {
        public $data;
        public function run() {
            $this->synchronized(function(){
				$this->data = true;
			});               
        }
}
$t = new T;
$t->start();
/* will return boolean false the data should not yet be set ( the timeout was reached ) */
$t->synchronized(function($thread){
	var_dump($thread->wait("data"));
}, $t);
?>
--EXPECT--
bool(true)
