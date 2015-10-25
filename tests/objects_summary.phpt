--TEST--
meminfo_objects_summary with no objects
--FILE--
<?php
    class A {};
    class B {};
    class C {};

    $a1 = new A();
    $a1->name = "A1";
    $a2 = new A();
    $a2->name = "A2";

    $b1 = new B();
    $b2 = new B();
    $b3 = new B();
    $b4 = new B();

    $c1 = new C();

    meminfo_objects_summary(fopen('php://stdout', 'w'));
?>
--EXPECTF--
Instances count by class:
rank         #instances   class
-----------------------------------------------------------------
1            4            B
2            2            A
3            1            C
