--TEST--
Test Threaded objects are still usable in ::__destruct()
--DESCRIPTION--
Test that threaded object are not destroyed too early
--FILE--
<?php
class Work extends Threaded
{
    private $test = true;

    public function __destruct()
    {
        var_dump(self::isRunning());
	var_dump($this);
    }
}
$work = new Work;
?>
--EXPECT--
bool(false)
object(Work)#1 (1) {
  ["test"]=>
  bool(true)
}

