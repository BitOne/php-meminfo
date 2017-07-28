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
  Zend Signed Integer (zend_long): 8 bytes.
  Zend Unsigned Integer (zend_ulong): 8 bytes.
  Zend Unsigned Char (zend_uchar): 1 bytes.
Structs size on this platform:
  Variable (zval): 16 bytes.
  Class (zend_class_entry): 504 bytes.
  Object (zend_object): 56 bytes.
