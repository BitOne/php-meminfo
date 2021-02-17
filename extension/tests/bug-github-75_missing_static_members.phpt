--TEST--
Check that a class static member is accounted by meminfo
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    class MyClass {
        public static $myStaticMember;
    }

    MyClass::$myStaticMember = new StdClass();

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $objectsCount = 0;
    foreach($meminfoData['items'] as $item) {
        if ($item['type'] === 'object') {
            $objectsCount++;
        }
    }

    echo "Objects count found by meminfo:".$objectsCount."\n";

?>
--EXPECT--
Objects count found by meminfo:1
