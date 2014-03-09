<?php
/**
 * pthreads extension stub file for code completion purposes
 *
 * @author Lisachenko Alexander <lisachenko.it@gmail.com>
 * @version 1.0.0
 */

/**
 * The default inheritance mask used when starting Threads and Workers
 */
define('PTHREADS_INHERIT_ALL', 0x111111);

/**
 * Nothing will be inherited by the new context
 */
define('PTHREADS_INHERIT_NONE', 0);

/**
 * Determines whether the ini entries are inherited by the new context
 */
define('PTHREADS_INHERIT_INI', 0x1);

/**
 * Determines whether the constants are inherited by the new context
 */
define('PTHREADS_INHERIT_CONSTANTS', 0x10);

/**
 * Determines whether the class table is inherited by the new context
 */
define('PTHREADS_INHERIT_CLASSES', 0x100);

/**
 * Determines whether the function table is inherited by the new context
 */
define('PTHREADS_INHERIT_FUNCTIONS', 0x100);

/**
 * Determines whether the included_files table is inherited by the new context
 */
define('PTHREADS_INHERIT_INCLUDES', 0x10000);

/**
 * Determines whether the comments are inherited by the new context
 */
define('PTHREADS_INHERIT_COMMENTS', 0x100000);

/**
 * Allow output headers from the threads
 */
define('PTHREADS_ALLOW_HEADERS', 0x1000000);

/**
 * Basic thread implementation
 *
 * An implementation of a Thread should extend this declaration, implementing the run method.
 * When the start method of that object is called, the run method code will be executed in separate Thread.
 *
 * @link http://www.php.net/manual/en/class.thread.php
 */
abstract class Thread
{

    /**
     * Fetches a chunk of the objects properties table of the given size
     *
     * @param int $size The number of items to fetch
     *
     * @link http://www.php.net/manual/en/thread.chunk.php
     * @return array An array of items from the objects member table
     */
    final public function chunk($size) {}

    /**
     * Will return the size of the properties table
     *
     * @return int
     */
    final public function count() {}

    /**
     * Detaches a thread
     *
     * @return bool A boolean indication of success
     */
    final public function detach() {}

    /**
     * Will return the identity of the Thread that created the referenced Thread
     *
     * @link http://www.php.net/manual/en/thread.getcreatorid.php
     * @return int A numeric identity
     */
    final public function getCreatorId() {}

    /**
     * Will return the instance of currently executing thread
     *
     * @return static
     */
    public function getCurrentThread() {}

    /**
     * Will return the identity of the currently executing thread
     *
     * @link http://www.php.net/manual/en/thread.getcurrentthreadid.php
     * @return int
     */
    final public static function getCurrentThreadId() {}

    /**
     * Will return information concerning the location of the termination to aid debugging
     *
     * @return array|bool
     */
    final public function getTerminationInfo() {}

    /**
     * Will return the identity of the referenced Thread
     *
     * @link http://www.php.net/manual/en/thread.getthreadid.php
     * @return int
     */
    final public function getThreadId() {}

    /**
     * Tell if the referenced Thread has been joined by another context
     *
     * @link http://www.php.net/manual/en/thread.isjoined.php
     * @return bool A boolean indication of state
     */
    final public function isJoined() {}

    /**
     * Tell if the referenced Thread is executing
     *
     * @link http://www.php.net/manual/en/thread.isrunning.php
     * @return bool A boolean indication of state
     */
    final public function isRunning() {}

    /**
     * Tell if the referenced Thread has been started
     *
     * @link http://www.php.net/manual/en/thread.isstarted.php
     * @return bool A boolean indication of state
     */
    final public function isStarted() {}

    /**
     * Tell if the referenced Thread exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @link http://www.php.net/manual/en/thread.isterminated.php
     * @return bool A boolean indication of state
     */
    final public function isTerminated() {}

    /**
     * Tell if the referenced Thread is waiting for notification
     *
     * @link http://www.php.net/manual/en/thread.iswaiting.php
     * @return bool A boolean indication of state
     */
    final public function isWaiting() {}

    /**
     * Causes the calling context to wait for the referenced Thread to finish executing
     *
     * @link http://www.php.net/manual/en/thread.join.php
     * @return bool A boolean indication of state
     */
    final public function join() {}

