--TEST--
Constants copying of an included interface
--DESCRIPTION--
We need to copy constants which are defined by a via autoloader included interface
--FILE--
<?php

class AutoLoader {
    public function __construct() {
        spl_autoload_register(array($this, 'loadClass'));
    }

    private function loadClass() {
        require __DIR__ .'/assets/ExternalConstantsInterface.php';
    }
}
new AutoLoader();

class MyClass implements ExternalConstantsInterface {}

var_dump(MyClass::BUZZ);

$thread = new Thread();
$thread->start(PTHREADS_INHERIT_ALL) && $thread->join();
--EXPECT--
int(1)