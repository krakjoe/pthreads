<?php

/*
 * This seems like a pretty good way to show you the difference between threads that are
 * syncrhonized with mutex and those that aren't.
 *
 * Will show 50 very neat rows <-.........-> then 50 threads doing the same thing with no mutex
 */

class MyWorkerThread extends Thread
{
    public function __construct($limit, $mutex = null)
    {
        $this->limit = $limit;
        $this->mutex = $mutex;
    }

    /**
     * {@inheritdoc}
     * @see Thread::run()
     */
    public function run()
    {
        if ($this->mutex) {
            $locked = Mutex::lock($this->mutex);
        }

        printf("%s#%lu:<-", !empty($locked) ? "Y" : "N", $this->getThreadId());
        for ($i = 0; $i < $this->limit; $i++) {
            echo ".";
        }
        printf("->\n");

        if ($this->mutex) {
            Mutex::unlock($this->mutex);
        }

        return true;
    }
}

$timer = microtime(true);

// create and lock a mutex
$mutex = Mutex::create(true);

// create workers
$workers = array();
for ($i = 0; $i < 50; $i++) {
    $workers[$i] = new MyWorkerThread(rand(30, 100), $mutex);
    // they cannot go anywhere, I have the mutex
    $workers[$i]->start();
}

printf("Release the (muzzled) hounds ... :\n");

Mutex::unlock($mutex);
foreach ($workers as $i => $worker) {
    $workers[$i]->join();
}

printf("Muzzled: %f seconds\n", microtime(true) - $timer);

// please remember to destroy mutex and condition variables
Mutex::destroy($mutex);

$timer = microtime(true);

// same again, no mutex
printf("Now no mutex ... :\n");

$workers = array();
for ($i = 0; $i < 50; $i++) {
    $workers[$i] = new MyWorkerThread(rand(30, 100));
    // they cannot go anywhere, I have the mutex
    $workers[$i]->start();
}

foreach ($workers as $worker) {
    $worker->join();
}

printf("Dribbling: %f seconds\n", microtime(true) - $timer);
