--TEST--
Check dumping class private field
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    class MyClass {
        public $pub_str;
        private $priv_str;

        function __construct(string $pub, string $priv) {
            $this->pub_str = $pub;
            $this->priv_str = $priv;
        }
    }

    $my_class_instance = new MyClass('public', 'private');

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $myArrayDump = [];

    foreach ($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && $item['symbol_name'] == 'my_class_instance') {
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
Symbol: my_class_instance
  Frame:<GLOBAL>
  Type:object
  Is root:1
  Children:
    Key: pub_str
      Type:string
      Is root:
    Key: priv_str
      Type:string
      Is root:
