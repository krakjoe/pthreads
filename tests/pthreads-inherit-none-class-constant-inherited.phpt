--TEST--
Inherited class constant in PTHREADS_INHERIT_NONE thread
--DESCRIPTION--
This test ensures that inherited class constants are available inside in PTHREADS_INHERIT_NONE threads without error.
--FILE--
<?php
class baseTest extends \Thread {

    const TEST_CONSTANT = 0x00;

    public function test() {
        $this->synchronized(function () {
            var_dump("Constant: " . self::TEST_CONSTANT);
        });
    }
}

class testClass extends baseTest {

    public function run() {
        $this->test();
    }
}

$x = new testClass();
$x->start(PTHREADS_INHERIT_NONE);
$x->join();
--EXPECT--
string(11) "Constant: 0"
