<?php

/*
	This is a simple example for Cond in pthreads, as Joe said "Normal users shouldn't really have to use this functionality", but there it is!
	
	When use?
	Cond can be helpful when one thread is waiting for some data from another. You can waste CPU cycles with normal while(true) or make while wait for signals.
*/


/* Create Mutex and Cond */
$mutex = Mutex::Create();
$cond = Cond::Create();

class RandomNumbersGenerator extends Thread {
	public function run() {
		/* Generate 20 random numbers */
		for($i = 0; $i < 20; ++$i){
			/* Some expensive equations (0.1-2s) */
			usleep(mt_rand(100,2000)*1000);
			
			$this->update( mt_rand(0, 100) );
		}
	}
	public function update($number){
		/* Require PTHREADS_ALLOW_GLOBALS flag */
		global $mutex, $cond;
		
		$this[] = $number;
		/* Again lock Mutex, this time from thread */
		Mutex::lock($mutex);
		/* Send signal */
		Cond::signal($cond);
		/* Don't forget unlock Mutex */
		Mutex::unlock($mutex);
	}
}

/* Create Thread */
$thread = new RandomNumbersGenerator();
$thread->start(PTHREADS_INHERIT_ALL | PTHREADS_ALLOW_GLOBALS);

/* You must lock Mutex before use Cond::wait() - Mutex protect Cond variable itself */
Mutex::lock($mutex);

$iterations = 0;

while(1) {
	/* Cond can unblock prematurely */
	while(!count($thread)){
		++$iterations;
		
		/* Don't wait for not working thread */
		if(!$thread->isRunning())
			/* Break both loops */
			break 2;
		
		/*  When there is no more numbers to shift:
			wait for signal or 1s whichever comes first */
		try {
			Cond::wait($cond, $mutex, 1 * 1000000);
		} catch(RuntimeException $e) {
			/* Timeout */
			continue;
		}
	}
	
	/* Get my lucky number */
	$number = $thread->shift();
	
	echo "Your lucky number is {$number}\n";
}
echo "Total iterations: {$iterations}\nI'm done here...\n";

$thread->join();

/* Remove Cond */
Cond::destroy($cond);
/* Unlock and remove Mutex */
Mutex::unlock($mutex, true);

