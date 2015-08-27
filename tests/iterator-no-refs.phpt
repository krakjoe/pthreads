--TEST--
Test disallowing iteration by reference
--DESCRIPTION--
We cannot supply a reference to properties, as modifying them would not have the desired result
To avoid surprises, we throw an exception ...
--FILE--
<?php
foreach (new Threaded() as $k => &$v) {}
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: iteration by reference is not allowed on Threaded objects in %s:2
Stack trace:
#0 {main}
  thrown in %s on line 2

