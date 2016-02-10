--TEST--
Test Explicit Cast
--DESCRIPTION--
Normally, arrays are cast to Volatile objects when set as a member of a Threaded object.
Casting the array as you are setting will override this behaviour, this is useful when passing an array out of a thread.
For an example of that, see examples/ClosureFuture.php
--FILE--
<?php
$threaded = new Threaded();

$threaded->member = (array) [
	"hello",
	"world"
];

var_dump($threaded);

$threaded->member = new Threaded;
$threaded->member[0] = (array) [
	"hello",
	"world"
];

var_dump($threaded);
?>
--EXPECTF--
object(Threaded)#%d (%d) {
  ["member"]=>
  array(2) {
    [0]=>
    string(5) "hello"
    [1]=>
    string(5) "world"
  }
}
object(Threaded)#%d (%d) {
  ["member"]=>
  object(Threaded)#%d (%d) {
    [0]=>
    array(2) {
      [0]=>
      string(5) "hello"
      [1]=>
      string(5) "world"
    }
  }
}
