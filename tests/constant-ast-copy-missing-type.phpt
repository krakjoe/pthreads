--TEST--
Test constant AST copy bug #564
--DESCRIPTION--
AST in op array literals caused corrupted heaps because it wasn't copied properly. 
--FILE--
<?php
namespace any\name\space {
	class Test extends \Thread {
		const KEY = '';

		public function run() {
			static $arrConst = [self::KEY => true];

			var_dump($arrConst);
		}
	}
}

namespace {
	use any\name\space\Test;

	$objTest = new Test();
	$objTest->start();
	$objTest->join();
}
?>
--EXPECTF--
array(1) {
  [""]=>
  bool(true)
}
