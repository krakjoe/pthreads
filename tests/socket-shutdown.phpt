--TEST--
Test of Socket::shutdown() - testing params and separate shutdown of read and write channel
--DESCRIPTION--
Test that creating and closing sockets works as expected on all platforms (gh issue #798)
--SKIPIF--
<?php
    if(!method_exists(\Socket::class, 'shutdown')) {
        die('skip.. Socket::shutdown() doesn\'t exist');
    }
--FILE--
<?php
    $socket = new \Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, 0);
    $socket->shutdown();

    var_dump($socket->shutdown(4)); // invalid - false

    $socket->bind('0.0.0.0');
    $socket->listen(1);

    var_dump($socket->shutdown(0)); // close reading

    try {
        var_dump($socket->shutdown(1)); // close writing
    } catch(Exception $exception) { var_dump('not connected'); }

    $socket->close();
?>
--EXPECTF--
Warning: Socket::shutdown() expects exactly 1 parameter, 0 given in %s on line %d
bool(false)
bool(true)
string(13) "not connected"
