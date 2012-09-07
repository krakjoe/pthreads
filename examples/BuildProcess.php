<?php
/*
* This code is what is used to test and build pthreads on as many versions of php as I have in my /usr/src folder
* This is not useful to you but a good example of threading making light work of something very laborious
* I sit in the root of the source I'm working in so /usr/src/php-5.4.6 for example:
*	Scan /usr/src for versions of php
*	Copy current code from ext/pthreads to every version of php in /usr/src
*	Create a thread for each version to configure, optionally make clean, make and run-tests to ensure my changes are good
* The script will exit when they finish ...
*/

class Build extends Thread {
	
	public function __construct($location, $clean, $verbose){
		$this->location = $location;
		$this->clean = $clean;
		$this->verbose = $verbose;
		$this->start();
	}

	public function run(){
		if(is_dir($this->location)){
			chdir($this->location);
			$named = preg_split("~/~", $this->location);
			$named = $named[count($named)-1];
			$command = "./configure --disable-all --enable-maintainer-zts --enable-pthreads --enable-debug --prefix=/usr/src/debug/$named 2>&1";
			printf("%s\n", $command);
			exec(
				$command,
				$stdout,
				$result
			);
			if ($result == 0) {
				if($this->clean)
					exec("make clean 2>&1", $stdout, $result);
				else	exec("rm -rf ext/pthreads/*.o ext/pthreads/*.lo");
				exec("make 2>&1", $stdout, $result);
				if( $result == 0 ){
					exec("TEST_PHP_EXECUTABLE=sapi/cli/php sapi/cli/php run-tests.php ext/pthreads 2>&1", $stdout, $result);
					if ($result == 0)
						printf("Tests passed in %s\n", $named);
					if ($result != 0 || $this->verbose) {
						echo implode("\n", $stdout);
						echo "\n";
					}
				}
			} else {
				echo implode("\n", $stdout);
				echo "\n";
			}
		}
	}
}

$threads = array();
foreach(glob("/usr/src/php-5*") as $version){
	$check = preg_split("~\.~", $version);
	if ($check[count($check)-1]!="bz2" && $check[count($check)-1]!="gz") {
		exec(
			"cp -rvf ext/pthreads/* $version/ext/pthreads 2>&1 >/dev/null"
		);
		$threads[count($threads)]=new Build($version, $argv[1], $argv[2]);
	}
}

?>