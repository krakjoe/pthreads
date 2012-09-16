<?php
/* A simple example of syncrhonization between two threads */
class Syncrhonizing extends Thread {
	/* The constructor takes just the mutex to keep things simple */
	public function __construct($lock){
		$this->lock = 	$lock;
	}

	/* 
		The run method shall first attempt to acquire the lock.
		Because the mutex passed is owned by the parent it cannot continue until
			the parent unlocks the mutex
		It does it's work and holds onto the lock until it's ready to return
			(pending a flush, which in the real world isn't required)
		So the parent doesn't need to unlock the mutex before destruction
	*/
	public function run(){
		/* see, first lock mutex */
		printf("Running(%d) %f\n", Mutex::lock($this->lock), microtime(true));
		/* print some stuff to standard output */
		$stdout = fopen("php://stdout", "w");
		while(++$i<rand(200,400)) {
			echo ".";
			fflush($stdout);
		}
		echo "\n";
		fflush($stdout);
		/* and unlock mutex, making it ready for destruction */
		printf("Returning(%d) %f\n", Mutex::unlock($this->lock), microtime(true));
		fflush($stdout);
		/* you should close resources, or not, whatever; I'm not your mother ... */
		return null;
	}
}
/* create a mutex and acquire lock */
$lock = Mutex::create(true);
/* create new thread, passing the lock as parameter */
$thread = new Syncrhonizing($lock);
/* start executing the child ... */
$thread->start();
/* the child will block until you release the mutex */
printf("Allow(%d):  %f\n", Mutex::unlock($lock), microtime(true));
/* ... continue normal not thread related stuff ... */
/* and then: */
$thread->join();
/* 
	another option would be to lock the mutex and unlock it and let the thread destroy itgetThreadId without calling join,
	when this is stable that will definitely be an option but right now I suggest you join everything you create somewhere
*/
printf("Cleaup(%d):  %f\n", Mutex::destroy($lock), microtime(true));
?>