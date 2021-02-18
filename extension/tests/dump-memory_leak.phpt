--TEST--
meminfo_dump check there's no memory leak
--FILE--
<?php
    require dirname(__FILE__) . '/fixtures/books.php';

    $rFilePointer = fopen('/dev/null', 'w');

    $arrayTest = [
        'itemBool' => true,
        'itemInteger' => 23,
        'itemDoubles' => 1.2e3,
        'itemNull' => null,
        'itemString' => 'hello',
        'itemObject' => $books,
        'itemArray' => (array) $books,
        'itemResource' => $rFilePointer,
    ];
    $attemptCount = 1000;
    gc_collect_cycles();
    $startM = memory_get_usage(true);
    while ($attemptCount-- > 0) {
        meminfo_dump($rFilePointer);
    }
    fclose($rFilePointer);
    gc_collect_cycles();
    $endM = memory_get_usage(true);
    if ($endM - $startM === 0) {
        echo 'Memory leak test was successful';
    } else {
        echo "Memory leak test was failed\n";
        printf("Memory leak:%s bytes",$endM - $startM);
    }
?>
--EXPECT--
Memory leak test was successful
