--TEST--
Check scalar dump from memory
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    $myString = "My nice string";
    $myInt = 42;
    $myFloat = 3.14;
    $myBoolean = true;
    $myNull = null;
    $myStream = fopen('/dev/null', 'w');

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    fclose($myStream);

    $scalars = [];

    foreach ($meminfoData['items'] as $item) {
        if (isset($item['symbol_name'])) {
            switch ($item['symbol_name']) {
                case 'myString':
                    $scalars[0] = $item;
                    break;
                case 'myInt':
                    $scalars[1] = $item;
                    break;
                case 'myFloat':
                    $scalars[2] = $item;
                    break;
                case 'myBoolean':
                    $scalars[3] = $item;
                    break;
                case 'myNull':
                    $scalars[4] = $item;
                    break;
                case 'myStream':
                    $scalars[5] = $item;
                    break;
            }
        }
    }

    foreach ($scalars as $scalar) {
        echo "Symbol: ".$scalar['symbol_name']."\n";
        echo "  Frame: ".$scalar['frame']."\n";
        echo "  Type: ".$scalar['type']."\n";
        echo "  Is root: ".$scalar['is_root']."\n";
    }
?>
--EXPECT--
Symbol: myString
  Frame: <GLOBAL>
  Type: string
  Is root: 1
Symbol: myInt
  Frame: <GLOBAL>
  Type: integer
  Is root: 1
Symbol: myFloat
  Frame: <GLOBAL>
  Type: double
  Is root: 1
Symbol: myBoolean
  Frame: <GLOBAL>
  Type: boolean
  Is root: 1
Symbol: myNull
  Frame: <GLOBAL>
  Type: null
  Is root: 1
Symbol: myStream
  Frame: <GLOBAL>
  Type: resource
  Is root: 1
