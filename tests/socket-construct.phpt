--TEST--
Test creating and closing sockets
--DESCRIPTION--
Test that creating and closing sockets works as expected on all platforms (gh issue #798)
--FILE--
<?php
$socket = new \Socket(\Socket::AF_INET, \Socket::SOCK_DGRAM, \Socket::SOL_UDP);
echo "created\n";
$socket->close();
echo "closed\n";
?>
--EXPECTF--
created
closed
