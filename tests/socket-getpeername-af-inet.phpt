--TEST--
Basic test of Socket::getPeerName() with AF_INET sockets
--FILE--
<?php
    $address = '127.0.0.1';
    $port = 31330 + rand(1,999);

    $socket = new \Socket(\Socket::AF_INET,\Socket::SOCK_STREAM, \Socket::SOL_TCP);

    if (!$socket->bind($address, $port)) {
        die("Unable to bind to $address");
    }
    $socket->listen(1);

    $client = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $client->connect($address, $port);

    var_dump($client->getPeerName(), $client->getPeerName(false));

    $client->close();
?>
--EXPECTF--
array(2) {
  ["host"]=>
  string(9) "127.0.0.1"
  ["port"]=>
  int(%d)
}
array(1) {
  ["host"]=>
  string(9) "127.0.0.1"
}