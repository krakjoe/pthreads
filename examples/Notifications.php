<?php
/*
* Note: in 5.4 series the output is handled differently from multiple threads on the command line, sometimes giving the idea that this sort of thing is executing the opposite to how it should...
* It isn't, but there's not much to be done about the new output handling in 5.4 ... we'll just have to live with it ...
*/
class ExampleThread extends Thread {
        public function run(){
                        $stdout = fopen("php://stdout", "w");
                        printf("I'm in the thread and you're waiting for me ...\n");
fflush($stdout);
                        printf("Allowing process to continue ...\n");
fflush($stdout);
                        printf("Thread Notifying Process 1: %d\n", $this->notify());
fflush($stdout);
                        printf("Process Waiting ...\n");
fflush($stdout);
                        printf("Thread Notifying Process 2: %d\n", $this->notify());
fflush($stdout);
					$this->wait();
					
					printf("Thread exiting ...");
        }
}

$t = new ExampleThread();

if ($t->start(true)) {
        printf("Process Working  ... then ...\n");
		
		echo "WHAT ...";
		
        printf("Process Waited: %d\n", $t->wait());
        printf("Now doing some work while thread is waiting ...\n");
		
		usleep(1000000);
		printf("And notify thread: %d\n", $t->notify());
}
?>
