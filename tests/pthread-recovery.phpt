--TEST--
Test pthread object recovery
--DESCRIPTION--
This test verifies the pthread object recovery of a joined thread
--FILE--
<?php
$objGlobalShare = new Threaded();
$objInstance    = new Threaded();

class TestOne extends Thread
{
    private $objGlobalShare;
    private $objInstance;

    public function __construct(Threaded $globalShare, Threaded $instance)
    {
        $this->objGlobalShare   = $globalShare;
        $this->objInstance      = $instance;
    }

    public function run()
    {
        $this->objGlobalShare['anykey'] = $this->objInstance;

        $this->objInstance[] = 'set by thread';
    }
}

$objTestOne = new TestOne($objGlobalShare, $objInstance);
$objTestOne->start();
$objTestOne->join();

var_dump($objGlobalShare);
?>
--EXPECTF--
object(Threaded)#1 (1) {
  ["anykey"]=>
  object(Threaded)#2 (1) {
    [0]=>
    string(13) "set by thread"
  }
}