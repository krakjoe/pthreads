<?php
/*
* In a large application, the overhead of each thread having to copy the entire context
* may become undesireable.
* Selective Inheritance serves as a way to choose which parts of the environment are available in threading contexts
* Following is some code that demonstrates the use of this feature
*
* Note: if a member of a pthreads object, is an object itself of a user defined type, and the class table is not inherited
*   you are asking for trouble !!
*/

class my_class {}

function my_function(){
    return __FUNCTION__;
}

define ("my_constant", 1);

class Selective extends Thread {
    public function run() {
        /* functions exist where PTHREADS_INHERIT_FUNCTIONS is set */
        var_dump(function_exists("my_function"));
        /* classes exist where PTHREADS_INHERIT_CLASSES is set **BE CAREFUL** */
        var_dump(class_exists("my_class"));
        /* constants exist where PTHREADS_INHERIT_CONSTANTS is set */
        var_dump(defined("my_constant"));
    }
}

?>
expect:
    bool(false)
    bool(false)
    bool(false)
<?php
$test = new Selective();
$test->start(PTHREADS_INHERIT_NONE);
$test->join();
?>
=======================================
expect:
    bool(false)
    bool(true)
    bool(true)
<?php
$test = new Selective();
$test->start(PTHREADS_INHERIT_ALL & ~PTHREADS_INHERIT_FUNCTIONS);
$test->join();
?>
=======================================
expect:
    bool(false)
    bool(false)
    bool(true)
<?php
$test = new Selective();
$test->start(PTHREADS_INHERIT_INI | PTHREADS_INHERIT_CONSTANTS);
?>
