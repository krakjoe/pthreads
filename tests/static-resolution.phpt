--TEST--
Test static:: regression
--DESCRIPTION--
Bug #210 shows static:: requires different logic to self::
--FILE--
<?php
class testbug extends Thread
{
    public static $somevar = 123;
    
    function __construct()
    {
        var_dump(self::$somevar); 
        var_dump(static::$somevar);
    }

    public function run()
    {
        var_dump(self::$somevar);
        var_dump(static::$somevar);
    }
}

$testbug = new testbug;
$testbug->start();
?>
--EXPECT--
int(123)
int(123)
int(123)
int(123)
