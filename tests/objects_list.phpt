--TEST--
meminfo_objects_list with some objects
--FILE--
<?php
    class A{};
    class B{};

    $a = new A();
    $b = new B();

    meminfo_objects_list(fopen('php://stdout', 'w'));
?>
--EXPECTF--
Objects list:
  - Class A, handle 1, refCount 1
  - Class B, handle 2, refCount 1
Total object buckets: 2. Current objects: 2.
