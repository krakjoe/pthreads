--TEST--
Test constant stdio copy bug #806 and #583 
--DESCRIPTION--
Copy stdin, stdout and stderr at thread creation with PTHREADS_INHERIT_ALL.
--FILE--
<?php
$thread = new class extends \Thread{
    public function run(){
        var_dump(defined('STDOUT'));

        fwrite(STDOUT, 'Hello'. PHP_EOL);
    }
};

$thread->start(PTHREADS_INHERIT_ALL) && $thread->join();

fwrite(STDOUT, 'World'. PHP_EOL);
?>
--EXPECTF--
bool(true)
Hello
World
