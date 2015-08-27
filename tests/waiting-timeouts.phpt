--TEST--
Test waiting timeouts
--DESCRIPTION--
This test verifies that reaching a timeout returns the correct value

Note that this is a dodgy test, ONLY EVER WAIT FOR SOMETHING
We assume, wrongly, that the thread will be active and waiting within 3
seconds.
--FILE--
<?php
class T extends Thread {
        public $data;
        public function run() {
			$start = time();
            $this->synchronized(function() use($start) {
				while (time() - $start < 3) {
					$this->wait(100000);
				}
			});
        }
}

$t = new T;
$t->start();
$t->synchronized(function($thread){
	var_dump($thread->wait(100)); # should return false because no notification sent
								  # but may wake up (and return true) because notification might come from
								  # somewhere else in the system and there is no precondition
								  # it will normally be safe to ignore this test failing
}, $t);
?>
--EXPECT--
bool(false)
