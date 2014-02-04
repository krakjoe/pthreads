--TEST--
This file contains a testcase showing a problem with predefined membervariables and the needed testThread.
--DESCRIPTION--
While programming with pthreads an error occured with a membervariable, that was empty although predefined in the descriptionheader of the php class. The observation had been made, that only membervariables set in the constructor are valued in the threadcontext. Currently we are using php in version 5.58.
--FILE--
<?php
define ("CONTAINERKEY" , "PredefinedMemberVariableTest");
define ("EXPECTED" , 1);


class TestThread extends Thread
{
    protected $predefinedMember = EXPECTED; //protected $predefinedMember;
    protected $container;

    public function __construct($container)
    {
        $this->container = $container;
        //$this->predefinedMember = EXPECTED;
    }

    public function run()
    {
        $this->container[CONTAINERKEY] = $this->predefinedMember;
    }
}

class TestContainer extends Stackable
{
    public function run()
    {
    }
}

$testContainer = new TestContainer();
$testThread = new TestThread($testContainer);
$testThread->start();
$testThread->join();

var_dump($testContainer);
?>
--EXPECT--
object(TestContainer)#1 (1) {
  ["PredefinedMemberVariableTest"]=>
  1
}