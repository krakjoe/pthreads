<?php
/**
* This example illustrates best practice with regard to using MySQLi in multiple threads
*
* For convenience and simplicity it uses a Pool.
**/

class Connect extends Worker {

    public function __construct($hostname, $username, $password, $database, $port = 3306) {
        $this->hostname = $hostname;
        $this->username = $username;
        $this->password = $password;
        $this->database = $database;
        $this->port     = $port;
    }
    
    public function getConnection() {
        if (!self::$link) {
            self::$link = new mysqli(
                $this->hostname, 
                $this->username, 
                $this->password, 
                $this->database, 
                $this->port);
        }
        
        /* do some exception/error stuff here maybe */       
         
        return self::$link;
    }
    
    protected $hostname;
    protected $username;
    protected $password;
    protected $database;
    protected $port;
    
    /**
    * Note that the link is stored statically, which for pthreads, means thread local
    **/
    protected static $link;
}

class Query extends Threaded {

    public function __construct($sql) {
        $this->sql = $sql;
    }
    
    public function run() {
        $mysqli = $this->worker->getConnection();
        
        $result = $mysqli->query($this->sql);
        
        if ($result) {    
            while (($row = $result->fetch_assoc())) {
                $rows[] = $row;
            }
        }
        
        $this->result = $rows;
    }
    
    public function getResult() {
        return $this->result;
    }
    
    protected $sql;
    protected $result;
}

$pool = new Pool(4, "Connect", ["localhost", "root", "", "mysql"]);
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->submit
    (new Query("SHOW PROCESSLIST;"));
$pool->shutdown();

/* ::collect is used here for shorthand to dump query results */
$pool->collect(function($query){
    var_dump(
        $done = $query->getResult());
    
    return count($done);
});
?>
