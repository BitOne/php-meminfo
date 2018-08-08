--TEST--
Check that a scalar with multiple references is detected only once by meminfo.
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    $a = "foo";

    $b =& $a;

    $c = strlen($b);

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $objectsCount = 0;
    $bReferenceFound = false;

    foreach($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && $item['symbol_name'] === 'b') {
            $bReferenceFound = true;
        }
    }

    if ($bReferenceFound) {
        echo "'b' reference found although it shouldn't be accounted.\n";
    } else {
        echo "Reference 'b' not found.\n";
    }
?>
--EXPECT--
Reference 'b' not found.
