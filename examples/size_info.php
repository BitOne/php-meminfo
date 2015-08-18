<?php

    $myDouble = 0.1;
    $myString = "My nice string";

    $myObject = new StdClass();
    $myObject->myDouble = $myDouble;
    $myObject->myString = $myString;

    $myArray = [];
    $myArray[] = "TEST";
    $myArray[] = $myDouble;
    $myArray[] = $myString;
    $myArray[] = $myArray;

    $myHash = [];
    $myHash["entry_one"] = $myString;
    $myHash["entry_two"] = $myDouble;

    $myHash2 = [];

    meminfo_size_info(fopen('php://stdout', 'w'));

