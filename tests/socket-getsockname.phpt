--TEST--
Basic test of Socket::getSockName() with AF_INET and AF_UNIX sockets
--FILE--
<?php
    $address = '127.0.0.1';
    $port = 31330 + rand(1,999);

    $inetSocket = new \Socket(\Socket::AF_INET,\Socket::SOCK_STREAM, \Socket::SOL_TCP);

    if (!$inetSocket->bind($address, $port)) {
        die("Unable to bind to $address");
    }
    var_dump($inetSocket->getSockName(), $inetSocket->getSockName(false));

    $inetSocket->close();

    $address = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($address))
        die('Temporary file socket already exists.');

    $unixSocket = new \Socket(\Socket::AF_UNIX,\Socket::SOCK_STREAM, 0);

    if (!$unixSocket->bind($address)) {
        die("Unable to bind to $address");
    }
    var_dump($unixSocket->getSockName());

    $unixSocket->close();
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
array(1) {
  ["host"]=>
  string(23) "/tmp/%s.sock"
}