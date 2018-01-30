--TEST--
Test error cases when creating a socket
--CREDITS--
Copied from php/php-src and adjusted, originally created by 
Russell Flynn <russ@redpill-linpro.com>
#PHPTestFest2009 Norway 2009-06-09 \o/
--INI--
error_reporting=E_ALL
display_errors=1
--FILE--
<?php
  // Test with no arguments
  $server = new Socket();
  
  // Test with less arguments than required
  $server = new Socket(\Socket::SOCK_STREAM, getprotobyname('tcp'));
  
  // Test with non integer parameters
  $server = new Socket(array(), 1, 1);
  
?>
--EXPECTF--
Warning: Socket::__construct() expects exactly 3 parameters, 0 given in %s on line %d

Warning: Socket::__construct() expects exactly 3 parameters, 2 given in %s on line %d

Fatal error: Uncaught TypeError: Argument 1 passed to Socket::__construct() must be of the type integer, array given in %ssocket-connect-error.php:%d
Stack trace:
#0 %ssocket-connect-error.php(%d): Socket->__construct(Array, 1, 1)
#1 {main}
  thrown in %ssocket-connect-error.php on line %d