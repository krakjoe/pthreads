--TEST--
Test lexical vars
--DESCRIPTION--
In pthreads v2, it was difficult to support lexical or function-static
scope vars, for various reasons.

In pthreads v3, it is now possible to have some support, the following things will work
	Threaded objects
	Closures
	Arrays
	Scalars

The following things will not work:
	Non-threaded objects
	Resources

Unsupported types should not raise notices, they will simply be undefined in the function scope.
--FILE--
<?php
class TestThread extends \Thread
{
    private $function;
    public function __construct(callable $function)
    {
        $this->function = $function;
    }
    public function run()
    {
        $function = $this->function;
        $function();
    }
}

$scalar = 1; # will work
$string = "thing"; # will work
$res = fopen("php://stdin", "r"); # won't work
$threaded = new Threaded(); # will work
$closure = function() { # will work
	echo "hi";
};
$array = [1, 2, 3, 4, 5]; # will work
$std = new stdClass; # won't work

$thread = new TestThread(function () use ($scalar, $string, $res, $threaded, $closure, $array, $std) {
    var_dump($scalar, $string, $res, $threaded, $closure, $array, $std);
});
$thread->start();
--EXPECTF--
int(1)
string(5) "thing"
NULL
object(Threaded)#2 (0) {
}
object(Closure)#3 (0) {
}
array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(4)
  [4]=>
  int(5)
}
NULL

