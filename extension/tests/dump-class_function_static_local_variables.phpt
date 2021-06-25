--TEST--
Check that static variables inside a class member function are accounted for
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
    $dump = fopen('php://memory', 'rw');

    class MyClass {
        public function myMethod() {
            static $staticLocalVar;
            
            if (!isset($staticLocalVar)) {
                $staticLocalVar = 'one time load';
            }
            
            return $staticLocalVar;
        }
    }
    
    (new MyClass)->myMethod();

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    $myArrayDump = [];

    foreach ($meminfoData['items'] as $item) {
        if (isset($item['symbol_name']) && $item['symbol_name'] == '$staticLocalVar') {
            echo "Symbol: " . $item['symbol_name'] . "\n";
            echo "  Frame: " . $item['frame'] . "\n";
            echo "  Type: " . $item['type'] . "\n";
            echo "  Is root: " . $item['is_root'] . "\n";
        }
    }
?>
--EXPECT--
Symbol: $staticLocalVar
  Frame: <STATIC_VARIABLE(MyClass::myMethod)>
  Type: string
  Is root: 1
