--TEST--
Test of Socket::getOption() with and without parameters
--FILE--
<?php
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $socket->getOption();

    try {
        $socket->getOption("hello", "world");
    } catch(Throwable $throwable) {
        var_dump($throwable->getMessage());
    }
    var_dump($socket->getOption(\Socket::SOL_SOCKET, \Socket::SO_REUSEADDR));
?>
--EXPECTF--
Warning: Socket::getOption() expects exactly 2 parameters, 0 given in %s on line %i
string(%i) "Argument 1 passed to Socket::getOption() must be of the type %s, string given"
int(%i)