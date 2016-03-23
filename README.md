MEMINFO
=======
PHP Meminfo is a PHP extension that gives you insights on the PHP memory content.

Its main goal is to help you understand memory leaks: by looking at data present in memory, you can better understand your application behaviour.

One of the main source of inspiration for this tool is the Java jmap tool with the -histo option (see `man jmap`).

Compatibility
-------------
Compiled and tested on:

 - PHP 5.4.4 (Debian 7)
 - PHP 5.5.8 (Ubuntu 12.04 LTS)
 - PHP 5.5.20 (CentOS 7)
 - PHP 5.6.17 (Debian 8)

Compilation instructions
------------------------
You will need the `phpize` command. It can be installed on a Debian based system by:
```bash
$ apt-get install php5-dev
```

Once you have this command, follow this steps:

## Compilation
From the root of the `extension/` directory:

```bash
$ phpize
$ ./configure --enable-meminfo
$ make
$ make install
```

## Enabling the extension
Add the following line to your `php.ini`:

```ini
extension=meminfo.so
```

Installing analyzers
--------------------
Analyzers allow to analyze a memory dump (see below).

```bash
$ cd analyzers
$ composer update
```

Usage
-----
All meminfo functions take a stream handle as a parameter. It allows you to specify a file (ex `fopen('/tmp/file.txt', 'w')`, as well as to use standard output with the `php://stdout` stream.

## Object instances count per class
Display the number of instances per class, ordered descending. Very useful to identify the content of a memory leak.

```php
    meminfo_objects_summary(fopen('php://stdout','w'));
```

The result will provide something similar to the following example generated at the end of a Symfony2 console launch:

```
    Instances count by class:
    num          #instances   class
    -----------------------------------------------------------------
    1            181          Symfony\Component\Console\Input\InputOption
    2            88           Symfony\Component\Console\Input\InputDefinition
    3            77           ReflectionObject
    4            46           Symfony\Component\Console\Input\InputArgument
    5            2            Symfony\Bridge\Monolog\Logger
    6            1            Symfony\Component\EventDispatcher\ContainerAwareEventDispatcher
    7            1            Doctrine\Bundle\MigrationsBundle\Command\MigrationsDiffDoctrineCommand
    ...
```

Note: It's a good idea to call the `gc_collect_cycles()` function before executing  `meminfo_objects_summary()`, as it will collect dead objects that has not been reclaimed by the ref counter due to circular references. See http://www.php.net/manual/en/features.gc.php for more details.


### Examples
The `examples/` directory at the root of the repository contains more detailed examples.
```bash
    $ php examples/objects_summary.php
```

## Memory state dump
This feature allow to dump the list of items present in memory at the time of the function execution. Each memory items (string, boolean, objects, array, etc...) are dumped in a JSON format, with the following information:
 - in memory address
 - type (object, array, int, string, ...)
 - class (only for objects)
 - object handle (only for objects.
 - self size (without the size of the linked objects)
 - is_root (tells if the item is directly linked to a variable)
 - symbol name (variable name, if linked to a variable)
 - execution frame (name of the method where the variable has been declared)
 - children: list of linked items with the key value if array or property name if object and the item address in memory

### Analyzing a memory dump
The analyzer is available from the `analyzer/` directory. It will be invoked with:
``` bash
$ bin/analyzer
```

#### Querying a memory dump
The `query` command on the analyzer allows you to filter out some items from a memory dump. The `-f` option can be used several times, effectively *anding* the filters. The supported operators are exact match `=` and regexp match `~`.

The `-v`option display all the information of the items found.

##### Examples
 - finding array that are not directly linked to a variable
```bash
$ bin/analyzer query -f "type=array" -f "is_root=0" my_dump_file.json
```
 - finding objects whose class name contains `Product` and linked to a variable
```bash
$ bin/analyzer query -f "class~Product" -f "is_root=1" -v my_dump_file.json
```

#### Finding out why an object has not been removed from memory
When you are tracking down a memory leak, it's very interesting to understand why an object is still in memory.

The analyzer provides the `ref-path` command that load the memory dump as a graph in memory and findout all paths linking an item to a root (a variable define in an execution frame).

Without the `-v` option, the output will contains only item memory adress and key/property name. Adding the `-v` option will display all the information of the linked items.

```bash
$ bin/analyzer ref-path my_dump_file.json 0x12345678
```

## List of items in memory
Provides a list of items in memory (objects, arrays, string, etc.) with their sizes.

```php
    meminfo_size_info(fopen('php://stdout','w'));
```

For example:
```json
// ...
    "0x7fe06ea50a40" : {
        "type" : "array",
        "size" : "96",
        "children" : {
            "0":"0x7fe06ea649b0"
        }

    },
    "0x7fe06ea649b0" : {
        "type" : "string",
        "size" : "99"

    },
//...
```

Note: The same remark about `gc_collect_cycles()` before `meminfo_objects_summary()` applies as well for this function.

### Examples
The `examples/` directory at the root of the repository contains more detailed examples.

    php examples/size_info.php

##List of currently active objects
Provides a list of live objects with their class and their handle, as well as the total number of active objects and the total number of allocated object buckets.

```php
    meminfo_objects_list(fopen('php://stdout','w'));
```

For example:

    Objects list:
      - Class MyClassB, handle 2, refCount 1
      - Class MyClassC, handle 5, refCount 1
      - Class MyClassC, handle 6, refCount 1
      - Class MyClassC, handle 7, refcount 1
    Total object buckets: 7. Current objects: 4.

Note: The same remark about `gc_collect_cycles()` before `meminfo_objects_summary()` applies as well for this function.

### Examples
The `examples/` directory at the root of the repository contains more detailed examples.

    php examples/objects_list.php

## Information on structs size
Display size in byte of main data structs size in PHP. Will mainly differ between 32bits et 64bits environments.

```php
    meminfo_structs_size(fopen('php://stdout','w'));
```

It can be useful to understand difference in memory usage between two platforms.

Example Output on 64bits environment:

```
    Structs size on this platform:
      Class (zend_class_entry): 568 bytes.
      Object (zend_object): 32 bytes.
      Variable (zval): 24 bytes.
      Variable value (zvalue_value): 16 bytes.
```

Usage in production
-------------------
PHP Meminfo can be used in production, as it does not have any impact on performances outside of the call to the `meminfo` functions.

Nevertheless, production environment is not where you debug ;)

Other memory debugging tools for PHP
-------------------------------------
 - XDebug (http://www.xdebug.net)
With the trace feature and the memory delta option (tool see XDebug documentation), you can trace function memory usage. You can use the provided script to get an aggregated view (TODO link)

 - PHP Memprof (https://github.com/arnaud-lb/php-memory-profiler)
Provides aggregated data about memory usage by functions. Far less resource intensive than a full trace from XDebug.

Troubleshooting
---------------

## "Call to undefined function" when calling meminfo_* functions
It certainly means the extension is not enabled.

Check the PHP Info output and look for the MemInfo data.

To see the PHP Info output, just create a page calling the `phpinfo();` function, and load it from your browser, or call `php -i` from command line.

Credits
-------
Thanks to Derick Rethans on his inspirational work on the essential XDebug. See http://www.xdebug.org/
