--TEST--
Test nested volatile variables
--DESCRIPTION--
This test verifies the possibility to nest volatile variables
--FILE--
<?php
class Node extends Volatile {}

class TestNestedWrite extends Thread {
    private $shared;

    public function __construct(Volatile $shared) {
        $this->shared = $shared;
    }

    public function run() {
        var_dump($this->shared['queue'][0]);

        // link to new var
        $this->shared['queue'][1] = $this->shared['queue'][0];

        // unset old ref
        //unset($this->shared['queue'][0]);

        // or replace ref
        $this->shared['queue'][0] = new Volatile();

        $this->shared['lock'] = true;

        while(!isset($this->shared['lock2'])) {}

        var_dump($this->shared['queue'][1]);

        $this->shared['lock3'] = true;
    }
}

class TestNestedRead extends Thread {
    private $shared;

    public function __construct(Volatile $shared) {
        $this->shared = $shared;
    }

    public function run() {
        while(!isset($this->shared['lock']));

        var_dump($this->shared['queue'][1]);

        $this->shared['queue'][1] = new Node();
        $this->shared['lock2'] = true;

        while(!isset($this->shared['quit']));
    }
}

class Test extends Thread {
    public function run() {
        $queue = new Volatile();
        $queue[0] = new Volatile();

        $shared = new Volatile();
        $shared['queue'] = $queue;

        $thread = new TestNestedWrite($shared);
        $thread->start();

        $thread2 = new TestNestedRead($shared);
        $thread2->start();

        while(!isset($shared['lock3']));

        $shared['quit'] = true;

        $thread2->join();
        $thread->join();
    }
}
$thread = new Test();
$thread->start();
$thread->join();
?>
--EXPECT--
object(Volatile)#4 (0) {
}
object(Volatile)#4 (0) {
}
object(Node)#6 (0) {
}

