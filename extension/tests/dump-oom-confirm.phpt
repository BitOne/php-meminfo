--TEST--
Confirm heap dump on OOM
--FILE--
<?php

$files = glob(__DIR__ . '/php_heap*.json');
echo count($files);
--EXPECT--
1
--CLEAN--
<?php

$files = glob(__DIR__ . '/php_heap*.json');
foreach($files as $file) {
  if(is_file($file)) {
    unlink($file);
  }
}
