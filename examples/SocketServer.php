<?php
/*
* @NOTE
*	RESOURCES ARE BEING TRIED OUT, THIS MAY CRASH OR KILL THE EXECUTOR OF THE SCRIPT: NOT MY FAULT
*
*	It's pretty clear that people want resources to work, and I cannot deny it would be useful.
*	Officially, resources are unsupported, if it doesn't work there is NOTHING I can do about it.
*	I do NOT have the time to make every kind of resource safe, or ensure compatibility.
*	I can apply some cleverness to make sure resources are destroyed by whoever creates them, that is ALL.
*	In the case of sockets, this appears to work, the code below ran all day on pthreads.org without interruption.
*	AGAIN: 	If this does not work, there is NOTHING more I can do. 
			If a particular type of resource wigs out, there is NOTHING I can do.
			If this kills you, horribly, there is NOTHING I can do.
*/
class Client extends Thread {
	public function __construct($socket){
		$this->socket = $socket;
		$this->start();
	}
	public function run(){
		$client = $this->socket;
		if ($client) {
			$header = 0;
			while(($chars = socket_read($client, 1024, PHP_NORMAL_READ))) {
				$head[$header]=trim($chars);
				if ($header>0) {
					if (!$head[$header] && !$head[$header-1])
						break;
				}
				$header++;
			}
			foreach($head as $header) {
				if ($header) {
					$headers[]=$header;	
				}
			}

			$response = array(	
				"head" => array(
					"HTTP/1.0 200 OK",
					"Content-Type: text/html"
				), 
				"body" => array()
			);
			
			socket_getpeername($client, $address, $port);
		
			$response["body"][]="<html>";
			$response["body"][]="<head>";
			$response["body"][]="<title>Multithread Sockets PHP ({$address}:{$port})</title>";
			$response["body"][]="</head>";
			$response["body"][]="<body>";
			$response["body"][]="<pre>";
			foreach($headers as $header)
				$response["body"][]="{$header}";
			$response["body"][]="</pre>";
			$response["body"][]="</body>";
			$response["body"][]="</html>";
			$response["body"] = implode("\r\n", $response["body"]);
			$response["head"][] = sprintf("Content-Length: %d", strlen($response["body"]));
			$response["head"] = implode("\r\n", $response["head"]);
				
			socket_write($client, $response["head"]);
			socket_write($client, "\r\n\r\n");
			socket_write($client, $response["body"]);
					
			socket_close($client);
		}
	}
}
/* ladies and gentlemen, the world first multi-threaded socket server in PHP :) */
$server = socket_create_listen(10000);
while(($client = socket_accept($server))){
	new Client($client);
	/* we will serve a few clients and quit, to show that memory is freed and there are no errors on shutdown (hopefully) */
	if (++$count>100)
		break;
	/* in the real world, do something here to ensure clients not running are destroyed */
	/* the nature of a socket server is an endless loop, 
		if you do not do something to explicitly destroy clients you create this may leak */
}
/* that is all */
?>
