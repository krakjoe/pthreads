<?php
/**
 * pthreads extension stub file for code completion purposes
 *
 * @author Lisachenko Alexander <lisachenko.it@gmail.com>
 * @version 2.0.0
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
 * Threaded class
 *
 * Threaded objects form the basis of pthreads ability to execute user code in parallel;
 * they expose and include synchronization methods and various useful interfaces.
 *
 * Threaded objects, most importantly, provide implicit safety for the programmer;
 * all operations on the object scope are safe.
 *
 * @link http://www.php.net/manual/en/class.threaded.php
 * @since 2.0.0
 */
class Threaded implements Traversable, Countable, ArrayAccess, Collectable
{
    /**
     * Increments the object's reference count
     */
    public function addRef() {}

    /**
     * Fetches a chunk of the objects properties table of the given size
     *
     * @param int $size The number of items to fetch
     *
     * @link http://www.php.net/manual/en/threaded.chunk.php
     * @return array An array of items from the objects member table
     */
    public function chunk($size) {}

    /**
     * {@inheritdoc}
     */
    public function count() {}

    /**
     * Decrements the object's reference count
     */
    public function delRef() {}

    /**
     * Runtime extending of the Threaded class
     *
     * @param string $class The name of the class to extend Threaded
     * @return bool A boolean indication of success
     */
    public static function extend($class) {}

    /**
     * Gets the object's reference count
     *
     * @return int The object's reference count
     */
    public function getRefCount() {}

    /**
     * A default method for marking an object as ready to be destroyed
     *
     * @return bool(true) The referenced object can be destroyed
     */
    public function isGarbage() {}

    /**
     * Tell if the referenced object is executing
     *
     * @link http://www.php.net/manual/en/threaded.isrunning.php
     * @return bool A boolean indication of state
     */
    public function isRunning() {}

    /**
     * Tell if the referenced object exited, suffered fatal errors, or threw uncaught exceptions during execution
     *
     * @link http://www.php.net/manual/en/threaded.isterminated.php
     * @return bool A boolean indication of state
     */
    public function isTerminated() {}

    /**
     * Merges data into the current object
     *
     * @param mixed $from The data to merge
     * @param bool $overwrite Overwrite existing keys flag, by default true
     *
     * @link http://www.php.net/manual/en/threaded.merge.php
     * @return bool A boolean indication of success
     */
    public function merge($from, $overwrite = true) {}

    /**
     * Send notification to the referenced object
     *
     * @link http://www.php.net/manual/en/threaded.notify.php
     * @return bool A boolean indication of success
     */
    public function notify() {}

    /**
     * Send notification to one context waiting on the Threaded
     *
     * @return bool A boolean indication of success
     */
    public function notifyOne() {}

    /**
     * {@inheritdoc}
     */
    public function offsetGet($offset) {}

    /**
     * {@inheritdoc}
     */
    public function offsetSet($offset, $value) {}

    /**
     * {@inheritdoc}
     */
    public function offsetExists($offset) {}

    /**
     * {@inheritdoc}
     */
    public function offsetUnset($offset) {}

    /**
     * Pops an item from the objects property table
     *
     * @link http://www.php.net/manual/en/threaded.pop.php
     * @return mixed The last item from the objects properties table
     */
    public function pop() {}

    /**
     * The programmer should always implement the run method for objects that are intended for execution.
     *
     * @link http://www.php.net/manual/en/threaded.run.php
     * @return void The methods return value, if used, will be ignored
     */
    public function run() {}

    /**
     * Shifts an item from the objects properties table
     *
     * @link http://www.php.net/manual/en/threaded.shift.php
     * @return mixed The first item from the objects properties table
     */
    public function shift() {}

    /**
     * Executes the block while retaining the synchronization lock for the current context.
     *
     * @param \Closure $function The block of code to execute
     * @param mixed $args... Variable length list of arguments to use as function arguments to the block
     *
     * @link http://www.php.net/manual/en/threaded.synchronized.php
     * @return mixed The return value from the block
     */
    public function synchronized(\Closure $function, $args = null) {}

