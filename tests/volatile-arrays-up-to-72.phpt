--TEST--
Test Volatile Arrays
--DESCRIPTION--
Arrays were difficult to use in pthreads, their behaviour was strange and inconsistent with Zend arrays.

pthreads v3 will coerce arrays to Volatile objects when they are set as members of Threaded objects.

Threaded objects have been made consistent with PHP arrays, so there should be no noticable difference between an array
and a volatile object.
--SKIPIF--
<?php if(version_compare(PHP_VERSION, '7.3.0-dev', '>=')) print "skip"; ?>
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

var_dump($threaded);

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
}
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

