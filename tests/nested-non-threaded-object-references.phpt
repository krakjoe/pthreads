--TEST--
Test nested non-threaded object references
--DESCRIPTION--
This test verifies the possibility to nest non-threaded object references
--FILE--
<?php
for ($i = 0; $i < 10; ++$i) {
    $a = new \stdClass();
    $a->b = [];
    for($j = 0; $j < 5; ++$j){
        $obj = new \stdClass();
        $obj->a = $a;
        $a->b[] = $obj;
    }
    new class($obj) extends \Threaded {
        public function __construct($faction) {
            $this->fraction = $faction;

            for ($i = 0; $i < 1000; ++$i) {
                $this->fraction;
            }
        }
    };
}
var_dump("ok");
?>
--EXPECT--
string(2) "ok"
