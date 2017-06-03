<?php

const THREADS_NUMBER = 5;
const REQUESTS_NUMBER = 20;

//queue to push requests to process
$requestsQueue = new Queue();
//queue to push responses
$responsesQueue = new Queue();
$monitor = new class extends Threaded {

    protected $done;

    public function isDone()
    {
        return $this->synchronized(function(){
            return $this->done;
        });
    }

    public function setDone()
    {
        $this->synchronized(function(){
            $this->done = true;
        });
    }

};
$threads = [];
for ($i = 0; $i < THREADS_NUMBER; $i++) {
    $thread = new class ($requestsQueue, $responsesQueue, $monitor) extends Thread {
        protected $requestsQueue;
        protected $responsesQueue;
        protected $monitor;

        public function __construct($requestsQueue, $responsesQueue, $monitor)
        {
            $this->requestsQueue = $requestsQueue;
            $this->responsesQueue = $responsesQueue;
            $this->monitor = $monitor;
        }

        public function run()
        {
            while (!$this->monitor->isDone()) {
                if ($url = $this->requestsQueue->shift()) {
                    $response = file_get_contents($url);
                    $this->responsesQueue->push([
                        'url' => $url,
                        'response' => $response
                    ]);
                } else {
                    usleep(100000);
                }
            };
        }

    };
    $thread->start();
    $threads[] = $thread;
}

for ($i = 0; $i < REQUESTS_NUMBER; $i++) {
    $requestsQueue->push(sprintf("http://www.bing.com/search?q=%s", $i));
}

$collected = 0;
while (true) {
    if ($response = $responsesQueue->shift()) {
        echo sprintf("$collected got response for url '%s' length is %s\n", $response['url'], strlen($response['response']));
        $collected++;
        if ($collected >= REQUESTS_NUMBER) {
            $monitor->setDone();
            echo "collected all\n";
            exit;
        }
    } else {
        usleep(100000);
    }
}