<?php

define("SQL_HOST", "127.0.0.1");
define("SQL_USER", "root");
define("SQL_PASS", "");
define("SQL_DB", "mysql");

class sql
{
    public static $connection;

    public static function __callstatic($method, $args)
    {
        if (self::$connection) {
            return call_user_func_array(array(self::$connection, $method), $args);
        } else {
            self::$connection = new mysqli(SQL_HOST, SQL_USER, SQL_PASS, SQL_DB);
            if (self::$connection) {
                return call_user_func_array(array(self::$connection, $method), $args);
            }
        }
    }
}

class UserThread extends Thread
{

    private $config;

    function __construct($config = null)
    {
        $this->config = $config;
    }

    /**
     * {@inheritdoc}
     * @see Thread::run()
     */
    public function run()
    {
        /* execute queries */
        if (sql::query("SELECT * FROM mysql.user")) {
            printf("...\n");
            if (sql::query("SELECT * FROM mysql.user")) {
                printf("...\n");
            }
        }
    }
}

if (sql::query("SELECT * FROM mysql.user;")) {
    printf("...\n");
    if (sql::query("SELECT * FROM mysql.user;")) {
        printf("...\n");
    }
}

$thread = new UserThread();
$thread->start();
