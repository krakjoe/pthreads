--TEST--
Test ArrayAccess compatibility
--DESCRIPTION--
This test verifies that ArrayAccess is executed for pthreads objects
--FILE--
<?php
class T extends Thread implements ArrayAccess {
	public function run() {
		$this["T"]="S";
	}
	
	public function offsetSet($offset, $data) {
		var_dump($m=__METHOD__);
		$this->$offset = $data;
	}

	public function offsetGet($offset) {
		var_dump($m=__METHOD__);
		return $this->$offset;
	}

	public function offsetExists($offset) {
		return isset($this->$offset);
	}	

	public function offsetUnset($offset) {
		unset($this->$offset);
	}
}

$t = new T();
$t->start();
$t->join();

foreach($t as $k => $v) {
	var_dump($v);
	var_dump($t[$k]);
	unset($t[$k]);
	var_dump($t[$k]);
}
?>
--EXPECT--
string(12) "T::offsetSet"
string(1) "S"
string(12) "T::offsetGet"
string(1) "S"
string(12) "T::offsetGet"
NULL

