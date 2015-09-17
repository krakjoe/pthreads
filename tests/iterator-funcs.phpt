--TEST--
Testing iterator functions
--DESCRIPTION--
Iterator functions were not properly copied, causing unexpected results and unsafe execution.
--FILE--
<?php
class MyIterator implements \Iterator { 
        protected $items;
        protected $position;

        public function __construct() {
                $this->items = array("joe", "bob", "fred");
                $this->position = 0;
        }

        public function next() {
                ++$this->position;
        }

        public function key() {
                return $this->position;
        }

        public function current() {
                return $this->items[$this->position];
        }

        public function valid() {
                return ($this->position < count($this->items));
        }

        public function rewind() {
                $this->myProtectedMethod();
                $this->position = 0;
        }

        protected function myProtectedMethod() {}
}

class MyThread extends \Thread {
        public function run() {
                $it = new \MyIterator();
                foreach ($it as $item) {}
                print "SUCCESS";
        }
}

$items = new \MyIterator();
foreach ($items as $item) {}
$thread = new \MyThread();
$thread->start();
$thread->join();
--EXPECT--
SUCCESS
