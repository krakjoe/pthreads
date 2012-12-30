<?php
class Test extends Thread {
	public $stop = false;
	
	public function __construct($socket){
		$this->socket = $socket;
	}
	
	public function run(){
		while(++$clients < 10 &&
			($client = socket_accept($this->socket))){
			var_dump($client);
			var_dump($this->socket);
			socket_close($client);
		}
	}
}
$sock = socket_create_listen(9090);
if ($sock) {
	$test = new Test($sock);
	$test->start();
}

?>
