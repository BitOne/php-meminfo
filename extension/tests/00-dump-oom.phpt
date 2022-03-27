--TEST--
Trigger PHP OOM
--FILE--
<?php

ini_set("meminfo.dump_on_limit", true);
ini_set("meminfo.dump_dir", __DIR__);

$things = [];
for ($i = 0; $i < 3000; $i++) {
    $things []= str_repeat("*", rand(50000, 100000));
}
--EXPECT--
--XFAIL--
This test triggers an OOM error which will write a heap dump into the test
directory. The test dump-oom-confirm.phpt will verify if the heap dump was
written. This test is prefixed with 00 to ensure that it runs before the
confirm test.
