<?php

/*
Threaded objects (which subsequently includes Volatile objects) are tied to the
context in which they are created. They can be used to fetch data from a thread,
but must be created in the outer most thread in which they are used.
*/

// create Threaded object in the main thread
$store = new Threaded();

$thread = new class($store) extends Thread {
	public $store;

	public function __construct(Threaded $store)
	{
		$this->store = $store;
	}

	public function run()
	{
		/*
		The following array cast is necessary to prevent implicit coercion to a
		Volatile object. Without it, accessing $store in the main thread after
		this thread has been destroyed would lead to RuntimeException of:
		"pthreads detected an attempt to connect to an object which has already
		been destroyed in %s:%d"

		See this StackOverflow post for additional information:
		https://stackoverflow.com/a/44852650/4530326
		*/
		$this->store[] = (array)[1,2,3];
	}
};

$thread->start() && $thread->join();

print_r($store);
