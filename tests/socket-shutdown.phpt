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

    var_dump($socket->shutdown(4)); // invalid - false

    $socket->bind('0.0.0.0');
    $socket->listen(1);

    var_dump($socket->shutdown(\Socket::SHUTDOWN_READ));

    try {
        var_dump($socket->shutdown(\Socket::SHUTDOWN_WRITE)); // close writing
    } catch(Exception $exception) { var_dump('not connected'); }

    $socket->close();
?>
--EXPECTF--
bool(false)
bool(true)
string(13) "not connected"
