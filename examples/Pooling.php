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

		$this->setData(array(
			'class' => __CLASS__,
			'threadId' => $this->worker->getThreadId()
		));
	}

	/**
	 * Here you can use to set some variable be recovered in collector.
	 * When you set an array in the \Thread without casting, it will be
	 * transformed into a Volatile object and will not be possible to recover
	 * the variable by collector. You can test it removing the cast and running
	 * the example.
	 *
	 * @see https://github.com/krakjoe/pthreads/issues/610.
	 *
	 * @param array $data Any data you need to recover by collector.
	 */
	public function setData($data)
	{
		if (is_array($data)) {
			$this->data = (array) $data;
			return;
		}

		$this->data = $data;
	}

	public $data;
}

class SafeLog extends Threaded {
	
	/*
	* If logging were allowed to occur without synchronizing
	*	the output would be illegible
	*/
	public function log($message, ... $args) {
		$this->synchronized(function($message, ... $args){
			if (is_callable($message)) {
				$message(...$args);
			} else echo vsprintf("{$message}\n", ... $args);
		}, $message, $args);
	}
}

$logger = new SafeLog();

/*
* Constructing the Pool does not create any Threads
*/
$pool = new Pool(8, 'WebWorker', [$logger, ["sqlite:example.db"]]);

/*
* Work count will be used to get all results
*/
$workCount = 14;

/*
* Only when there is work to do are threads created
*/
for ($i=0; $i < $workCount; $i++) {
	$pool->submit(new WebWork());
}

/** @var array It will be populated by collect callback */
$garbage = array();

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
* The second rule that will be checked is that the garbage must be full with
* the same count of submited workers.
*
* Collecting in a continuous loop will cause the garbage list to be emptied.
*/
while ($pool->collect(function($work) use (&$garbage) {
	if ($work->isGarbage()) {
		$garbage[] = $work->data;
	}

	return $work->isGarbage();
}) || count($garbage) < $workCount) continue;

/*
* We could submit more stuff here, the Pool is still waiting for Collectables
*/
$logger->log(function($pool) {
	var_dump($pool);
}, 'Printing pool before shutdown', $pool);

/*
* Shutdown Pools at the appropriate time, don't leave it to chance !
*/
$pool->shutdown();

/*
* The garbage could be accessed after pool shutdown.
*/
$logger->log(function($garbage) {
	var_dump($garbage);
}, 'Printing the garbage:', $garbage);
?>
