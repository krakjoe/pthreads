--TEST--
Test magic __isset and __unset
--DESCRIPTION--
This test verifies that __isset and __unset work as expected
--FILE--
<?php
class Test extends Threaded {

    public function __isset($key) {
        printf("magic %s\n", __FUNCTION__);
        
        return isset($this[$key]);
    }
    
    public function __unset($key) {
        printf("magic %s\n", __FUNCTION__);
        
        unset($this[$key]);
    }
}
$test = new Test();
$test->one = 1;
var_dump(isset($test->one));
unset($test->one);
var_dump(isset($test->one));
?>
--EXPECT--
magic __isset
bool(true)
magic __unset
magic __isset
bool(false)
