--TEST--
Test exception handler inheritance (non threaded objects)
--DESCRIPTION--
Non-threaded objects caused pthreads up to v3.0.1 to fault when used as excepion handlers.

Before we start a new thread, we apply a function to the exception handler stack that rebuilds their
property tables.
--FILE--
<?php
class ExceptionHandler
{
    public function handle(Throwable $e)
    {
        echo $e->getMessage();
    }
}

class ExceptionThread extends Thread
{
	public function traceable() {
		throw new Exception("OK");
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
OK


