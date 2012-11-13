--TEST--
Test INI inheritance (bug 20)
--DESCRIPTION--
This test will ensure that INI directives are inherited and or created upon initialization of new threads
--FILE--
<?php
ini_set("include_path", ":/var/lib/other");
class Test extends Thread {
	public function run(){
		printf("%s: %s\n", __METHOD__, ini_get("include_path"));
	}
}
$test = new Test();
$test->start();
?>
--EXPECT--
Test::run: :/var/lib/other
