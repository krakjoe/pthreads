--TEST--
Test iterating on Threaded
--DESCRIPTION--
Regression test for bugs introduced with Threaded iteration in PHP 7.3
--FILE--
<?php
$threaded = new Threaded();

var_dump($threaded->count());
foreach($threaded as $k => $prop){
	var_dump("should not happen");
}

for($i = 0; $i < 5; ++$i){
	$threaded[] = "value$i";
}

foreach($threaded as $i => $prop){
	var_dump($i, $prop);
}
?>
--EXPECTF--
int(0)
int(0)
string(6) "value0"
int(1)
string(6) "value1"
int(2)
string(6) "value2"
int(3)
string(6) "value3"
int(4)
string(6) "value4"