    /**
     * Kills the referenced thread, dangerously !
     *
     * @link http://www.php.net/manual/en/thread.kill.php
     */
    final public function kill() {}

    /**
     * Lock the referenced objects storage for the calling context
     *
     * @link http://www.php.net/manual/en/thread.lock.php
     * @return bool A boolean indication of state
     */
    final public function lock() {}

    /**
     * Merges data into the current object
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag, by default true
     *
     * @link http://www.php.net/manual/en/thread.merge.php
     * @return bool A boolean indication of success
     */
    final public function merge($from, $overwrite = true) {}

    /**
     * Send notification to the referenced Thread
     *
     * @link http://www.php.net/manual/en/thread.notify.php
     * @return bool A boolean indication of success
     */
    final public function notify() {}

    /**
     * Pops an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/thread.pop.php
     * @return mixed The last item from the objects properties table
     */
    final public function pop() {}

    /**
     * The run method of a Thread is executed in a Thread when a call to Thread::start is made
     *
     * @link http://www.php.net/manual/en/thread.run.php
     * @return void The methods return value, if used, will be ignored
     */
    abstract public function run();

    /**
     * Shifts an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/thread.shift.php
     * @return mixed The first item from the objects properties table
     */
    final public function shift() {}

    /**
     * Will start a new Thread to execute the implemented run method
     *
     * @param int $options An optional mask of inheritance constants, by default PTHREADS_INHERIT_ALL
     *
     * @link http://www.php.net/manual/en/thread.start.php
     * @return bool A boolean indication of success
     */
    final public function start($options = PTHREADS_INHERIT_ALL) {}

    /**
     * Executes the block while retaining the synchronization lock for the current context.
     *
     * @param \Closure $function The block of code to execute
     * @param mixed $args... Variable length list of arguments to use as function arguments to the block
     *
     * @link http://www.php.net/manual/en/thread.synchronized.php
     * @return mixed The return value from the block
     */
    final public function synchronized(\Closure $function, $args = null) {}

    /**
     * Unlock the referenced objects storage for the calling context
     *
     * @link http://www.php.net/manual/en/thread.unlock.php
     * @return bool A boolean indication of success
     */
    final public function unlock() {}

    /**
     * Will cause the calling Thread to wait for notification from the referenced Thread
     *
     * @param int $timeout An optional timeout in millionths of a second
     *
     * @link http://www.php.net/manual/en/thread.wait.php
     * @return bool A boolean indication of success
     */
    final public function wait($timeout) {}
}

/**
 * Worker
 *
 * Worker Threads have a persistent context, as such should be used over Threads in most cases.
 *
 * @link http://www.php.net/manual/en/class.worker.php
 */
abstract class Worker
{
    /**
     * Fetches a chunk of the objects properties table of the given size
     *
     * @param int $size The number of items to fetch
     *
     * @link http://www.php.net/manual/en/worker.chunk.php
     * @return array An array of items from the objects member table
     */
    final public function chunk($size) {}

    /**
     * Will return the size of the properties table
     *
     * @return int
     */
    final public function count() {}

    /**
     * Will return the identity of the Thread that created the referenced Thread
     *
     * @link http://www.php.net/manual/en/worker.getcreatorid.php
     * @return int
     */
    final public function getCreatorId() {}

    /**
     * Returns the number of Stackables waiting to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.getstacked.php
     * @return int An integral value
     */
    final public function getStacked() {}

    /**
     * Will return information concerning the location of the termination to aid debugging
     *
     * @return array|bool
     */
    final public function getTerminationInfo() {}

    /**
     * Will return the identity of the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.getthreadid.php
     * @return int
     */
    final public function getThreadId() {}

    /**
     * Tell if the referenced Worker has been shutdown
     *
     * @link http://www.php.net/manual/en/worker.isshutdown.php
     * @return bool A boolean indication of state
     */
    final public function isShutdown() {}

    /**
     * Tell if the referenced Worker has been started
     *
     * @link http://www.php.net/manual/en/worker.isstarted.php
     * @return bool A boolean indication of state
     */
    final public function isStarted() {}

    /**
     * Tell if the referenced Worker exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @link http://www.php.net/manual/en/worker.isterminated.php
     * @return bool A boolean indication of state
     */
    final public function isTerminated() {}

