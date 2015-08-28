--TEST--
Testing sane handling of resources and objects bug #39
--DESCRIPTION--
Test that resources and objects are not corrupted when written to thread storage
--FILE--
<?php
class Work extends Threaded {
	public function run(){
		
	}
}

class Test extends Thread {
	public function run(){
		$test = new Work();
		$this->test = $test;
		print_r($this->test);
		var_dump($this->test);
		$stream = tmpfile();
		var_dump($stream);
		$this->stream = $stream;
		stream_set_blocking($this->stream, 0);
		var_dump($this->stream);
	}
}

$test =new Test();
$test->start();
?>
--EXPECTF--
Work Object
(
)
object(Work)#2 (0) {
}
resource(%d) of type (stream)
resource(%d) of type (stream)