    /**
     * Waits for notification from the Stackable
     *
     * @param int $timeout An optional timeout in microseconds
     *
     * @link http://www.php.net/manual/en/threaded.wait.php
     * @return bool A boolean indication of success
     */
    public function wait($timeout = 0) {}
}

/**
 * Basic thread implementation
 *
 * An implementation of a Thread should extend this declaration, implementing the run method.
 * When the start method of that object is called, the run method code will be executed in separate Thread.
 *
 * @link http://www.php.net/manual/en/class.thread.php
 */
class Thread extends Threaded
{

    /**
     * Will return the identity of the Thread that created the referenced Thread
     *
     * @link http://www.php.net/manual/en/thread.getcreatorid.php
     * @return int A numeric identity
     */
    public function getCreatorId() {}

    /**
     * Will return the instance of currently executing thread
     *
     * @return static
     */
    public static function getCurrentThread() {}

    /**
     * Will return the identity of the currently executing thread
     *
     * @link http://www.php.net/manual/en/thread.getcurrentthreadid.php
     * @return int
     */
    public static function getCurrentThreadId() {}

    /**
     * Will return the identity of the referenced Thread
     *
     * @link http://www.php.net/manual/en/thread.getthreadid.php
     * @return int
     */
    public function getThreadId() {}

    /**
     * Tell if the referenced Thread has been joined by another context
     *
     * @link http://www.php.net/manual/en/thread.isjoined.php
     * @return bool A boolean indication of state
     */
    public function isJoined() {}

    /**
     * Tell if the referenced Thread has been started
     *
     * @link http://www.php.net/manual/en/thread.isstarted.php
     * @return bool A boolean indication of state
     */
    public function isStarted() {}

    /**
     * Causes the calling context to wait for the referenced Thread to finish executing
     *
     * @link http://www.php.net/manual/en/thread.join.php
     * @return bool A boolean indication of state
     */
    public function join() {}

    /**
     * Will start a new Thread to execute the implemented run method
     *
     * @param int $options An optional mask of inheritance constants, by default PTHREADS_INHERIT_ALL
     *
     * @link http://www.php.net/manual/en/thread.start.php
     * @return bool A boolean indication of success
     */
    public function start($options = PTHREADS_INHERIT_ALL) {}
}

/**
 * Worker
 *
 * Worker Threads have a persistent context, as such should be used over Threads in most cases.
 *
 * When a Worker is started, the run method will be executed, but the Thread will not leave until one
 * of the following conditions are met:
 *   - the Worker goes out of scope (no more references remain)
 *   - the programmer calls shutdown
 *   - the script dies
 * This means the programmer can reuse the context throughout execution; placing objects on the stack of
 * the Worker will cause the Worker to execute the stacked objects run method.
 *
 * @link http://www.php.net/manual/en/class.worker.php
 */
class Worker extends Thread
{

    /**
     * Executes the optional collector on each of the tasks, removing the task if true is returned
     *
     * @param callable $collector The collector to be executed upon each task
     * @return int The number of tasks left to be collected
     */
    public function collect($collector = null) {}

    /**
     * Executes the collector on the collectable object passed
     *
     * @param callable $collectable The collectable object to run the collector on
     * @return bool The referenced object can be destroyed
     */
    public function collector($collectable) {}

    /**
     * Returns the number of threaded tasks waiting to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.getstacked.php
     * @return int An integral value
     */
    public function getStacked() {}

    /**
     * Tell if the referenced Worker has been shutdown
     *
     * @link http://www.php.net/manual/en/worker.isshutdown.php
     * @return bool A boolean indication of state
     */
    public function isShutdown() {}

