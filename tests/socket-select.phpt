--TEST--
Test parameter handling in Socket::select()
--FILE--
<?php
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $socket->listen();

    \Socket::select(); //emits warning

    $read = null;
    $write = null;
    $except = null;

    try{
        \Socket::select($read, $write, $except, 0);
    }catch(\InvalidArgumentException $e){
        var_dump($e->getMessage());
    }

    $read   = [$socket];
    $write  = null;
    $except = null;

    // Valid arguments, return immediately
    var_dump(\Socket::select($read, $write, $except, 0));

    $read = [$socket];

    // Valid sec argument, wait 1 second
    var_dump(\Socket::select($read, $write, $except, 1, 0, $errno));
    var_dump($errno);

    $read = [$socket];

    try{
        //invalid sec argument, throw
        \Socket::select($read, $write, $except, -1, 0, $errno);
    }catch(\InvalidArgumentException $e){
        var_dump($e->getMessage());
    }

    try{
        //invalid usec argument, throw
        \Socket::select($read, $write, $except, 1, -1, $errno);
    }catch(\InvalidArgumentException $e){
        var_dump($e->getMessage());
    }

    $socket->close();
--EXPECTF--

Warning: Socket::select() expects at least 4 parameters, 0 given in %s on line %d
string(22) "No valid sockets given"
int(0)
int(0)
int(0)
string(22) "sec cannot be negative"
string(23) "usec cannot be negative"