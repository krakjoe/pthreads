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
Fatal error: Uncaught Error: Trying to clone an uncloneable object of class Threaded in %s:3
Stack trace:
#0 {main}
  thrown in %s on line 3


