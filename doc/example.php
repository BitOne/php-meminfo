<?php

class MyClassA {
    private $myObjectName;

    public function __construct($name)
    {
        $this->myObjectName = $name;
    }
};

$myString = "My very nice string";
$myFloat = 3.14;
$myInt = 42;
$myNull = null;

$myRootArray = [];

for($i = 0; $i < 100; $i++) {
    $myRootArray['first level']['second level'][] = new MyClassA('My nice object name '.$i);
};

meminfo_dump(fopen('/tmp/php_mem_dump.json','w'));


