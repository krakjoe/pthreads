--TEST--
socket_bind - basic test
--CREDITS--
Copied from php/php-src and adjusted, originally created by 
Florian Anderiasch
fa@php.net
--SKIPIF--
<?php
    if (getenv("SKIP_ONLINE_TESTS")) {
        die("skip test requiring internet connection");
    }
?>
--FILE--
<?php
	$rand = rand(1,999);
    $socket = new Socket(\Socket::AF_INET, \Socket::SOCK_STREAM, \Socket::SOL_TCP);
    $bind   = $socket->bind('0.0.0.0', 31330+$rand);
    var_dump($bind);

    // Connect to destination address
    $s_conn  = $socket->connect('www.php.net', 80);
    var_dump($s_conn);

    // Write
    $request = 'GET / HTTP/1.1' . "\r\n";
    $s_write = $socket->write($request);
    var_dump($s_write);

    // Close
    $s_close = $socket->close();
    var_dump($s_close);
?>

--EXPECTF--
bool(true)
bool(true)
int(16)
NULL
