--TEST--
meminfo_objects_summary with no objects
--FILE--
<?php
    meminfo_objects_summary(fopen('php://stdout', 'w'));
?>
--EXPECT--
Instances count by class:
rank         #instances   class
-----------------------------------------------------------------
