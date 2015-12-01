# Threading for PHP - Share Nothing, Do Everything :)

[![Build Status](https://travis-ci.org/krakjoe/pthreads.svg?branch=master)](https://travis-ci.org/krakjoe/pthreads)
[![Average time to resolve an issue](http://isitmaintained.com/badge/resolution/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Average time to resolve an issue")
[![Percentage of issues still open](http://isitmaintained.com/badge/open/krakjoe/pthreads.svg)](http://isitmaintained.com/project/krakjoe/pthreads "Percentage of issues still open")
[![Join the chat at https://gitter.im/krakjoe/pthreads](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/krakjoe/pthreads?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

This project provides multi-threading that is compatible with PHP based on Posix Threads.

## Highlights

* An easy to use, quick to learn OO Threading API for PHP7
* Execute any and all predefined and user declared methods and functions, including closures.
* Ready made synchronization included
* A world of possibilities ...

## Technical Features

* High Level Threading
* Synchronization
* Worker Threads
* Thread Pools
* Complete Support for OO - ie. traits, interfaces, inheritance etc
* Full read/write/execute support for Threaded objects

## Requirements

* PHP7+
* ZTS Enabled ( Thread Safety )
* Posix Threads Implementation

Testing has been carried out on x86, x64 and ARM, in general you just need a compiler and pthread.h

## PHP7

For PHP7, pthreads has been almost completely rewritten to be more efficient, easier to use and more robust.

While documentation on php.net is waiting to be updated and translated, I will give a brief changelog here:

The API for v3 has changed, the following things have been removed:

 * ```Mutex``` and ```Cond```
 * ```Threaded::lock``` and ```Threaded::unlock```
 * ```Threaded::isWaiting```
 * ```Threaded::from```
 * ```Thread::kill``` (there be dragons)
 * ```Threaded::getTerminationInfo``` (this was unsafe, a better, safe impl can be done in userland)
 * Special behaviour of ```protected``` and ```private``` methods on ```Threaded``` objects

The following things have significant changes:
 
 * The method by which ```Threaded``` objects are stored as member properties of other ```Threaded``` objects.
 * The structure used by a ```Worker``` for stack (```Collectable``` objects to execute inserted by ```Worker::stack```).
 * The ```Pool::collect``` mechanism was moved from ```Pool``` to ```Worker``` for a more robust ```Worker``` and simpler ```Pool``` inheritance.
 * The method by which iteration occurs on ```Threaded``` objects, such that it uses memory more efficiently.
 * ```Threaded::synchronized``` provides true synchronization (state and properties lock).
 * ```Worker``` objects no longer require that you retain a reference to ```Collectable``` objects on the stack.
 * Unified monitor (cond/mutex/state) for ```Threaded``` objects
 * ```Threaded``` members of ```Threaded``` objects are immutable
 * ```Volatile``` objects, excempt from immutability
 * ```array``` coerced to ```Volatile``` when set as member of ```Threaded```
 * ```Collectable``` converted to interface, to make ```extends Volatile implements Collectable``` possible.

Some blog posts explaining these changes:

 * [A Letter from the Future](http://blog.krakjoe.ninja/2015/08/a-letter-from-future.html)
 * [Addendum to A Letter from the Future](http://blog.krakjoe.ninja/2015/09/addendum-letter-from-future.html)

As of yet, documentation has not been updated to reflect these changes ... sorry about that ... one human ...

More detail to come ...

### Supported PHP Versions

pthreads v3 requires PHP7 or above. PHP5 needs to use pthreads v2 which can be found in the PHP5 branch.

### Windows Support

Yes !! Windows support is offered thanks to the pthread-w32 library.

Releases for Windows can be found: http://windows.php.net/downloads/pecl/releases/pthreads/

##### Simple Windows Installation

* Add `pthreadVC2.dll` (included with the Windows releases) to the same directory as `php.exe` eg. `C:\xampp\php`
* Add `php_pthreads.dll` to PHP extention folder eg. `C:\xampp\php\ext`

### Mac OSX Support

Yes !! Users of Mac will be glad to hear that pthreads is now tested on OSX as part of the development process.

### Hello World

As is customary in our line of work:

```php
<?php
$thread = new class extends Thread {
	public function run() {
		echo "Hello World\n";
	}
};

$thread->start() && $thread->join();
?>
```

### Are you serious ?

Absolutely, this is not a hack, we _don't_ use forking or any other such nonsense, what you create are honest to goodness posix threads that are completely compatible with PHP and safe ... this is true multi-threading :)

### SAPI Support

pthreads v3 is restricted to operating in CLI only: I have spent many years trying to explain that threads in a web server just don't make sense, after 1,111 commits to pthreads I have realised that, my advice is going unheeded.

So I'm promoting the advice to hard and fast fact: *you can't use pthreads safely and sensibly anywhere but CLI.*

Thanks for listening ;)

### Documentation 

Documentation can be found in the PHP manual: http://docs.php.net/manual/en/book.pthreads.php, and some examples can be found in the "examples" folder in the master repository.

Further insights and occasional announcements can be read at the http://pthreads.org site where pthreads is developed and tested in the real world.

Here are some links to articles I have prepared for users: everybody should read them before they do anything else:

 - https://gist.github.com/krakjoe/6437782
 - https://gist.github.com/krakjoe/9384409

If you have had the time to put any cool demo's together and would like them showcased on pthreads.org please get in touch.

*Note: the documentation in the manual and the documents above refer to pthreads v2, it has not yet been updated ... anyone want to help ?*

### Polyfill

It's possible to write code that optionally takes advantage of parallelism where the environment has ```pthreads``` loaded.

This is made possible by [pthreads-polyfill](http://github.com/krakjoe/pthreads-polyfill) which can be found on [packagist](https://packagist.org/packages/krakjoe/pthreads-polyfill).

Having required the appropriate package in your composer.json, the following code is executable everywhere:

```php
<?php
require_once("vendor/autoload.php");

if (extension_loaded("pthreads")) {
	    echo "Using pthreads\n";
} else  echo "Using polyfill\n";

$pool = new Pool(4);

$pool->submit(new class extends Threaded Implements Collectable {
	    public function run() {
	            echo "Hello World\n";
	            $this->garbage = true;
	    }

		public function isGarbage() : bool { 
			return $this->garbage; 
		}

		private $garbage = false;
});

while ($pool->collect(function(Collectable $task){
	    return $task->isGarbage();
})) continue;

$pool->shutdown();
?>
```

Some guidance on getting started, and detail regarding how the polyfill came to exist can be found [here](http://blog.krakjoe.ninja/2015/09/what-polly-really-wants.html).

### Feedback

Please submit issues, and send your feedback and suggestions as often as you have them.

### Reporting Bugs

If you believe you have found a bug in pthreads, please open an issue: Include in your report *minimal, executable, reproducing code*.

Minimal:     reduce your problem to the smallest amount of code possible; This helps with hunting the bug, but also it helps with integration and regression testing once the bug is fixed.

Executable:  include all the information required to execute the example code, code snippets are not helpful.

Reproducing: some bugs don't show themselves on every execution, that's fine, mention that in the report and give an idea of how often you encounter the bug.

__It is impossible to help without reproducing code, bugs that are opened without reproducing code will be closed.__

Please include version and operating system information in your report.

*Please do not post requests to help with code on github; I spend a lot of time on Stackoverflow, a much better place for asking questions.*

Have patience; I am one human being.

### Developers

There is no defined API for you to create your own threads in your extensions, this project aims to provide Userland threading, it does not aim to provide a threading API for extension developers. I suggest you allow users to decide what they thread and keep your own extension focused on your functionality.
