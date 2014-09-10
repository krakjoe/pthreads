--TEST--
Test isset and empty are compliant with standard objects (fix bug #347)
--FILE--
<?php

class standard {
    public $t_false = false;
    public $t_null = null;
    public $t_emptyStr = "";
    public $t_zero1 = 0;
    public $t_zero2 = 0.0;
    public $t_zero3 = "0";
    public $t_notSet;
    public $t_notEmpty = true;
}

class threadedObject extends \Threaded {
    public $t_false = false;
    public $t_null = null;
    public $t_emptyStr = "";
    public $t_zero1 = 0;
    public $t_zero2 = 0.0;
    public $t_zero3 = "0";
    public $t_notSet;
    public $t_notEmpty = true;
}

$stdObject = new standard;

$threaded = new threadedObject;

foreach ([
    't_false',
    't_null',
    't_emptyStr',
    't_zero1',
    't_zero2',
    't_zero3',
    't_notSet',
    't_notEmpty'] as $prop) {
    
    printf("%s:\n", $prop);
    var_dump(isset($stdObject->$prop) == isset($threaded->$prop));
    var_dump(empty($stdObject->$prop) == empty($threaded->$prop));
    printf("\n");
}

--EXPECT--
t_false:
bool(true)
bool(true)

t_null:
bool(true)
bool(true)

t_emptyStr:
bool(true)
bool(true)

t_zero1:
bool(true)
bool(true)

t_zero2:
bool(true)
bool(true)

t_zero3:
bool(true)
bool(true)

t_notSet:
bool(true)
bool(true)

t_notEmpty:
bool(true)
bool(true)
