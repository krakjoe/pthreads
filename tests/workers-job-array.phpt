--TEST--
Test pthreads workers with array property
--DESCRIPTION--
This test verifies that threaded object property can return array value.
--FILE--
<?php

class Job extends Threaded {

    public function run()
    {
        $this->result = [1,2,3,4];
    }

    public function getResult()
    {
        return $this->result;
    }

    public function isGarbage() : bool {
        return count($this->result);
    }
    
    public $result;
}

$worker = new Worker;
$worker->start();
$worker->stack($jobs[] = new Job());
$worker->stack($jobs[] = new Job());
$worker->stack($jobs[] = new Job());
$worker->stack($jobs[] = new Job());
$worker->join();
foreach ($jobs as $job) {
    print_r($job->getResult());
}
while($worker->collect()) continue;
$worker->shutdown();
--EXPECTF--
