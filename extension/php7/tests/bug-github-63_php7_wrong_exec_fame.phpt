--TEST--
Check that we have the right frame name on PHP 7
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');
    $myVariableInGlobalScope = 'scope';

    function functionLevel1() {
        $myVariableInsideAFunction1 = 'level_1';
        functionLevel2();
    }

    function functionLevel2() {
        $myVariableInsideAFunction2 = 'level_2';
        functionLevel3();
    }

    function functionLevel3() {
        global $dump;
        $myVariableInsideAFunction3 = 'level_3';

        meminfo_dump($dump);
    }

    functionLevel1();

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $symbolNames = ['myVariableInsideAFunction1', 'myVariableInsideAFunction2', 'myVariableInsideAFunction3', 'myVariableInGlobalScope'];
    foreach($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && in_array($item['symbol_name'], $symbolNames)) {
            echo 'Frame for '.$item['symbol_name'].': '.$item['frame']."\n";
        }
    }

?>
--EXPECT--
Frame for myVariableInsideAFunction3: functionLevel3()
Frame for myVariableInsideAFunction2: functionLevel2()
Frame for myVariableInsideAFunction1: functionLevel1()
Frame for myVariableInGlobalScope: <GLOBAL>
