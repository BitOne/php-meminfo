<?php

class MyClassA {};

$a = array();
meminfo_symbol_table(fopen('php://stdout','w'));
$b = true;
meminfo_symbol_table(fopen('php://stdout','w'));
$s = "My b variable";
meminfo_symbol_table(fopen('php://stdout','w'));
$d = (125.3);
meminfo_symbol_table(fopen('php://stdout','w'));

$o = new MyClassA();
meminfo_symbol_table(fopen('php://stdout','w'));

function myFunction()
{
    $a = array();
    meminfo_symbol_table(fopen('php://stdout','w'));
    $GLOBALS['a'] = $a;
    meminfo_symbol_table(fopen('php://stdout','w'));
}
myFunction();
meminfo_symbol_table(fopen('php://stdout','w'));

meminfo_symbol_table(fopen('php://stdout','w'));
