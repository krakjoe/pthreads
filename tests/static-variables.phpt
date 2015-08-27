--TEST--
Test static scoped variables (bug 391)
--DESCRIPTION--
This test verifies that statically scoped variables in functions made outside of threads are available inside threads without error excluding arrays, objects, and resources
--FILE--
<?php
function funcWithStaticVar($set = null)
{
    static $var;

    if ($set !== null) {
        $var = $set;
    }

    return $var;
}

class TestThread extends Thread
{
    public function run()
    {
        var_dump(funcWithStaticVar());
    }
}

// Test with an int, which should be copied.
funcWithStaticVar(42);
$thread = new TestThread();
$thread->start();
$thread->join();

// Test with a string, which should be copied.
funcWithStaticVar("pthreads rocks!");
$thread = new TestThread();
$thread->start();
$thread->join();

// Test with an array, which should be nullified.
funcWithStaticVar([
    "pthreads", "rocks", 4, "real!"
]);
$thread = new TestThread();
$thread->start();
$thread->join();

// Test with an object, which should be nullified.
funcWithStaticVar(new stdClass);
$thread = new TestThread();
$thread->start();
$thread->join();

// Test with a resource, which should be nullified.
$fp = fopen(__FILE__, 'r');
funcWithStaticVar($fp);
$thread = new TestThread();
$thread->start();
$thread->join();
fclose($fp);
?>
--EXPECT--
int(42)
string(15) "pthreads rocks!"
NULL
NULL
NULL
