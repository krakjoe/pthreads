--TEST--
Test fix for #658 with inheritance
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php
$task = new class extends Thread {
    public function run()
    {
        $this->prop = new class extends Threaded {};
		var_dump($this->prop);
    }
};
$task->start() && $task->join();
--EXPECT--
object(class@anonymous)#2 (0) {
}