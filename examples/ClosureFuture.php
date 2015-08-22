<?php
/**
* NOTE: THIS IS A 5.6 EXAMPLE ONLY
*
* This is just scrap code, but rather cool scrap code!
*
* Notes:
*   Lexcial vars and threads do not mix, do not try, rather use this kind of abstraction or Threaded::from
*       to create thread safe objects.
*
* That's all I've got to say about that ...
**/
class Future extends Thread {

    private function __construct(Closure $closure, array $args = []) {
        $this->closure = $closure;
        $this->args    = $args;	
		$this->owner   = Thread::getCurrentThreadId();
    }

    public function run() {
        $closure = $this->closure;
        $this->synchronized(function() use($closure) {
            $this->result = 
                $closure(...$this->args);
            $this->notify();
        });
    }

    public function getResult() {
        return $this->synchronized(function(){
            while (!$this->result)
                $this->wait();
            return $this->result;
        });
    }
    
    public static function of(Closure $closure, array $args = []) {
        $future = 
            new self($closure, $args);
        $future->start();
        return $future;
    }

	public function __destruct() {
		if (Thread::getCurrentThreadId() == $this->owner) {
			$this->join();
		}
	}
    
	protected $owner;
    protected $closure;
    protected $args;
    protected $result;
}

$test = ["Hello", "World"];

$closure = function($test) {
    return $test;
};

/* make call in background thread */
$future = Future::of($closure, [$test]);

/* get result of background and foreground call */
var_dump($future->getResult(), $closure($test));
?>
