<?php
/*
 * This example demonstrates the use of magic methods, and is a possible answer to the question:
 *   "Can we have something like go channels with pthreads?"
 * Channels in go abstract away the complexity of parallel programming, but are dependant on goroutines
 *   and language level support.
 *  We can't have language level support easily, but we can implement channels using magic PHP.
 */

class Channel extends Threaded {
    /* setting a value on the channel shall cause waiters to wake up */
    final public function __set($key, $value) {
        return $this->synchronized(function() use ($key, $value) {
            $this[$key] = $value;
            return $this->notify();
        });
    }
    
    /* getting a value on the channel shall cause callers to wait until it's available */
    final public function __get($key) {
        return $this->synchronized(function() use($key) {
            while (!isset($this[$key]))
                $this->wait();
            return $this[$key];
        });
    }
}

class Routine extends Threaded implements Collectable {
    public function __construct(Channel $channel) {
        $this->channel = $channel;
    }
    
    public function run() {
        /* sending on the channel */
        $this
            ->channel["message"] = "Hello World";
        $this
            ->channel["gold"] = 3.462;
    }

	public function isGarbage() : bool { return true; }
    
    protected $channel;
}

$channel = new Channel();
$pool = new Pool(4);

$pool->submit(
    new Routine($channel));

/* recving on the channel */
printf("Message: %s, Gold: %.3f\n", 
    $channel["message"], 
    $channel["gold"]);

$pool->shutdown();
?>
