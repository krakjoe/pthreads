--TEST--
Test of Socket::shutdown() - testing params and separate shutdown of read and write channel
--SKIPIF--
<?php
    if(getenv("SKIP_ONLINE_TESTS")) die("skip online test");
    if(!method_exists(\Socket::class, 'shutdown')) {
        die('skip.. Socket::shutdown() doesn\'t exist');
    }
--FILE--
<?php
    $socket = new \Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);

    try{
        $socket->shutdown(4); // invalid
    }catch(\InvalidArgumentException $e){
        var_dump($e->getMessage());
    }
    
    try {
        var_dump($socket->shutdown(\Socket::SHUTDOWN_READ)); // close reading
    } catch(Exception $exception) { var_dump('not connected'); }

    $socket->connect('www.php.net', 80);

    var_dump($socket->shutdown(\Socket::SHUTDOWN_READ)); // close reading
    var_dump($socket->shutdown(\Socket::SHUTDOWN_WRITE)); // close writing

    $socket->close();
?>
--EXPECT--
string(21) "Invalid shutdown type"
string(13) "not connected"
bool(true)
bool(true)
