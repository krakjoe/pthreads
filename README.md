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

## Supported PHP Versions

While pthreads will compile against 5.4, only 5.3 versions are officially supported at this time. 

There are differences that are still being researched in 5.4 that need to be addressed before pthreads can be used in a 5.4 environment.
5.3.16 is the version that I am using to develop the code but I have taken the time to compile against every release in the 5.3 series so far and can report than everything works as intended in all versions from 5.3.

5.2 is unsupported, and will remain that way as the last release in the 5.2 series was 18 months ago; I'd much rather look to supporting future versions than past.

### Hello World
```php
<?php
class AsyncOperation extends Thread {
  public function __construct($arg){
    $this->arg = $arg;
  }

  public function run(){
    if($arg){
      printf("Hello %s\n", $arg);
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