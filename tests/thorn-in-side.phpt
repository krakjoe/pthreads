--TEST--
Testing thorn in my side hasn't returned
--DESCRIPTION--
This test verifies my nemisis has not returned
--FILE--
<?php
class parentClass {
    private $var;

    public function __construct() {
        echo $this->var;
    }
}

class childClass extends parentClass {

}

class clientThread extends Thread {

    public function run() {
        $objChild = new childClass();

    }               

}


$objClientThread = new clientThread();
$objClientThread->start();
--EXPECT--
