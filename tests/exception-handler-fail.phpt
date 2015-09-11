--TEST--
Test exception handler inheritance (fail)
--DESCRIPTION--
Exception handlers cannot be objects that are not Threaded, this test verifies that such
handlers cause threads to fail gracefully rather than fault.
--FILE--
<?php
class ExceptionHandler
{
    public function handle(Exception $e)
    {
        var_dump($e);
    }
}

class ExceptionThread extends Thread
{
	public function traceable() {
		throw new Exception();
	}

    public function run()
    {
        $this->traceable();
    }
}

$handler = new ExceptionHandler();

set_exception_handler([$handler, "handle"]);

$t = new ExceptionThread();
$t->start();
$t->join();
?>
--EXPECT--
Fatal error: Uncaught RuntimeException: cannot setup exception handler to use non-threaded object, use Threaded object or ::class in [no active file]:0
Stack trace:
#0 {main}
  thrown in [no active file] on line 0

