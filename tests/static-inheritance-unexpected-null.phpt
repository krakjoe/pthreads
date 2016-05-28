--TEST--
Testing gh bug #585 (Static class member nulled after class extends)
--FILE--
<?php
class A{
    public static $var = 'A';

    public function gVar(){
        return static::$var;
    }
}

class B extends A{
    public static $var = 'B';
}

class thr extends Thread{
    public function run(){
        $a = new A();
        var_dump($a->gVar());
    }
}

$thr = new thr();
$thr->start();
$thr->join();
--EXPECT--
string(1) "A"
