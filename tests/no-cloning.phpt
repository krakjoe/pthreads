--TEST--
Test no cloning
--DESCRIPTION--
This test that clone will not operate (or have strange side effects)
--FILE--
<?php
$threaded = new Threaded();
$clone = clone $threaded;
?>
--EXPECTF--
Fatal error: Uncaught RuntimeException: Threaded objects cannot be cloned in %s:3
Stack trace:
#0 {main}
  thrown in %s on line 3


