--TEST--
Test fix for #659
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php
$task = new class extends Thread {

    public function run()
    {
        $this->prop = new class {};

		var_dump($this->prop); /* we do expect null: anon classes cannot be serialized */
    }
};

$task->start() && $task->join();
--EXPECT--
NULL
