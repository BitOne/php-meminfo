--TEST--
meminfo_objects_list with a deleted object
--FILE--
<?php
    class A{};
    class B{};

    $a = new A();
    $b = new B();

    unset($a);

    meminfo_objects_list(fopen('php://stdout', 'w'));
?>
--EXPECT--
Objects list:
  - Class B, handle 2, refCount 1
Total object buckets: 2. Current objects: 1.
