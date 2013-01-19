<?php
class StackableArray extends Stackable implements ArrayAccess {
	public $counter;

    public function run() {}

    public function __construct() {
        $this->counter = 0;
    }
    public function offsetExists($offset) {
        return isset($this[$offset]);
    }
    public function offsetUnset($offset) {
        unset($this[$offset]);
    }
    public function offsetGet($offset) {
        return isset($this[$offset]) ? $this[$offset] : null;
    }
    public function offsetSet($offset, $value) {
        if(is_null($offset)) {
                $this[++$this->counter] = $value;
        } else {
                $this[$offset] = $value;
        }
    }
}

/** this should work now **/
/** eventually without the call to increment the couter **/
$s = new StackableArray();
$s[++$s->counter]="h";
print_r($s);
?>
