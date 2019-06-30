--TEST--
Basic anonymous class support, fix #505
--DESCRIPTION--
Unbound anon class causing segfaults, we delay copy but still cannot serialize the anon
--FILE--
<?php
class Test extends Thread {
	/**
	 * doccomment run
	 */
	public function run() {
		$this->alive = true;
		/**
		 * doccomment anonymous
		 */
		$this->anonymous = new class extends Thread {
			const CONSTANT = 'constant';
			/**
			 * @var
			 */
			public $pubProp;
			protected $protProp;
			private $privProp;
			public static $staticProp;
			public function run() {
				var_dump('anonymous run');
				$this->ready = true;
			}
			public function method() {
				var_dump('method executed');
			}
			public static function staticMethod() {}
		};
		var_dump($this->anonymous);
		$this->anonymous->start();
		$this->anonymous->join();
		while($this->alive) {}
	}
}
$test = new Test();
$test->start();
while(true) {
	if(isset($test->anonymous, $test->anonymous->ready)) {
		var_dump($test->anonymous);
		$test->anonymous->run();
		$test->anonymous->method();
		$test->alive = false;
		break;
	}
}
$test->join();
--EXPECTF--
object(class@anonymous)#%d (3) {
  ["pubProp"]=>
  NULL
  ["protProp"]=>
  NULL
  ["privProp"]=>
  NULL
}
string(13) "anonymous run"
object(class@anonymous)#%d (4) {
  ["pubProp"]=>
  NULL
  ["protProp"]=>
  NULL
  ["privProp"]=>
  NULL
  ["ready"]=>
  bool(true)
}
string(13) "anonymous run"
string(15) "method executed"