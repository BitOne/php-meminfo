--TEST--
meminfo_objects_list with no objects
--FILE--
<?php
    meminfo_objects_list(fopen('php://stdout', 'w'));
?>
--EXPECT--
Objects list:
Total object buckets: 0. Current objects: 0.
