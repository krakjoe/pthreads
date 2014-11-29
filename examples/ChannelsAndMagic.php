<?php
/*
 * This example demonstrates the use of magic methods, and is a possible answer to the question:
 *   "Can we have something like go channels with pthreads?"
 * Channels in go abstract away the complexity of parallel programming, but are dependant on goroutines
 *   and language level support.
 *  We can't have language level support easily, but we can implement channels using magic PHP.
 */
 
/*
 * A channel needs a pool to execute, and a routine definition
 */
abstract class Channel extends Threaded {

    public function __construct(Pool $pool) {
        $pool->submit($this);
    }
    
    public function __set($key, $value) {
        return $this->synchronized(function() use ($key, $value) {
            $this[$key] = $value;
            return $this->notify();
        });
    }
    
    public function __get($key) {
        return $this->synchronized(function() use($key) {
            while (!isset($this[$key]))
                $this->wait();
            return $this[$key];
        });
    }
    
    public function run() { $this->routine(); }
    
    abstract public function routine();
}

###################################################################################################

class TestChannel extends Channel {

    public function routine() {
        /* sending data on the channel will take care of synchronization */
        $this["message"] = "Hello World";
        $this["gold"] = 3.462;
    }
}

$pool = new Pool(4);

$channel = new TestChannel($pool);

/* recving data from the channel will take care of synchronization */

printf("Message: %s, Gold: %.3f\n", 
    $channel["message"], 
    $channel["gold"]);

/* collect, sometime !! */

$pool->shutdown();
?>
