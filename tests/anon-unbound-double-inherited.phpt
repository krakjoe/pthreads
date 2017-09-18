--TEST--
Test anonymous classes (unbound double inherited class)
--DESCRIPTION--
This test verifies that anonymous Threaded objects (with double inheritance)
work as expected
--FILE--
<?php

/** c */
class C extends Threaded
{
    use T1, T2 {
        T2::t insteadof T1;
        T1::t as t2;
    }
    const C = 0;
    public $c2 = 2;
    public function __construct(){}
    public function __destruct(){}
    public function __call($a, $b){}
    public static function __callStatic($a, $b){}
    public function __set($p, $v){}
    public function __isset($p){}
    public function __unset($p){}
    public function __sleep(){}
    public function __wakeup(){}
    public function __toString(){}
    public function __invoke(){}
    public function __set_state($ps){}
    public function __clone(){}
    public function __debugInfo(){}
    public function run(){var_dump(self::C, $this->c2);}
}

trait T1 {function t(){}}
trait T2 {function t(){}}

$w = new Worker();
$w->start();
$w->stack(/** b */new class extends C {});
$w->shutdown();
--EXPECT--
int(0)
int(2)