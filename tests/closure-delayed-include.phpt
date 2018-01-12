--TEST--
Closure copying of an included closure
--DESCRIPTION--
We need to copy closures which are defined and bound by included classes
--FILE--
<?php

class Foo extends \Thread {
    /** @var bool */
    public $running;

    /** @var \Threaded */
    private $shared;

    public function __construct(\Threaded $shared) {
        $this->shared = $shared;
    }

    public function run() {
        $this->running = true;

        require __DIR__ .'/assets/ExternalClosureDefinition.php';

        $this->shared['loader'] = new ExternalClosureDefinition();
        $this->synchronized(function () {
            while($this->running) {
                $this->wait(0);
            }
        });
     }
}

$shared = new \Threaded();

$foo = new Foo($shared);
$foo->start();

while(true) {
    if(!isset($shared['loader'])) {
        continue;
    }
    $closureDefinition = $shared['loader'];
    $closureDefinition->load();
    break;
}
$foo->running = false;
$foo->notify() && $foo->join();
--EXPECT--
string(11) "Hello World"