    /**
     * Tell if a Worker is executing Stackables
     *
     * @link http://www.php.net/manual/en/worker.isworking.php
     * @return bool A boolean indication of state
     */
    final public function isWorking() {}

    /**
     * Kills the referenced worker, dangerously !
     *
     * @link http://www.php.net/manual/en/worker.kill.php
     */
    final public function kill() {}

    /**
     * Merges data into the current object
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag, by default true
     *
     * @link http://www.php.net/manual/en/worker.merge.php
     * @return bool A boolean indication of success
     */
    final public function merge($from, $overwrite = true) {}

    /**
     * Pops an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/worker.pop.php
     * @return mixed The last item from the objects properties table
     */
    final public function pop() {}

    /**
     * Runs worker
     *
     * The run method should prepare the Workers members ( and resources ) -
     * Stackables have access to the Worker and it's methods/members/resources during execution
     *
     * @link http://www.php.net/manual/en/tworker.run.php
     * @return void The methods return value, if used, will be ignored
     */
    abstract public function run();

    /**
     * Shifts an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/worker.shift.php
     * @return mixed The first item from the objects properties table
     */
    final public function shift() {}

    /**
     * Shuts down the Worker after executing all the Stackables previously stacked
     *
     * @link http://www.php.net/manual/en/worker.shutdown.php
     * @return bool A boolean indication of success
     */
    final public function shutdown() {}

    /**
     * Appends the referenced Stackable to the stack of the referenced Worker
     *
     * @param Stackable $work An object of type Stackable to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.stack.php
     * @return int The new length of the stack
     */
    final public function stack(Stackable $work) {}

    /**
     * Will start a new Thread, executing Worker::run and then waiting for Stackables
     *
     * @param int $options An optional mask of inheritance constants, by default PTHREADS_INHERIT_ALL
     *
     * @link http://www.php.net/manual/en/worker.start.php
     * @return bool A boolean indication of success
     */
    final public function start($options = PTHREADS_INHERIT_ALL) {}

    /**
     * Removes the referenced Stackable ( or all Stackables if parameter is null ) from stack of the referenced Worker
     *
     * @param Stackable $work An object of type Stackable
     *
     * @link http://www.php.net/manual/en/worker.unstack.php
     * @return int The new length of the stack
     */
    final public function unstack(Stackable $work = null) {}
}

/**
 * Stackable class
 *
 * Stackables are tasks that are executed by Worker threads.
 * You can synchronize with, read, and write Stackable objects before, after and during their execution.
 *
 * @link http://www.php.net/manual/en/class.stackable.php
 */
abstract class Stackable
{
    /**
     * Fetches a chunk of the objects properties table of the given size
     *
     * @param int $size The number of items to fetch
     *
     * @link http://www.php.net/manual/en/stackable.chunk.php
     * @return array An array of items from the objects member table
     */
    final public function chunk($size) {}

    /**
     * Will return the size of the properties table
     *
     * @return int
     */
    final public function count() {}

    /**
     * Will return information concerning the location of the termination to aid debugging
     *
     * @return array|bool
     */
    final public function getTerminationInfo() {}

    /**
     * A Stackable is running when a Worker Thread is executing it
     *
     * @link http://www.php.net/manual/en/stackable.isrunning.php
     * @return bool A boolean indication of state
     */
    final public function isRunning() {}

    /**
     * Tell if the referenced Stackable exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @link http://www.php.net/manual/en/stackable.isterminated.php
     * @return bool A boolean indication of state
     */
    final public function isTerminated() {}

    /**
     * Tell if the referenced Stackable is waiting for notification
     *
     * @link http://www.php.net/manual/en/stackable.iswaiting.php
     * @return bool A boolean indication of state
     */
    final public function isWaiting() {}

    /**
     * Lock the referenced objects storage for the calling context
     *
     * @link http://www.php.net/manual/en/stackable.lock.php
     * @return bool A boolean indication of state
     */
    final public function lock() {}

    /**
     * Merges data into the current object
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag, by default true
     *
     * @link http://www.php.net/manual/en/stackable.merge.php
     * @return bool A boolean indication of success
     */
    final public function merge($from, $overwrite = true) {}

