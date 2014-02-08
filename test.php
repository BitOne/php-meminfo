<?php

$toto = "5";

$titi = array();

class TotObject {
    public $pubVarTiti = "This is vartiti from totobject";

    protected $protTestArray1;

    private $privateTestArray2;
}

class TitObject {
    var $varTiti = "this is vartiti from titobject";
    var $varToti = "this is vartoti from titobject";

}

class ThirdClass {
    var $varThirdClass = "This is third class";
}

$lolo = new TotObject();

function test1() {
    
    $test1_variable = 1.5;
    $testobj1 = new TotObject();
    $testobj1->testArray1 = array();
    $testobj1->testArray2 = array();
    $testobj1->testArray1[] = array('Test1234567890', array (array (array('1234567890'),'1234567890')));
    $testobj1->testArray1[] =& $testobj1->testArray2;
    $testobj1->testArray2[] = $testobj1->testArray1;
    $testobj1->varTiti = new ThirdClass();
    $lolo = new TitObject();
    $lolo->varTiti = $testobj1;
    $lolo->varToti = array(1 => "first", 2 => "second");
    meminfo_objects_list();
   // test2();

}

function test2() {
    $test2_variable = "test2";
    test3();
}

function test3() {
    $lilo = "testlilo";
    $lili = new TotObject();
    $titi = $lili;
    $tata = new TotObject();

    meminfo_objects_list();
}

meminfo_structs_size();

test1();

