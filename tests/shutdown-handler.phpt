--TEST--
Test shutdown handlers #204
--DESCRIPTION--
Shutdown handlers that were closures were causing segfaults
--FILE--
<?php
class Test extends Thread {
        public function run() {
                register_shutdown_function(function(){
                        var_dump(new stdClass());
                });
        }
}

$test = new Test();
$test->start();
?>
--EXPECTF--
object(stdClass)#%d (0) {
}

