--TEST--
IPv4 Loopback test
--FILE--
<?php
	/* Setup socket server */
    $server = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, getprotobyname('tcp'));
    if (!$server) {
        die('Unable to create AF_INET socket [server]');
    }
    $bound = false;

    for($port = 31337; $port < 31357; ++$port) {
        try {
            if ($server->bind('127.0.0.1', $port)) {
                $bound = true;
                break;
            }
        } catch (RuntimeException $re) {}
    }
	    
    if (!$bound) {
        die("Unable to bind to 127.0.0.1");
    }
    if (!$server->listen(2)) {
        die('Unable to listen on socket');
    }

    /* Connect to it */
    $client = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, getprotobyname('tcp'));
    if (!$client) {
        die('Unable to create AF_INET socket [client]');
    }
    if (!$client->connect('127.0.0.1', $port)) {
        die('Unable to connect to server socket');
    }

    /* Accept that connection */
    /** @var Socket $socket */
    $socket = $server->accept(\Socket::class);
    if (!$socket) {
        die('Unable to accept connection');
    }

    $client->write("ABCdef123\n");

    $data = $socket->read(10, 0, \Socket::BINARY_READ);
    var_dump($data);

    $client->close();
    $socket->close();
    $server->close();
?>
--EXPECT--
string(10) "ABCdef123
"
