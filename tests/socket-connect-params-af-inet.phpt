--TEST--
Socket::connect() - AF_INET - test with empty parameters
--FILE--
<?php
    $rand = rand(1,999);
    $socket = new \Socket(\Socket::AF_INET, \Socket::SOCK_DGRAM, 0);
    $socket->bind('0.0.0.0');
    // wrong parameter count
    $socket->connect();
    try {
        $socket->connect('0.0.0.0');
    } catch(Exception $exception) {
        var_dump($exception->getMessage());
    }
    $socket->connect('127.0.0.1', 31330+$rand);
    $socket->close();
?>
--EXPECTF--

Warning: Socket::connect() expects at least 1 parameter, 0 given in %s on line %i
string(43) "Socket of type AF_INET requires 2 arguments"
