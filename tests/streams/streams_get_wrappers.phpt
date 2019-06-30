--TEST--
array Streams::getWrappers ( void );
--FILE--
<?php
var_dump(Streams::getWrappers());
class Foo extends Threaded { }
Streams::registerWrapper("foo", "Foo");
var_dump(in_array("foo", Streams::getWrappers()));
?>
--EXPECT--
array(6) {
  [0]=>
  string(3) "php"
  [1]=>
  string(4) "file"
  [2]=>
  string(4) "glob"
  [3]=>
  string(4) "data"
  [4]=>
  string(4) "http"
  [5]=>
  string(3) "ftp"
}
bool(true)