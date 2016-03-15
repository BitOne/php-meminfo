--TEST--
meminfo_struct_size in 64 bits
--SKIPIF--
<?php if (php_uname('m') != 'x86_64') echo "skip x86_64 arch needed\n"; ?>
--FILE--
<?php
    meminfo_structs_size(fopen('php://stdout', 'w'));
?>
--EXPECT--
Simple Zend Type size on this platform
  Zend Unsigned Int (zend_uint): 4 bytes.
  Zend Unsigned Char (zend_uchar): 1 bytes.
Structs size on this platform:
  Variable value (zvalue_value): 16 bytes.
  Variable (zval): 24 bytes.
  Class (zend_class_entry): 576 bytes.
  Object (zend_object): 32 bytes.
