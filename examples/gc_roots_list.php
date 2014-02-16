<?php

class MyClassA {};

$a = array();
$b = true;
$s = "My b variable";
$d = (125.3);

$o = new MyClassA();

echo "* After instantiation\n";
meminfo_gc_roots_list(fopen('php://stdout','w'));

function myFunction()
{
    $a = array();
    echo "* After instanciation in function\n";
    meminfo_gc_roots_list(fopen('php://stdout','w'));
    $GLOBALS['a'] = $a;
    echo "* After assigning to GLOBALS\n";
    meminfo_gc_roots_list(fopen('php://stdout','w'));
}
myFunction();
echo "* After function call\n";
meminfo_gc_roots_list(fopen('php://stdout','w'));

