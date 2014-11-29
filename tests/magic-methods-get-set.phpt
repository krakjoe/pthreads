--TEST--
Test magic __get and __set
--DESCRIPTION--
This test verifies that __set and __get work as expected
--FILE--
<?php
class Test extends Threaded {

    public function __get($key) {
        printf(
            "magic %s\n", __FUNCTION__);
        return $this[$key];
    }
    
    public function __set($key, $value) {
        printf(
            "magic %s\n", __FUNCTION__);
        return $this[$key] = $value;
    }
}

$test = new Test();
$test->one = "one";
var_dump($test->one);

?>
--EXPECT--
magic __set
magic __get
string(3) "one"
