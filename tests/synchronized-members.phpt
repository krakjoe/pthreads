--TEST--
Test member sync ( 10 second test )
--DESCRIPTION--
This test verifies that syncronization on a member is working
--FILE--
<?php
define("WAIT_TIME", 10000000);
define("WAIT_OFF", WAIT_TIME/10);

class T extends Thread {
        public $data;
        public function run() {
                usleep(WAIT_TIME);
                $this->data = true;
        }
}
$t = new T;
$t->start();
/* will return boolean false the data should not yet be set ( the timeout was reached ) */
var_dump($t->wait("data", WAIT_TIME/2));
/* will return boolean true as the data is now set */
var_dump($t->wait("data", WAIT_TIME+WAIT_OFF));
?>
--EXPECT--
bool(false)
bool(true)
