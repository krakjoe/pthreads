<?php
/**
* A synchronized Future for a Closure
*
*	This example takes a Closure and executes it in parallel, storing the result
*	and fetching the result are synchronized operations
*
*	This means that a call to getResult() will block the calling context until a result
*	is available
**/
class Future extends Thread {

    private function __construct(Closure $closure, array $args = []) {
        $this->closure = $closure;
        $this->args    = $args;	
    }

    public function run() {
        $this->synchronized(function() {
            $this->result = 
               (array) ($this->closure)(...$this->args);
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
    
	protected $owner;
    protected $closure;
    protected $args;
    protected $result;
}

/* some data */
$test = ["Hello", "World"];

/* a closure to execute in background and foreground */
$closure = function($test) {
    return $test;
};

/* make call in background thread */
$future = Future::of($closure, [$test]);

/* get result of background and foreground call */
var_dump($future->getResult(), $closure($test));
?>