    /**
     * Send notification to the referenced Stackable that is waiting
     *
     * @link http://www.php.net/manual/en/stackable.notify.php
     * @return bool A boolean indication of success
     */
    final public function notify() {}

    /**
     * Pops an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/stackable.pop.php
     * @return mixed The last item from the objects properties table
     */
    final public function pop() {}

    /**
     * The run method of a Stackable is executed by the Worker Thread
     *
     * @link http://www.php.net/manual/en/stackable.run.php
     * @return void The methods return value, if used, will be ignored
     */
    abstract public function run();

    /**
     * Shifts an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/stackable.shift.php
     * @return mixed The first item from the objects properties table
     */
    final public function shift() {}

    /**
     * Executes the block while retaining the synchronization lock for the current context.
     *
     * @param \Closure $function The block of code to execute
     * @param mixed $args... Variable length list of arguments to use as function arguments to the block
     *
     * @link http://www.php.net/manual/en/stackable.synchronized.php
     * @return mixed The return value from the block
     */
    final public function synchronized(\Closure $function, $args = null) {}

    /**
     * Unlock the referenced objects storage for the calling context
     *
     * @link http://www.php.net/manual/en/stackable.unlock.php
     * @return bool A boolean indication of success
     */
    final public function unlock() {}

    /**
     * Waits for notification from the Stackable
     *
     * @param int $timeout An optional timeout in millionths of a second
     *
     * @link http://www.php.net/manual/en/stackable.wait.php
     * @return bool A boolean indication of success
     */
    final public function wait($timeout) {}
}

/**
 * Mutex class
 *
 * The static methods contained in the Mutex class provide direct access to Posix Mutex functionality.
 *
 * @link http://www.php.net/manual/en/class.mutex.php
 */
class Mutex
{

    /**
     * Create, and optionally lock a new Mutex for the caller
     *
     * @param bool $lock Setting lock to true will lock the Mutex for the caller before returning the handle
     *
     * @link http://www.php.net/manual/en/mutex.create.php
     * @return int A newly created and optionally locked Mutex handle
     */
    final public static function create($lock = false) {}

    /**
     * Destroy mutex
     *
     * Destroying Mutex handles must be carried out explicitly by the programmer when they are
     * finished with the Mutex handle.
     *
     * @param int $mutex A handle returned by a previous call to Mutex::create().
     *
     * @link http://www.php.net/manual/en/mutex.destroy.php
     * @return bool A boolean indication of success
     */
    final public static function destroy($mutex) {}

    /**
     * Attempt to lock the Mutex for the caller.
     *
     * An attempt to lock a Mutex owned (locked) by another Thread will result in blocking.
     *
     * @param int $mutex A handle returned by a previous call to Mutex::create().
     *
     * @link http://www.php.net/manual/en/mutex.lock.php
     * @return bool A boolean indication of success
     */
    final public static function lock($mutex) {}

    /**
     * Attempt to lock the Mutex for the caller without blocking if the Mutex is owned (locked) by another Thread.
     *
     * @param int $mutex A handle returned by a previous call to Mutex::create().
     *
     * @link http://www.php.net/manual/en/mutex.trylock.php
     * @return bool A boolean indication of success
     */
    final public static function trylock($mutex) {}

    /**
     * Release mutex
     *
     * Attempts to unlock the Mutex for the caller, optionally destroying the Mutex handle.
     * The calling thread should own the Mutex at the time of the call.
     *
     * @param int $mutex A handle returned by a previous call to Mutex::create().
     * @param bool $destroy When true pthreads will destroy the Mutex after a successful unlock.
     *
     * @link http://www.php.net/manual/en/mutex.unlock.php
     * @return bool A boolean indication of success
     */
    final public static function unlock($mutex, $destroy = false) {}
}

/**
 * Condition class
 *
 * The static methods contained in the Cond class provide direct access to Posix Condition Variables.
 *
 * @link http://www.php.net/manual/en/class.cond.php
 */
class Cond
{
    /**
     * Broadcast to all Threads blocking on a call to Cond::wait().
     *
     * @param int $condition A handle to a Condition Variable returned by a previous call to Cond::create()
     *
     * @link http://www.php.net/manual/en/cond.broadcast.php
     * @return bool A boolean indication of success
     */
    final public static function broadcast($condition) {}

