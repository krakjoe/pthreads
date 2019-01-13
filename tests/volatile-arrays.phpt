--TEST--
Test Volatile Arrays
--DESCRIPTION--
Arrays were difficult to use in pthreads, their behaviour was strange and inconsistent with Zend arrays.

pthreads v3 will coerce arrays to Volatile objects when they are set as members of Threaded objects.

Threaded objects have been made consistent with PHP arrays, so there should be no noticable difference between an array
and a volatile object.
--FILE--
<?php
$threaded = new Threaded();

$threaded->test = [
	"greeting" => "Hello World", 
	"child" => [
		"of" => "mine",
		"grandchild" => [
			"of" => "parents"
		]
	]
];

/*
 This looks strange, but needs to be consistent with zend, so we'll test here ... 
*/
$threaded["0"] = [];
$threaded[1] = [];
$threaded[2] = 'foo';
$threaded['3'] = 'bar';

var_dump($threaded);
var_dump(isset($threaded["0"]));
var_dump(isset($threaded[0]));

unset($threaded["2"], $threaded[3]);

/*
 This kind of thing would simply fail before, creating really unexpected results
*/
$threaded->test["child"]["of"] = "yours";
$threaded->test["child"]["grandchild"]["of"] = "devil";

var_dump($threaded);
?>
--EXPECTF--
object(Threaded)#%d (%d) {
  ["test"]=>
  object(Volatile)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(Volatile)#%d (%d) {
      ["of"]=>
      string(4) "mine"
      ["grandchild"]=>
      object(Volatile)#%d (%d) {
        ["of"]=>
        string(7) "parents"
      }
    }
  }
  [0]=>
  object(Volatile)#%d (%d) {
  }
  [1]=>
  object(Volatile)#%d (%d) {
  }
  [2]=>
  string(3) "foo"
  [3]=>
  string(3) "bar"
}
bool(true)
bool(true)
object(Threaded)#%d (%d) {
  ["test"]=>
  object(Volatile)#%d (%d) {
    ["greeting"]=>
    string(11) "Hello World"
    ["child"]=>
    object(Volatile)#%d (%d) {
      ["of"]=>
      string(5) "yours"
      ["grandchild"]=>
      object(Volatile)#%d (%d) {
        ["of"]=>
        string(5) "devil"
      }
    }
  }
  [0]=>
  object(Volatile)#%d (%d) {
  }
  [1]=>
  object(Volatile)#%d (%d) {
  }
}