    /**
     * Shuts down the Worker after executing all the threaded tasks previously stacked
     *
     * @link http://www.php.net/manual/en/worker.shutdown.php
     * @return bool A boolean indication of success
     */
    public function shutdown() {}

    /**
     * Appends the referenced object to the stack of the referenced Worker
     *
     * @param Threaded $work Threaded object to be executed by the referenced Worker
     *
     * @link http://www.php.net/manual/en/worker.stack.php
     * @return int The new length of the stack
     */
    public function stack(Threaded &$work) {}

    /**
     * Removes the referenced object ( or all objects if parameter is null ) from stack of the referenced Worker
     *
     * @param Threaded $work Threaded object previously stacked onto Worker
     *
     * @link http://www.php.net/manual/en/worker.unstack.php
     * @return int The new length of the stack
     */
    public function unstack(Threaded &$work = null) {}
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
     * @param Threaded $task The task for execution
     *
     * @return int the identifier of the Worker executing the object
     */
    public function submit(Threaded $task) {}

    /**
     * Submit the task to the specific Worker in the Pool
     *
     * @param int $worker The worker for execution
     * @param Threaded $task The task for execution
     *
     * @return int the identifier of the Worker that accepted the object
     */
    public function submitTo($worker, Threaded $task) {}
}

/**
 * Collectable Class
 *
 * Garbage Collection interface for references to objects on Worker stacks
 *
 * @link http://www.php.net/manual/en/class.collectable.php
 */
interface Collectable
{
	/**
	 * Determine whether an object is ready to be destroyed
	 *
	 * @return bool Whether the referenced object can be destroyed
	 */
	public function isGarbage() : bool;
}

