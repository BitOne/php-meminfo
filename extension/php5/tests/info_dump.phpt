--TEST--
meminfo_objects_list with some objects
--FILE--
<?php
    $docTest = new DOMDocument();
    $docTest->load(__DIR__.'/fixture/book.xml');

    $rFilePointer = fopen('php://memory', 'rw');

    $arrayTest = [
        'itemBool' => true,
        'itemInteger' => 23,
        'itemDoubles' => 1.2e3,
        'itemNull' => null,
        'itemString' => 'hello',
        'itemObject' => $docTest,
        'itemArray' => (array) $docTest,
        'itemResource' => $rFilePointer,
    ];

    meminfo_info_dump($rFilePointer);

    rewind($rFilePointer);
    $jsonO = json_decode(stream_get_contents($rFilePointer), true);
    fclose($rFilePointer);
    if (is_array($jsonO)) {
        $items = $jsonO['items'];
        printf(
            "meminfo_info_dump JSON decode ok\nz_val_count:%d\n",
            count($items)
        );
        uasort($items, function ($itemA, $itemB) {
            return $itemA['type'] > $itemB['type']
            || ($itemA['type'] == $itemB['type'] && $itemA['size'] > $itemB['size']);
        });

        foreach($items as $item){
            printf(
                "item type:%s size:%d\n",
                $item['type'],
                $item['size']
            );
        }
    } else {
        echo 'meminfo_info_dump JSON decode fail';
    }
?>
--EXPECT--
meminfo_info_dump JSON decode ok
z_val_count:62
item type:array size:96
item type:array size:96
item type:array size:96
item type:array size:96
item type:array size:96
item type:array size:96
item type:array size:96
item type:array size:96
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:boolean size:24
item type:double size:24
item type:double size:24
item type:integer size:24
item type:integer size:24
item type:integer size:24
item type:integer size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:null size:24
item type:object size:56
item type:resource size:24
item type:string size:24
item type:string size:24
item type:string size:25
item type:string size:25
item type:string size:25
item type:string size:25
item type:string size:27
item type:string size:27
item type:string size:27
item type:string size:29
item type:string size:29
item type:string size:29
item type:string size:29
item type:string size:33
item type:string size:37
item type:string size:46
item type:string size:64
item type:string size:64
item type:string size:64
item type:string size:84
item type:string size:84
item type:string size:84
item type:string size:84
item type:string size:84
item type:string size:87
item type:string size:87
item type:string size:1054
