--TEST--
Test pthreads workers default collector
--DESCRIPTION--
This test verifies that the default collector works as expected
--FILE--
<?php
$worker = new Worker();
$worker->start();

$i = 0;
while ($i<10) {
	$worker->stack(new class extends Threaded implements Collectable {
		public function run() {
			$this->garbage = true;
		}

		public function isGarbage() : bool {
			if ($this->garbage)
				var_dump($this->garbage);
			return $this->garbage;
		}

		private $garbage = false;
	});
	$i++;
}

var_dump($i);
while ($worker->collect()) continue;
var_dump($worker->getStacked());
$worker->shutdown();
?>
--EXPECTF--
int(10)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)
int(0)
