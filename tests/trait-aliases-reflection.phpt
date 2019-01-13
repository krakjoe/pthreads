--TEST--
Test trait aliases reflection
--DESCRIPTION--
Trait aliases not properly copied
--FILE--
<?php
trait Hello {
    public function world():void { }
}

class Foo {
    use Hello { world as sun; }
}
$t = new class extends Thread {
    public function run() {
        $foo = new Foo();

        $class = new ReflectionClass($foo);
        var_dump($class->getTraitAliases());
    }
};
$t->start() && $t->join();
--EXPECT--
array(1) {
  ["sun"]=>
  string(12) "Hello::world"
}

