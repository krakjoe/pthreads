--TEST--
Basic test of Socket::getPeerName() with AF_UNIX sockets
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $address = sprintf("/tmp/%s.sock", uniqid());

    $socket = new \Socket(\Socket::AF_UNIX,\Socket::SOCK_STREAM, 0);

    if (!$socket->bind($address)) {
        die("Unable to bind to $address");
    }
    $socket->listen(1);

    $client = new Socket(\Socket::AF_UNIX, \Socket::SOCK_STREAM, 0);
    $client->connect($address);

    var_dump($client->getPeerName());

    $client->close();
?>
--EXPECTF--
array(1) {
  ["host"]=>
  string(23) "/tmp/%s.sock"
}