<?php

    function testObject($myObject)
    {
        $varInsideTestObject = "My super var";
        $anotherVarInsideFunction  = 42;

        secondFunction();
    }

    function secondFunction()
    {
        $in2ndFunction1 = "test1";
        $in2ndFunction2 = "test2";
        $arrayIn2Function = ['Test de array', 1, 2, 3];
        meminfo_info_dump(fopen('php://stdout', 'w'));

    }

    $myDouble = 0.1;
    $myString = "My really nice string 39 characters long";

    $myObject = new StdClass();
    $myObject->myDouble = $myDouble;
    $myObject->myString = $myString;

    $myObject2 = new StdClass();

    $myObjectRef = $myObject;

    $myArray = [];
    $myArray[] = "TEST";
    $myArray[] = $myDouble;
    $myArray[] = $myString;
    $myArray[] = $myArray;

    $myHash = [];
    $myHash["entry_one"] = $myString;
    $myHash["entry_two"] = $myDouble;
    $myHash["entry_\"three\""] = "It has a key with double quotes";

    $myHash2 = [];

    testObject($myObject);
