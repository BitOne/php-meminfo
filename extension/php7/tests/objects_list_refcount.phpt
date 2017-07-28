--TEST--
meminfo_objects_list with several refs
--FILE--
<?php
    class A{};
    class B{};
    class C{};

    $a1 = new A();

    $b1 = new B();
    $b2 = $b1;

    $c1 = new C();
    $c2 = $c1;
    $c3 = $c2;

    meminfo_objects_list(fopen('php://stdout', 'w'));
?>
--EXPECT--
Objects list:
  - Class A, handle 1, refCount 1
  - Class B, handle 2, refCount 2
  - Class C, handle 3, refCount 3
Total object buckets: 3. Current objects: 3.
