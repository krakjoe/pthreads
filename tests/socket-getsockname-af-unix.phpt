--TEST--
Basic test of Socket::getSockName() with AF_UNIX sockets
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    $address = sprintf("/tmp/%s.sock", uniqid());

    if (file_exists($address))
        die('Temporary file socket already exists.');

    $unixSocket = new \Socket(\Socket::AF_UNIX,\Socket::SOCK_STREAM, 0);

    if (!$unixSocket->bind($address)) {
        die("Unable to bind to $address");
    }
    var_dump($unixSocket->getSockName());

    $unixSocket->close();
?>
--EXPECTF--
array(1) {
  ["host"]=>
  string(23) "/tmp/%s.sock"
}