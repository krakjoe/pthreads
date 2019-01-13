--TEST--
Test Worker::unstack while a task is currently executing
--DESCRIPTION--
Unstacking a task would cause it to be freed from the worker stack, but a currently-executing task would not be told not to point to the destroyed task, resulting in an attempted double-free when collecting garbage from workers
--FILE--
<?php
$w = new Worker();
$w->start();

class Task extends Threaded{
    public function run() : void{
        sleep(1);
    }
}

for($i = 0; $i < 2; ++$i){
    $w->stack(new Task);
}

usleep(500000);
$w->unstack();
$w->shutdown();

var_dump("ok");
?>
--EXPECTF--
string(2) "ok"
