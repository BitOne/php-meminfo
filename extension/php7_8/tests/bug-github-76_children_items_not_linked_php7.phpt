--TEST--
Check that all children items are properly linked through their identifiers
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--

<?php
    $dump = fopen('php://memory', 'rw');

    class MyClass {
        public $myDeclaredVar;
    }

    $myObject = new MyClass();
    $myObject->myDeclaredVar = "My declared var content";
    $myObject->myUndeclaredVar = "My undeclared content";
    $myObject->childObject = new stdClass();

    $myArray = [
        "objectInArray" => new StdClass(),
        "arrayInArray" => ["foo" => "bar"]
    ];

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    foreach($meminfoData['items'] as $item) {
        if (isset($item['children'])) {
            foreach($item['children'] as $child) {
                if (!isset($meminfoData['items'][$child])) {
                    echo "Child $child not found!\n";
                }
            }
        }
    }
?>
--EXPECT--