    /**
     * Creates a new Condition Variable for the caller.
     *
     * @link http://www.php.net/manual/en/cond.create.php
     * @return int A handle to a Condition Variable
     */
    final public static function create() {}

    /**
     * Destroy a condition
     *
     * Destroying Condition Variable handles must be carried out explicitly by the programmer when they are
     * finished with the Condition Variable.
     * No Threads should be blocking on a call to Cond::wait() when the call to Cond::destroy() takes place.
     *
     * @param int $condition A handle to a Condition Variable returned by a previous call to Cond::create()
     *
     * @link http://www.php.net/manual/en/cond.destroy.php
     * @return bool A boolean indication of success
     */
    final public static function destroy($condition) {}

    /**
     * Signal a Condition
     *
     * @param int $condition A handle to a Condition Variable returned by a previous call to Cond::create()
     *
     * @link http://www.php.net/manual/en/cond.signal.php
     * @return bool A boolean indication of success
     */
    final public static function signal($condition) {}

    /**
     * Wait for a signal on a Condition Variable, optionally specifying a timeout to limit waiting time.
     *
     * @param int $condition A handle to a Condition Variable returned by a previous call to Cond::create()
     * @param int $mutex A handle returned by a previous call to Mutex::create() and owned (locked) by the caller.
     * @param int $timeout An optional timeout, in microseconds
     *
     * @return bool A boolean indication of success
     */
    final public static function wait($condition, $mutex, $timeout = null) {}
}

/**
 * Pool class
 *
 * A Pool is a container for, and controller of, a number of Worker threads, the number of threads can be adjusted
 * during execution, additionally the Pool provides an easy mechanism to maintain and collect references in the
 * proper way.
 *
 * @link http://www.php.net/manual/en/class.pool.php
 */
class Pool
{
    /**
     * The maximum number of Worker threads allowed in this Pool
     *
     * @var integer
     */
    protected $size;

    /**
     * The name of the Worker class for this Pool
     *
     * @var string
     */
    protected $class;

    /**
     * The array of Worker threads for this Pool
     *
     * @var array|Worker[]
     */
    protected $workers;

    /**
     * The array of Stackables submitted to this Pool for execution
     *
     * @var array|Stackable[]
     */
    protected $work;

    /**
     * The constructor arguments to be passed by this Pool to new Workers upon construction
     *
     * @var array
     */
    protected $ctor;

    /**
     * The numeric identifier for the last Worker used by this Pool
     *
     * @var integer
     */
    protected $last;

    /**
     * Construct a new Pool of Workers
     *
     * @param integer $size The maximum number of Workers this Pool can create
     * @param string $class The class for new Workers
     * @param array $ctor An array of arguments to be passed to new Workers
     *
     * @link http://www.php.net/manual/en/pool.__construct.php
     */
    public function __construct($size, $class, array $ctor = array()) {}

    /**
     * Shuts down all Workers, and collect all Stackables, finally destroys the Pool
     *
     * @link http://www.php.net/manual/en/pool.__destruct.php
     */
    public function __destruct() {}

    /**
     * Collect references to completed tasks
     *
     * Allows the Pool to collect references determined to be garbage by the given collector
     *
     * @param callable $collector
     *
     * @link http://www.php.net/manual/en/pool.collect.php
     */
    public function collect(callable $collector) {}

    /**
     * Resize the Pool
     *
     * @param integer $size The maximum number of Workers this Pool can create
     *
     * @link http://www.php.net/manual/en/pool.resize.php
     */
    public function resize($size) {}

    /**
     * Shutdown all Workers in this Pool
     *
     * @link http://www.php.net/manual/en/pool.shutdown.php
     */
    public function shutdown() {}

    /**
     * Submit the task to the next Worker in the Pool
     *
     * @param Stackable $task The task for execution
     *
     * @return int the length of the stack on the selected Worker
     */
    public function submit(Stackable $task) {}

    /**
     * Submit the task to the specific Worker in the Pool
     *
     * @param Worker $worker Worker instance
     * @param Stackable $task The task for execution
     *
     * @return int the length of the stack on the selected Worker
     */
    public function submitTo(Worker $worker, Stackable $task) {}
}
