<?php


class MyClassA {
}

$a = new MyClassA();
$a->v = 5;

function myFunction() {
    $myDate = new \DateTime();

    echo "\n* Objects summary in function call with inside DateTime object\n";
    meminfo_objects_list(fopen('php://stdout', 'w'));
    meminfo_objects_summary(fopen('php://stdout', 'w'));
}

myFunction();

