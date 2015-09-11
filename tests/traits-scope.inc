<?php
class MyClass {
	use \MyTrait;

	public function doSomething() {
		$this->getSomething(true);
		printf("OK\n");
	}
}

