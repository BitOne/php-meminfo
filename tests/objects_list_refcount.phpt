--TEST--
meminfo_objects_list with several refs
--FILE--
<?php
    class A{};
    class B{};
    class C{};

    $a1 = new A();

    // Zend optimisation seems to make this the same ref (COW ?)
    $b1 = new B();
    $b2 = $b1;

    // Getting the ref seems to disable this optim
    $c1 = new C();
    $c2 = &$c1;
    $c3 = $c2;

    meminfo_objects_list(fopen('php://stdout', 'w'));
?>
--EXPECTF--
Objects list:
  - Class A, handle 1, refCount 1
  - Class B, handle 2, refCount 1
  - Class C, handle 3, refCount 2
Total object buckets: 3. Current objects: 3.
