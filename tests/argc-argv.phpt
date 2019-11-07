--TEST--
Test argc/argv inheritance
--DESCRIPTION--
Verifies that argc and argv are present and correct on threads.
--INI--
register_argc_argv=1
--ARGS--
--test=1
--FILE--
<?php
$t = new class extends Thread{
	public function run() : void{
		global $argc, $argv;
		var_dump($argc, $argv);
	}
};
$t->start();
?>
--EXPECTF--
int(2)
array(2) {
  [0]=>
  string(%d) "%s"
  [1]=>
  string(8) "--test=1"
}
