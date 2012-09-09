<?php

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