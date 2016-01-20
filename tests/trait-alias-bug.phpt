--TEST--
Test pthreads connections
--DESCRIPTION--
This test verifies that variables are bound properly by pthreads
--FILE--
<?php

trait A {
  protected function x() {}
}

class B {
  use A {
    x as public;
  }
}

class My extends Thread{
    function run(){
        for($i=1;$i<2;$i++){
            echo Thread::getCurrentThreadId();
        }
    }
}

$a = new My();
$a->start();
$a->join();
--EXPECTF--
%d

