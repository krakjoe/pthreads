--TEST--
Test sane handling of resources from foreign contexts
--DESCRIPTION--
Test that resources from other thread contexts can properly be accessed and are only destroyed by their owner
--FILE--
<?php
class myWorker extends Worker {
    private $foreignResource;

    public function __construct($useResource) {
        $this->foreignResource = $useResource;
        var_dump([
            'Resource received and stored:',
            is_resource($useResource),
            is_resource($this->foreignResource),
        ]);
    }

    public function run () {

        // polute EG(regular_list) with another resource of our own
        $localThreadFP = tmpfile();

        var_dump(array(
            'Foreign resource conversion test:',

            // test conversion followed by destruction
            // the resource shall be added and removed from
            // PTHREADS_ZG(resources) and EG(regular_list) by the following
            // statement:

            is_resource($this->foreignResource),

            // now make the resource permanently available locally to tasks by
            // storing it in a variable. It will stay in PTHREADS_ZG(resources)
            // and EG(regular_list) until the worker is shutdown):

            is_resource($foreignResource = $this->foreignResource),

            // test that we still get the correct resource when it's already in
            // EG(regular_list)
            $foreignResource === $this->foreignResource
        ));

        fclose($localThreadFP);
    }

}

class Work extends Threaded implements Collectable {

    public function run () {
	$foreignResource = $this->worker->foreignResource;

        var_dump(array(
            'Foreign resource is accessible from task:',
            is_resource($foreignResource),
            $foreignResource === $this->worker->foreignResource,
            'wrote ' . fwrite($foreignResource, '42') . ' bytes',
        ));

    }

	public function isGarbage() : bool { return true; }
}

$fp = tmpfile();

$task = new Work();

$worker = new myWorker($fp);
$worker->start();
$worker->stack($task);

var_dump(array(
    'Our worker is shutdown:',
    $worker->shutdown(),
));

var_dump(array(
    'Our resource is still valid:',
    is_resource($fp) && rewind($fp),
    fread($fp, 2) === "42",
));

var_dump(array(
    'Our resource can be closed:',
     fclose($fp),
 ));

var_dump(array(
    'Our resource is closed:',
    !is_resource($fp),
));
?>
--EXPECT--
array(3) {
  [0]=>
  string(29) "Resource received and stored:"
  [1]=>
  bool(true)
  [2]=>
  bool(true)
}
array(4) {
  [0]=>
  string(33) "Foreign resource conversion test:"
  [1]=>
  bool(true)
  [2]=>
  bool(true)
  [3]=>
  bool(true)
}
array(4) {
  [0]=>
  string(41) "Foreign resource is accessible from task:"
  [1]=>
  bool(true)
  [2]=>
  bool(true)
  [3]=>
  string(13) "wrote 2 bytes"
}
array(2) {
  [0]=>
  string(23) "Our worker is shutdown:"
  [1]=>
  bool(true)
}
array(3) {
  [0]=>
  string(28) "Our resource is still valid:"
  [1]=>
  bool(true)
  [2]=>
  bool(true)
}
array(2) {
  [0]=>
  string(27) "Our resource can be closed:"
  [1]=>
  bool(true)
}
array(2) {
  [0]=>
  string(23) "Our resource is closed:"
  [1]=>
  bool(true)
}

