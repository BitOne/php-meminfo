<?php

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

    meminfo_size_info(fopen('php://stdout', 'w'));
