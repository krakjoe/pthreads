--TEST--
Test statics aren't getting fucked with when starting new threads (many bugs)
--FILE--
<?php
class StaticClass{
    public static $list = array();
    public static $test;
    public static $testObject;

    public function __construct(){
        self::$list[] = $this;
        self::$test = "randomvalue";
    }
}

class ThreadClass extends Thread{   

}

new StaticClass;
StaticClass::$testObject = new StaticClass;
echo "BEFORE:\n";
var_dump(StaticClass::$list, StaticClass::$test, StaticClass::$testObject);
echo "\n";

$thread = new ThreadClass;
$thread->start();

echo "AFTER\n";
var_dump(StaticClass::$list, StaticClass::$test, StaticClass::$testObject);
echo "\n";
--EXPECTF--
BEFORE:
array(2) {
  [0]=>
  object(StaticClass)#%d (0) {
  }
  [1]=>
  object(StaticClass)#%d (0) {
  }
}
string(11) "randomvalue"
object(StaticClass)#%d (0) {
}

AFTER
array(2) {
  [0]=>
  object(StaticClass)#%d (0) {
  }
  [1]=>
  object(StaticClass)#%d (0) {
  }
}
string(11) "randomvalue"
object(StaticClass)#%d (0) {
}

