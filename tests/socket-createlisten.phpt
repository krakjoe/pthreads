--TEST--
Basic test of Socket::createListen()
--FILE--
<?php
    $port = 31330 + rand(1,999);

    try {
        \Socket::createListen();
    } catch(Exception $exception) { var_dump($exception); }

    $socket = \Socket::createListen($port);
    $sockName = $socket->getSockName();

    $client = new \Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $client->connect($sockName['host'], $sockName['port']);

    var_dump($client->getPeerName(), $client->getPeerName(false));

    $client->close();
    $socket->close();
?>
--EXPECTF--

Warning: Socket::createListen() expects at least 1 parameter, 0 given in %s on line %i
array(2) {
  ["host"]=>
  string(9) "127.0.0.1"
  ["port"]=>
  int(%i)
}
array(1) {
  ["host"]=>
  string(9) "127.0.0.1"
}