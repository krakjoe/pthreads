<?php
class SocketServer extends Thread {
	public $socket;
	public $maxThreads;
	public $isRunning;
	public $threads;
	public $host;
	public $port;
	public $backlog;
	public function __construct(int $maxThreads, string $host, int $port, int $backlog) {
		$this->maxThreads = $maxThreads;
		$this->isRunning = false;
		$this->threads = [ ];
		$this->host = $host;
		$this->port = $port;
		$this->backlog = $backlog;
	}
	public function run() {
		$this->socket = new Socket ( AF_INET, SOCK_STREAM, SOL_TCP );
		
		$this->socket->setOption ( SOL_SOCKET, SO_REUSEADDR, 1 );
		
		$this->socket->bind ( $this->host, $this->port );
		
		$this->socket->listen ( $this->backlog );
		
		$this->socket->setBlocking ( false );
		
		$this->isRunning = true;
		
		for($i = 0; $i < $this->maxThreads; $i ++) {
			$this->threads [$i] = new RequestHandler ( $this->socket );
			$this->threads [$i]->start ();
		}
		
		$this->synchronized ( function () {
			do {
				foreach ( $this->threads as $key => $thread ) {
					if ($thread->isRunning ()) {
						continue;
					}
					$thread->join ();
					$this->threads [$key] = new RequestHandler ( $this->socket );
					$this->threads [$key]->start ();
				}
				$this->wait ( 500000 ); // 500 ms
			} while ( $this->isRunning );
		} );
	}
	public function shutdown() {
		$this->isRunning = false;
		foreach ( $this->threads as $thread ) {
			$thread->shutdown ();
		}
		$this->socket->close ();
	}
}
class RequestHandler extends Thread {
	public $mainSocket;
	public $threadSocket;
	public $isRunning;
	public function __construct(Socket $socket) {
		$this->mainSocket = $socket;
		$this->isRunning = false;
	}
	public function run() {
		$this->isRunning = true;
		
		while ( $this->isRunning ) {
			
			$this->threadSocket = $this->mainSocket->accept ();
			
			if ($this->threadSocket === false) {
				$this->synchronized ( function () {
					$this->wait ( 5000 ); // 5ms
				} );
				continue;
			}
			
			$threadId = $this->getThreadId ();
			
			$response = "Welcome\nYou are connected with Thread-ID $threadId\nEnter \"quit\" to quit\n\n";
			
			$this->threadSocket->write ( $response, strlen ( $response ) );
			
			do {
				$buffer = trim ( $this->threadSocket->read ( 1024 ) );
				
				if ('quit' === $buffer) {
					break;
				}
				
				$talkBack = "You entered $buffer.\n";
				
				$this->threadSocket->write ( $talkBack, strlen ( $talkBack ) );
			} while ( $this->isRunning );
			
			$this->threadSocket->write ( "Goodbye\n", 9 );
			$this->threadSocket->close ();
			
			$this->isRunning = false;
		}
	}
	public function shutdown() {
		$this->isRunning = false;
	}
}

echo 'Multi-Threaded Socket Server started with PID ' . posix_getpid () . "\n";

$server = new SocketServer ( 3, '127.0.0.1', 9004, 2 );
$server->start ();

$running = true;

$shutdown = function () use (&$running, $server) {
	echo "Shutting down... \n";
	$running = false;
	$server->shutdown ();
	$server->join ();
	echo "Finished\n";
};

pcntl_signal ( SIGINT, $shutdown );

while ( $running ) {
	pcntl_signal_dispatch ();
	usleep ( 10000 );
}
