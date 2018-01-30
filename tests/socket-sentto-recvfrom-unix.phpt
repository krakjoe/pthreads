--TEST--
Test if Socket::recvfrom() receives data sent by Socket::sendto() through a Unix domain socket
--CREDITS--
Copied from php/php-src and adjusted, originally created by 
Falko Menge <mail at falko-menge dot de>
PHP Testfest Berlin 2009-05-09
--SKIPIF--
<?php
if (substr(PHP_OS, 0, 3) == 'WIN') {
	die('skip.. Not valid for Windows');
}
--FILE--
<?php
    try {
        $socket = new Socket(AF_UNIX, SOCK_DGRAM, SOL_UDP);
    }catch(RuntimeException $exception) {
        var_dump($exception->getMessage());
    }
    $socket = new Socket(AF_UNIX, SOCK_DGRAM, 0);
    if (!$socket) {
        die('Unable to create AF_UNIX socket');
    }
    if (!$socket->setBlocking(false)) {
        die('Unable to set nonblocking mode for socket');
    }

    try {
        $socket->recvfrom($buf, 12, 0, $from, $port);
    } catch(RuntimeException $exception) {
        var_dump($exception->getMessage());
    }
    $address = sprintf("/tmp/%s.sock", uniqid());
    if (!$socket->bind($address)) {
        die("Unable to bind to $address");
    }

    $msg = "Ping!";
    $len = strlen($msg);
    $bytes_sent = $socket->sendto($msg, $len, 0); // cause warning
    $bytes_sent = $socket->sendto($msg, $len, 0, $address);
    if ($bytes_sent == -1) {
        @unlink($address);
        die('An error occurred while sending to the socket');
    } else if ($bytes_sent != $len) {
        @unlink($address);
        die($bytes_sent . ' bytes have been sent instead of the ' . $len . ' bytes expected');
    }

    $from = "";
    var_dump($socket->recvfrom($buf, 0, 0, $from)); // expect false
    $bytes_received = $socket->recvfrom($buf, 12, 0, $from);
    if ($bytes_received == -1) {
        @unlink($address);
        die('An error occurred while receiving from the socket');
    } else if ($bytes_received != $len) {
        @unlink($address);
        die($bytes_received . ' bytes have been received instead of the ' . $len . ' bytes expected');
    }
    echo "Received $buf";

    $socket->close();
    @unlink($address);
?>
--EXPECTF--
string(%d) "Error (%d): Protocol not supported"
string(%d) "Error (%d): Resource temporarily unavailable"

Warning: Socket::sendto() expects at least 4 parameters, 3 given in %s on line %d
bool(false)
Received Ping!
