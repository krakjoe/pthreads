--TEST--
Test trait aliases
--DESCRIPTION--
Fix bug #274, trait aliases causing memory errors
--FILE--
<?php
trait testTrait {
    public function fromTrait ($foo) {
        var_dump(__FUNCTION__);
    }
}

class myThread extends Thread {
    use testTrait  {fromTrait as fromTraitAliased;}

    public function run() {
        $this->fromTrait('blah');
    }
}

$t = new myThread();
$t->start();
$t->join();
--EXPECT--
string(9) "fromTrait"

