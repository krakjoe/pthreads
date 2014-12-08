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
		for($i = 0; $i < 20; ++$i)
			$this->update( mt_rand(0, 100) );
		
		/* "magic number" */
		$this->update( -1 );
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

while(1) {
	if(!count($thread))
		/* Wait for signal if there is no more numbers to shift */
		Cond::wait($cond, $mutex);
	
	/* Get my lucky number */
	$number = $thread->shift();
	
	/* Break on "magic number" */
	if($number === -1)
		break;
	
	echo "Your lucky number is {$number}\n";
}
echo "I'm done here...\n";

/* Remove Cond */
Cond::destroy($cond);
/* Unlock and remove Mutex */
Mutex::unlock($mutex, true);

