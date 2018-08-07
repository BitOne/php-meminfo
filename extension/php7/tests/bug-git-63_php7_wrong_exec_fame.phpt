--TEST--
Check that we have the right frame name on PHP 7
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    function myTestFunction() {
        global $dump;
        $myVariableInsideAFunction = 'test';

        meminfo_dump($dump);
    }

    myTestFunction();

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    foreach($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && $item['symbol_name'] === 'myVariableInsideAFunction') {
            echo 'Frame for '.$item['symbol_name'].': '.$item['frame']."\n";
        }
    }

?>
--EXPECT--
Frame for myVariableInsideAFunction: myTestFunction()
