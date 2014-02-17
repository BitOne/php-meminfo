<?php

class MyClassA {
}

class MyClassB {
}

class MyClassC {
}

$objectA = new MyClassA();
$objectB = new MyClassB();
$objectA2 = $objectA;
$objectA3 = $objectA;

echo " * zval references have been copied\n";
meminfo_objects_list(fopen('php://stdout', 'w'));

$objectARef1 = &$objectA;
$objectARef2 = $objectA;

echo " * objectA turned into object reference\n";
meminfo_objects_list(fopen('php://stdout', 'w'));
