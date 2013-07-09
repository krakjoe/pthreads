<?php

/*
 * Advancements in worker threads and stacking ...
 * You can now stack anything that extends stackable ...
 * so your tasks can be completely different.
 * You can synchronize with individual stackables being executed in a worker ...
 * Worker has a specific definition rather than being built into every Thread as earlier versions were ...
 */

class Work extends Stackable
{

    public function __construct($data)
    {
        $this->local = $data;
    }

    /**
     * {@inheritdoc}
     * @see Stackable::run()
     */
    public function run()
    {
        // We import the worker here to execute it's methods
        if ($this->worker) {
            printf("Running %s in %s\n", __CLASS__, $this->worker->getName());
            $this->worker->addAttempt();
            $this->worker->addData($this->getData());
        } else {
            printf("failed to get worker something is wrong ...\n");
        }
        print_r($this);
    }

    protected function getData()
    {
        return $this->local;
    }
}

class ExampleWorker extends Worker
{

    public function __construct($name)
    {
        $this->setSetup(false);
        $this->setName($name);
        $this->setData(array());
    }

    /**
     * The run method should just prepare the environment for the work that is coming ...
     * @see Worker::run()
     */
    public function run()
    {
        if (!$this->setup) {
            $this->setSetup(true);
            $this->setName(sprintf("%s (%lu)", __CLASS__, $this->getThreadId()));
        } else {
            printf("%s is being run ... why ??\n", $this->getName());
        }
        printf("Setup %s Complete\n", $this->getName());
    }

    public function setSetup($setup)
    {
        $this->setup = $setup;
    }

    public function getName()
    {
        return $this->name;
    }

    public function setName($name)
    {
        $this->name = $name;
    }

    /**
     * About protected methods:
     * Even if a public or private method read/writes member data the
     * reads and writes will be safe.
     * So in this particular example they arent strictly required for
     * the objects to function as intended.
     * Normally methods will read and manipulate data in a more complicated manner than in this example.
     * If a method reads and writes thread data it will often be a good idea to protect the method.
     */
    public function addAttempt()
    {
        $this->attempts++;
    }

    public function getAttempts()
    {
        return $this->attempts;
    }

    /**
     * this method overwrites thread data
     */
    protected function setData($data)
    {
        $this->data = $data;
    }

    /**
     * this method reads and writes thread data
     */
    protected function addData($data)
    {
        $this->data = array_merge($this->data, $data);
    }

    /**
     * this method reads data but makes no changes, doesnt usually happen in the real world
     */
    protected function getData()
    {
        return $this->data;
    }
}

$worker = new ExampleWorker("My Worker Thread");
$work = array();
for ($i = 0; $i < 10; $i++) {
    printf("Stacking: %d/%d\n", $i, $worker->stack($work[] = new Work(array(rand() * 100))));
}
$worker->start();
usleep(10000);
while ($i++ < 20) {
    printf("Stacking: %d/%d\n", $i, $worker->stack($work[] = new Work(array(rand() * 100))));
}
$worker->shutdown();
printf("Result: %d/%d\n", $worker->getAttempts(), $worker->getStacked());
print_r($worker->getData());
