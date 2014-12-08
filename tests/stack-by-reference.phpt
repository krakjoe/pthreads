--TEST--
Test stacking by reference ONLY
--DESCRIPTION--
Stacking without a reference is not allowed, so the API should enforce the requirement
--FILE--
<?php
class T extends Threaded { 
	public function run() {

	}
}

class W extends Worker {
	public function run() {}
}

$t = new W();
$t->start();
try {
    $t->stack(new T());
} catch(Exception $ex) {
    echo $ex->getMessage();
}
$t->shutdown();
?>
--EXPECT--
Worker::stack expects $work to be a reference

