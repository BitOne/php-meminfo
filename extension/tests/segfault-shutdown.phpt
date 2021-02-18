--TEST--
Check that meminfo_dump during shutdown function doesn't segfault.
--SKIPIF--
<?php
    if (!extension_loaded('json')) die('skip json ext not loaded');
?>
--FILE--
<?php
register_shutdown_function(function () {
    $myArray = [
        "1|\x1f" => "My data"
    ];

    $dump = fopen('php://memory', 'rw');

    meminfo_dump($dump);

    rewind($dump);
    $meminfoData = json_decode(stream_get_contents($dump), true);
    fclose($dump);

    if (is_array($meminfoData)) {
        echo  "meminfo_dump JSON decode ok\n";
    } else {
        echo "meminfo_dump JSON decode fail\n";
    }
});
?>
--EXPECT--
meminfo_dump JSON decode ok
