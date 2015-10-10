--TEST--
Test static scope vars gh issue #501
--DESCRIPTION--
Static member reference vars were not copied appropriately
--FILE--
<?php
class Test extends Threaded
{
    public function method()
    {
        static $string = 'value';
		static $int = 1;
		static $array = [1,2,3];

        var_dump($string, $int, $array);
    }
}
$objInstance = new Test();
$objInstance->method();

$objThread = new class extends Thread
{
    public function run()
    {
        $objInstance = new Test();
        $objInstance->method();
    }
};

$objThread->start();
$objThread->join();
--EXPECT--
string(5) "value"
int(1)
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}
string(5) "value"
int(1)
array(3) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
}

