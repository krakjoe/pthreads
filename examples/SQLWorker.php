<?php
class SQLQuery extends Stackable {
	
	public function __construct($sql) { 
		$this->sql = $sql; 
	}
	
	public function run() {
		/** included in 0.0.37 is access to the real worker object for stackables **/
		$this->synchronized(function(){
		    if ($this->worker->isReady()) {
			    $result = mysql_query($this->sql, $this->worker->getConnection());  
			    if ($result) {
				    while(($row = mysql_fetch_assoc($result))){
					    /** $this->rows[]=$row; segfaults */
					    /** you could array_merge, but the fastest thing to do is what I'm doing here */
					    /** even when the segfault is fixed this will still be the best thing to do as writing to the object scope will always cause locking */
					    /** but the method scope isn't shared, just like the global scope **/
					    $rows[]=$row;
				    }
				    mysql_free_result($result);
			    } else printf("%s got no result\n", __CLASS__);
		    } else printf("%s not ready\n", $this->worker->getConnection());
		    $this->rows = $rows;
		    $this->notify();
		});
	}
	
	protected function getResults() { return $this->rows; }
}

class SQLWorker extends Worker {
	public $mysql;	
	
	public function __construct($host, $user, $pass, $db) {
		$this->host = $host;
		$this->user = $user;
		$this->pass = $pass;
		$this->db = $db;
		$this->ready = false;
		$this->name = null;
	}
	public function run() {
		$this->setName(sprintf("%s (%lu)", __CLASS__, $this->getThreadId()));
		if (($this->mysql = mysql_connect($this->host, $this->user, $this->pass))) {
			$this->ready = (boolean) mysql_select_db($this->db, $this->mysql);
		}

	}
	public function getConnection(){ return $this->mysql; }
	protected function isReady($flag = null) { 
		if (is_null($flag)) {
			return $this->ready;
		} else $this->ready = $flag;
	}
	public function setName($name) 	{ $this->name = $name;}
	public function getName() 		{ return $this->name; }
}

if (count($argv) == 6) {
	$sql = new SQLWorker($argv[1], $argv[2], $argv[3], $argv[4]);
	$sql->start();
	$query = new SQLQuery($argv[5]);
	$sql->stack($query);
	$query->synchronized(function($query){
	    if (!$query->rows) {
	        $query->wait();
	    }
	}, $query);
	print_r($query->getResults());
	$sql->shutdown();
} else printf("usage: {$argv[0]} hostname username password database query\n");
?>
