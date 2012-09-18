<?php
class MyWorker extends Thread {
	public function run(){
		if($this->attempt>1)
			usleep(rand(1000, 3000));
		printf("%s: %lu running: %d/%d ...\n", __CLASS__, $this->getThreadId(), $this->attempt++, $this->getStacked());
		return $this->attempt;
	}
}
$work = new MyWorker;
while($i++<10){
	printf(
		"Stacking: %d/%d\n", $i, $work->stack($work)
	);
}
$work->start();
usleep(10000);
while($i++<20){
	printf(
		"Stacking: %d/%d\n", $i, $work->stack($work)
	);
}
printf("Result: %d/%d\n", $work->join(), $work->getStacked());
?>