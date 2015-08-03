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

    echo "Size of myDouble\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myDouble);
    echo "\n";

    echo "Length of myString:";
    echo strlen($myString)."\n";

    echo "Size of myString\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myString);

    echo "\n";

    echo "Size of myObject\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myObject);

    echo "Size of myArray\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myArray);

    echo "Size of myHash\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myHash);

    echo "Size of myHash2\n";
    meminfo_element_size(fopen('php://stdout', 'w'), $myHash2);