class Socket extends Threaded
{
	const AF_UNIX = 1;
	const AF_INET = 2;
	const AF_INET6 = 10;
	const SOCK_STREAM = 1;
	const SOCK_DGRAM = 2;
	const SOCK_RAW = 3;
	const SOCK_SEQPACKET = 5;
	const SOCK_RDM = 4;
	const SO_DEBUG = 1;
	const SO_REUSEADDR = 2;
	const SO_REUSEPORT = 15;
	const SO_KEEPALIVE = 9;
	const SO_DONTROUTE = 5;
	const SO_LINGER = 13;
	const SO_BROADCAST = 6;
	const SO_OOBINLINE = 10;
	const SO_SNDBUF = 7;
	const SO_RECBUF = 8;
	const SO_SNDLOWAT = 19;
	const SO_RCVLOWAT = 18;
	const SO_SNDTIMEO = 21;
	const SO_RCVTIMEO = 20;
	const SO_TYPE = 3;
	const SO_ERROR = 4;
	const SO_BINDTODEVICE = 25;
	const SOMAXCONN = 128;
	const TCP_NODELAY = 1;
	const SOL_SOCKET = 1;
	const SOL_TCP = 6;
	const SOL_UDP = 17;
	const MSG_OOB = 1;
	const MSG_WAITALL = 256;
	const MSG_CTRUNC = 8;
	const MSG_TRUNC = 32;
	const MSG_PEEK = 2;
	const MSG_DONTROUTE = 4;
	const MSG_EOR = 128;
	const MSG_CONFIRM = 2048;
	const MSG_ERRQUEUE = 8192;
	const MSG_NOSIGNAL = 16384;
	const MSG_MORE = 32768;
	const MSG_WAITFORONE = 65536;
	const MSG_CMSG_CLOEXEC = 1073741824;
	const EPERM = 1;
	const ENOENT = 2;
	const EINTR = 4;
	const EIO = 5;
	const ENXIO = 6;
	const E2BIG = 7;
	const EBADF = 9;
	const EAGAIN = 11;
	const ENOMEM = 12;
	const EACCES = 13;
	const EFAULT = 14;
	const ENOTBLK = 15;
	const EBUSY = 16;
	const EEXIST = 17;
	const EXDEV = 18;
	const ENODEV = 19;
	const ENOTDIR = 20;
	const EISDIR = 21;
	const EINVAL = 22;
	const ENFILE = 23;
	const EMFILE = 24;
	const ENOTTY = 25;
	const ENOSPC = 28;
	const ESPIPE = 29;
	const EROFS = 30;
	const EMLINK = 31;
	const EPIPE = 32;
	const ENAMETOOLONG = 36;
	const ENOLCK = 37;
	const ENOSYS = 38;
	const ENOTEMPTY = 39;
	const ELOOP = 40;
	const EWOULDBLOCK = 11;
	const ENOMSG = 42;
	const EIDRM = 43;
	const ECHRNG = 44;
	const EL2NSYNC = 45;
	const EL3HLT = 46;
	const EL3RST = 47;
	const ELNRNG = 48;
	const EUNATCH = 49;
	const ENOCSI = 50;
	const EL2HLT = 51;
	const EBADE = 52;
	const EBADR = 53;
	const EXFULL = 54;
	const ENOANO = 55;
	const EBADRQC = 56;
	const EBADSLT = 57;
	const ENOSTR = 60;
	const ENODATA = 61;
	const ETIME = 62;
	const ENOSR = 63;
	const ENONET = 64;
	const EREMOTE = 66;
	const ENOLINK = 67;
	const EADV = 68;
	const ESRMNT = 69;
	const ECOMM = 70;
	const EPROTO = 71;
	const EMULTIHOP = 72;
	const EBADMSG = 74;
	const ENOTUNIQ = 76;
	const EBADFD = 77;
	const EREMCHG = 78;
	const ERESTART = 85;
	const ESTRPIPE = 86;
	const EUSERS = 87;
	const ENOTSOCK = 88;
	const EDESTADDRREQ = 89;
	const EMSGSIZE = 90;
	const EPROTOTYPE = 91;
	const ENOPROTOOPT = 92;
	const EPROTONOSUPPORT = 93;
	const ESOCKTNOSUPPORT = 94;
	const EOPNOTSUPP = 95;
	const EPFNOSUPPORT = 96;
	const EAFNOSUPPORT = 97;
	const EADDRINUSE = 98;
	const EADDRNOTAVAIL = 99;
	const ENETDOWN = 100;
	const ENETUNREACH = 101;
	const ENETRESET = 102;
	const ECONNABORTED = 103;
	const ECONNRESET = 104;
	const ENOBUFS = 105;
	const EISCONN = 106;
	const ENOTCONN = 107;
	const ESHUTDOWN = 108;
	const ETOOMANYREFS = 109;
	const ETIMEDOUT = 110;
	const ECONNREFUSED = 111;
	const EHOSTDOWN = 112;
	const EHOSTUNREACH = 113;
	const EALREADY = 114;
	const EINPROGRESS = 115;
	const EISNAM = 120;
	const EREMOTEIO = 121;
	const EDQUOT = 122;
	const ENOMEDIUM = 123;
	const EMEDIUMTYPE = 124;
	
	
	public function __construct(int $domain = AF_INET, int $type = SOCK_STREAM, int $protocol = 0) {}
	
	public function setOption(int $level, int $name, int $value):bool {}
	
	public function getOption(int $level, int $name):int {}
	
	public function bind(string $host, int $port):bool {}
	
	public function listen(int $backlog):bool {}
	
	public static function accept(string $class = self::class):Socket {}
	
	public function connect(string $host, int $port):bool {}
	
	public function select(array &$read, array &$write, array &$except, int $sec, int $usec = 0):int {}
	
	public function read(int $length = 0, int $flags = 0):string {}
	
	public function write(string $buffer = null, int $length = 0):string {}
	
	public function send(string $buffer = null, int $length = 0, int $flags = 0):string {}
	
	public function setBlocking(bool $blocking = false):bool {}
	
	public function getPeerName(bool $port = true):array {}
	
	public function getSockName(bool $port = true):array {}
	
	public function close():bool {}
}
