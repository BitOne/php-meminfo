<?php

class MyClassA {
}

class MyClassB {
    public $otherObject;
    public $otherObject2;
}

class MyClassC {
}

$objectA = new MyClassA();
$objectAref2 =& $objectA;
$objectB = new MyClassB();
$objectB->otherObject = $objectA;
$objectB->otherObject2 = $objectA;


meminfo_objects_list(fopen('php://stdout', 'w'));

