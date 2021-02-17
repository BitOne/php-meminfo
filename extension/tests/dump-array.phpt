--TEST--
Check array dump from memory
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    $myArray = [];
    $myArray["my string"] = "My nice string";
    $myArray["my int"] = 42;
    $myArray["my float"] = 3.14;
    $myArray["my boolean"] = true;
    $myArray["my null"] = null;
    $myArray["my resource"] = fopen('/dev/null', 'w');

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $myArrayDump = [];

    foreach ($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && $item['symbol_name'] == 'myArray') {
            $myArrayDump = $item;
        }
    }

    echo "Symbol: ".$myArrayDump['symbol_name']."\n";
    echo "  Frame:".$myArrayDump['frame']."\n";
    echo "  Type:".$myArrayDump['type']."\n";
    echo "  Is root:".$myArrayDump['is_root']."\n";

    echo "  Children:\n";

    foreach ($myArrayDump['children'] as $key => $child) {
        echo "    Key: ".$key."\n";
        echo "      Type:";
        $type = $meminfoData['items'][$child]['type'];
        if ('int' === $type) {
            echo "integer";
        } elseif ('bool' === $type) {
            echo "boolean";
        } else {
            echo $type;
        }
        echo "\n";
        echo "      Is root:".$meminfoData['items'][$child]['is_root']."\n";
    }
?>
--EXPECT--
Symbol: myArray
  Frame:<GLOBAL>
  Type:array
  Is root:1
  Children:
    Key: my string
      Type:string
      Is root:
    Key: my int
      Type:integer
      Is root:
    Key: my float
      Type:float
      Is root:
    Key: my boolean
      Type:boolean
      Is root:
    Key: my null
      Type:null
      Is root:
    Key: my resource
      Type:resource
      Is root:
