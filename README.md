# Posix Threading for PHP5+

This project aims to provide multi-threading that is compatible with PHP based on Posix Threads.

## Features

* Posix Threading
* Mutex
* Condition Variables

This is enough to test the water, should the extension prove popular and stable more will be developed.

## Requirements

* PHP5.3.16* on x64 or x86
* ZTS Enabled ( Thread Safety )
* Posix Threads implementation that PHP will compile with

Testing has been carried out both on x86 and x64 architechtures with varying hardware, this code may work in untested environments, keep me updated ...

### *Supported PHP Versions

pthreads should compile and work well with any version of PHP from 5.3.0 to 5.4.6 as from version 0.14 of pthreads ( committed 05/09/2012 )

The support for 5.4 is about 5 minutes old, please do report any feedback you have when working with ANY supported version of PHP.

### Hello World

As is customary in our line of work:

```php
<?php
class AsyncOperation extends Thread {
  public function __construct($arg){
    $this->arg = $arg;
  }

  public function run(){
    if($this->arg){
      printf("Hello %s\n", $this->arg);
    }
  }
}
$thread = new AsyncOperation("World");
if($thread->start())
  $thread->join();
?>
```

### Documentation

Some documentation can be found on the Github Wiki pages, and some examples can be found in the "examples" folder in the master repository.

### Feedback

Please submit issues, and send your feedback and suggestions as often as you have them.

### Developers

There is no defined API for you to create your own threads in your extensions, this project aims to provide Userland threading, it does not aim to provide a threading API for extension developers. I suggest you allow users to decide what they thread and keep your own extension focused on your functionality.