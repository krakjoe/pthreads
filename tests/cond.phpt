--TEST--
Test condition variable operations
--DESCRIPTION--
This test will ensure condition variable functionality
--FILE--
<?php
class CondTest extends Thread{
	public function __construct($cond, $ready){
		$this->cond = $cond;
		$this->ready = $ready;
	}
	
	public function run(){
		
		if($this->cond){
			var_dump(Mutex::lock($this->ready));
			var_dump(Cond::broadcast($this->cond)); 
			var_dump(Mutex::unlock($this->ready));
			return true;
		}
	}
}
$cond = Cond::create();
$ready = Mutex::create(true);
$thread = new CondTest($cond, $ready);
if($thread->start()){
	$mutex = Mutex::create(true);
	var_dump(Mutex::unlock($ready) && Cond::wait($cond, $mutex));
	var_dump($thread->join());
	var_dump(Mutex::unlock($mutex));
	var_dump(Mutex::destroy($mutex));
	var_dump(Mutex::destroy($ready));
	var_dump(Cond::destroy($cond));
}
?>
--EXPECT--
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)