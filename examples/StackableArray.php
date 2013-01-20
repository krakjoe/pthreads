<?php
/**
* NOTES:
*	You can and should use a Stackable as a base object for all arrays you wish to pass to other threads
*	Members of arrays that are themselves an array should also be a derivative of Stackable ( or another threaded object ), though it's not strictly necessary.
*	Annonymous members are unsupported ([]=""), keeping a counter in the array object solves the problem and is completely safe
*	Even if 100(0) threads are accessing the array, nothing bad will happen
*	Code that attempts to write an anonymous member will generate an E_WARNING, no member will be set
*	Can anyone think of anything else ?
*/

/* you might want to set this to 100 before running if you're running on older ( dual core ) hardware */
/* never, _ever_, _ever_, _ever_ create 1000 threads in a PHP application, if you think there's a need to create that many threads:
	you are doing it wrong */
/* the number is high to show that manipulating a stackable as an array in this way is completely safe and reliable */
$hammers = 500;
/** $hammers threads are about to edit this array */
/*
* NOTE
*	pthreads overrides the dimension read/writers with our own handlers
*	Our internal handlers are not setup to execute ArrayAccess interfaces
*	If we did execute ArrayAccess methods, you would pay a high price:
*		referencing an array in this way would keep switching in and out of the VM to call your handlers,
*		because these arrays are meant to provide efficiency using the ArrayAccess interface is unsuitable.
*/
class StackableArray extends Stackable {
	/*
	* Always think about caching these types of objects, don't waste the run method or your workers
	*/
    public function run() {}
}
/* a thread for editing */
class T extends Thread {
	public function __construct($test){
		$this->test = $test;
		$this->start();
	}
	public function run(){
		/*
		* NOTE
		*	If your editing of an array is this simple
		*		then don't protect these instructions
		*		you will incurr additional unecessary locking
		*/
		$this->test[]=rand(0, 10);
	}
}
/* create the array here for passing */
$s = new StackableArray();
/* set a pointless value */
$s[]="h";
/* show it was set */
print_r($s);
$ts = array();
/* hammer the crap out of the array */
while(@$i++ < $hammers){
	$ts[]=new T($s);
}
/* we want all threads to complete */
foreach($ts as $t)
	$t->join();

$s[510] = "test";
/* show it was all set without corruption */
print_r($s);
?>
