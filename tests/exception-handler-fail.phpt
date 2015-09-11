--TEST--
Test exception handler inheritance (fail)
--DESCRIPTION--
Exception handlers cannot be objects that are not Threaded, this test verifies that such
handlers cause threads to fail silently rather than fault.
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
--EXPECTF--
Fatal error: Uncaught Exception in %s:13
Stack trace:
#0 %s(18): ExceptionThread->traceable()
#1 [internal function]: ExceptionThread->run()
#2 {main}
  thrown in %s on line 13


