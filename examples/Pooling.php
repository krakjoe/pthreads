<?php
class WebWorker extends Worker {

	/*
	* We accept a SafeLog which is shared among Workers and an array
	* containing PDO connection parameters
	*
	* The PDO connection itself cannot be shared among contexts so
	* is declared static (thread-local), so that each Worker has it's
	* own PDO connection.
	*/
	public function __construct(SafeLog $logger, array $config = []) {
		$this->logger = $logger;
		$this->config = $config;
	}

	/*
	* The only thing to do here is setup the PDO object
	*/
	public function run() {
		if (isset($this->config)) {
			self::$connection = new PDO(... $this->config);
		}
	}

	public function getLogger()     { return $this->logger; }
	public function getConnection() { return self::$connection; }
	
	private $logger;
	private $config;
	private static $connection;
}

class WebWork extends Threaded {
	/*
	* An example of some work that depends upon a shared logger
	* and a thread-local PDO connection
	*/
	public function run() {
		$logger = $this->worker->getLogger();
		$logger->log("%s executing in Thread #%lu", 
			__CLASS__, $this->worker->getThreadId());

		if ($this->worker->getConnection()) {
			$logger->log("%s has PDO in Thread #%lu", 
				__CLASS__, $this->worker->getThreadId());
		}
	}
}

class SafeLog extends Threaded {
	
	/*
	* If logging were allowed to occur without synchronizing
	*	the output would be illegible
	*/
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args){
			echo vsprintf("{$message}\n", ... $args);
		}, $message, $args);
	}
}

/*
* Constructing the Pool does not create any Threads
*/
$pool = new Pool(8, 'WebWorker', [new SafeLog(), ["sqlite:example.db"]]);

/*
* Only when there is work to do are threads created
*/
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());
$pool->submit(new WebWork());

/*
* The Workers in the Pool retain references to the WebWork objects submitted
* in order to release that memory the Pool::collect method must be invoked in the same
* context that created the Pool.
*
* The Worker::collect method is invoked for every Worker in the Pool, the garbage list
* for each Worker is traversed and each Collectable is passed to the provided Closure.
*
* The Closure must return true if the Collectable can be removed from the garbage list.
*
* Worker::collect returns the size of the garbage list, Pool::collect returns the sum of the size of
* the garbage list of all Workers in the Pool. 
*
* Collecting in a continuous loop will cause the garbage list to be emptied. 
*/
while ($pool->collect(function($work){
	return $work->isGarbage();
})) continue;

/*
* We could submit more stuff here, the Pool is still waiting for Collectables
*/
var_dump($pool);

/*
* Shutdown Pools at the appropriate time, don't leave it to chance !
*/
$pool->shutdown();
?>
