<?php
class Test extends Thread {
	public $stop = false;
	
	public function __construct($socket){
		$this->socket = $socket;
	}
	
	public function run(){
		while(++$clients < 10 &&
			($client = socket_accept($this->socket))){
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
	while(++$worker<5){
		$workers[$worker] = new Test($sock);
		$workers[$worker]->start();
	}
	printf("%d threads waiting on port %d\n", count($workers), $argv[1]);
}

foreach ($workers as $thread)
    $thread->join();
?>
