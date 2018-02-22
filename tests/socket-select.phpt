--TEST--
Test parameter handling in Socket::select().
--FILE--
<?php
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $socket->listen();

    try {
        Socket::select();
    } catch(Throwable $throwable) {
        var_dump($throwable->getMessage());
    }
    $read   = [$socket];
    $write  = null;
    $except = null;

    // Valid arguments, return immediately
    var_dump(Socket::select($read, $write, $except, 0));

    $read = [$socket];

    // Valid sec argument, wait 1 second
    var_dump(Socket::select($read, $write, $except, 1, 0, $errno));
    var_dump($errno);

    $read = [$socket];

    // Invalid sec argument, return immediately
    var_dump(Socket::select($read, $write, $except, -1, 0, $errno));
    var_dump($errno);

    $socket->close();
--EXPECTF--

Warning: Socket::select() expects at least 4 parameters, 0 given in %s on line %d
int(0)
int(0)
int(0)
bool(false)
int(22)