--TEST--
Testing merging members (long keys)
--DESCRIPTION--
This tests that merging ranges works as expected (long keys)
--FILE--
<?php
class Storage extends Threaded {
    public function run() {}
}

$storage = new Storage();
$storage->merge(range("0", "3"));
var_dump(count($storage));
?>
--EXPECTF--
int(4)
