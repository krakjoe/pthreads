--TEST--
Private class constant fetching
--DESCRIPTION--
This test ensures that private class constants are properly fetched when
static:: is used.
--FILE--
<?php
class A
{
	private const A = 1;
	private const B = [2, []];
	private const C = true;
	private const D = false;
	private const E = 1.1;
	private const F = 'a';

	public static function getA()
	{
		var_dump(
			static::A,
			static::B,
			static::C,
			static::D,
			static::E,
			static::F
		);
	}
}

$t = new class extends Thread {
	public function run()
	{
		A::getA();
	}
};

$t->start() && $t->join();
--EXPECT--
int(1)
array(2) {
  [0]=>
  int(2)
  [1]=>
  array(0) {
  }
}
bool(true)
bool(false)
float(1.1)
string(1) "a"
