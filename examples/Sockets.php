<?php

class Test extends Thread
{
    public $stop = false;

    public function __construct($socket)
    {
        $this->socket = $socket;
    }

    /**
     * {@inheritdoc}
     * @see Thread::run()
     */
    public function run()
    {
        $clients = 0;
        while (++$clients < 10 && ($client = socket_accept($this->socket))) {
            printf("Accept in %lu\n", $this->getThreadId());
            var_dump($client);
            var_dump($this->socket);
            socket_close($client);
        }
    }
}
$workers = array();
$sock = socket_create_listen($argv[1]);
if ($sock) {
    for ($worker = 0; $worker < 5; $worker++) {
        $workers[$worker] = new Test($sock);
        $workers[$worker]->start();
    }
    printf("%d threads waiting on port %d\n", count($workers), $argv[1]);
}
