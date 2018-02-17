--TEST--
Socket::connect() - AF_UNIX - test with empty parameters
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $rand = rand(1,999);
    $socket = new \Socket(\Socket::AF_UNIX,\Socket::SOCK_DGRAM, 0);

    if (!$socket->setBlocking(false)) {
        die('Unable to set nonblocking mode for socket');
    }
    $address = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($address))
        die('Temporary file socket already exists.');

    if (!$socket->bind($address)) {
        die("Unable to bind to $address");
    }
    $socket->connect();
    $socket->connect($address);
    $socket->connect($address, 31330+$rand);

    $socket->close();
?>
--EXPECTF--

Warning: Socket::connect() expects at least 1 parameter, 0 given in %s on line %i
