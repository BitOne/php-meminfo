<?php

class MyClassA {
}

class MyClassB {
}

class MyClassC {
}

$objectA = new MyClassA();
$objectB = new MyClassB();

echo "* Objects summary after instanciating objects A and B\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

$objectsC = array();

for ($i = 0; $i < 5; $i++) {
    $objectsC[] = new MyClassC();
}

echo "\n* Objects summary after instanciating 5 objects C\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

unset($objectA);
unset($objectsC[0]);
unset($objectsC[1]);

echo "\n* Objects summary after unset on some objects\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

$myClosure = function() {
    $a = 1;
};

echo "\n* Objects summary after instantiating an anonymous function\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

$objectBClone = clone $objectB;

echo "\n* Objects summary after cloning objectB\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

function myFunction() {
    $myDate = new \DateTime();

    echo "\n* Objects summary in function call with inside DateTime object\n";
    meminfo_objects_summary(fopen('php://stdout', 'w'));
}

myFunction();

echo "\n* Objects summary after function call\n";
meminfo_objects_summary(fopen('php://stdout', 'w'));

