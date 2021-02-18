MEMINFO
=======
PHP Meminfo is a PHP extension that gives you insights on the PHP memory content.

Its main goal is to help you understand memory leaks: by looking at data present in memory, you can better understand your application behaviour.

One of the main sources of inspiration for this tool is the Java jmap tool with the -histo option (see `man jmap`).

![Build](https://github.com/BitOne/php-meminfo/workflows/Build%20the%20extension%20and%20test%20the%20analyzers/badge.svg)

Compatibility
-------------
PHP 7.x, 8.0.

For older versions of PHP, you can use the following releases:
 - 5.6: PHP Meminfo v1.1
 - 5.5: PHP Meminfo v1.0.5 (may work with PHP 5.3 and PHP 5.4 but not tested)

Compilation instructions
------------------------
## Compilation
From the root of the `extension` directory:

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
$ cd analyzer
$ composer install
```

Usage
-----
## Dumping memory content

```php
meminfo_dump(fopen('/tmp/my_dump_file.json', 'w'));

```

This function generates a dump of the PHP memory in a JSON format. This dump can be later analyzed by the provided analyzers.

This function takes a stream handle as a parameter. It allows you to specify a file (ex `fopen('/tmp/file.txt', 'w')`, as well as to use standard output with the `php://stdout` stream.

## Displaying a summary of items in memory
```bash
$ bin/analyzer summary <dump-file>

Arguments:
  dump-file             PHP Meminfo Dump File in JSON format
```

### Example
```bash
$ bin/analyzer summary /tmp/my_dump_file.json
+----------+-----------------+-----------------------------+
| Type     | Instances Count | Cumulated Self Size (bytes) |
+----------+-----------------+-----------------------------+
| string   | 132             | 7079                        |
| MyClassA | 100             | 7200                        |
| array    | 10              | 720                         |
| integer  | 5               | 80                          |
| float    | 2               | 32                          |
| null     | 1               | 16                          |
+----------+-----------------+-----------------------------+
```

## Displaying a list of objects with the largest number of children
```bash
$ bin/analyzer top-children [options] [--] <dump-file>

Arguments:
  dump-file             PHP Meminfo Dump File in JSON format

Options:
  -l, --limit[=LIMIT]   limit [default: 5]
```

### Example
```bash
$ bin/analyzer top-children /tmp/my_dump_file.json
+-----+----------------+----------+
| Num | Item ids       | Children |
+-----+----------------+----------+
| 1   | 0x7ffff4e22fe0 | 1000000  |
| 2   | 0x7fffe780e5c8 | 11606    |
| 3   | 0x7fffe9714ef0 | 11602    |
| 4   | 0x7fffeab63ca0 | 3605     |
| 5   | 0x7fffd3161400 | 2400     |
+-----+----------------+----------+

```

## Querying the memory dump to find specific objects
```bash
$ bin/analyzer query [options] [--] <dump-file>

Arguments:
  dump-file              PHP Meminfo Dump File in JSON format

Options:
  -f, --filters=FILTERS  Filter on an attribute. Operators: =, ~. Example: class~User (multiple values allowed)
  -l, --limit=LIMIT      Number of results limit (default 10).
  -v                     Increase the verbosity
```

### Example

```bash
$ bin/analyzer query -v -f "class=MyClassA" -f "is_root=0" /tmp/php_mem_dump.json
+----------------+-------------------+------------------------------+
| Item ids       | Item data         | Children                     |
+----------------+-------------------+------------------------------+
| 0x7f94a1877008 | Type: object      | myObjectName: 0x7f94a185cca0 |
|                | Class: MyClassA   |                              |
|                | Object Handle: 1  |                              |
|                | Size: 72 B        |                              |
|                | Is root: No       |                              |
+----------------+-------------------+------------------------------+
| 0x7f94a1877028 | Type: object      | myObjectName: 0x7f94a185cde0 |
|                | Class: MyClassA   |                              |
|                | Object Handle: 2  |                              |
|                | Size: 72 B        |                              |
|                | Is root: No       |                              |
+----------------+-------------------+------------------------------+
| 0x7f94a1877048 | Type: object      | myObjectName: 0x7f94a185cf20 |
|                | Class: MyClassA   |                              |
...

```

## Displaying the reference path
The reference path is the path between a specific item in memory (identified by its
pointer address) and all the intermediary items up to the one item that is attached
to a variable still alive in the program.

This path shows which items are responsible for the memory leak of the specific item
provided.

```bash
$ bin/analyzer ref-path <item-id> <dump-file>

Arguments:
  item-id               Item Id in 0xaaaaaaaa format
  dump-file             PHP Meminfo Dump File in JSON format

Options:
  -v                     Increase the verbosity
```

### Example

```bash
$ bin/analyzer ref-path -v 0x7f94a1877068 /tmp/php_mem_dump.json
Found 1 paths
Path from 0x7f94a1856260
+--------------------+
| Id: 0x7f94a1877068 |
| Type: object       |
| Class: MyClassA    |
| Object Handle: 4   |
| Size: 72 B         |
| Is root: No        |
| Children count: 1  |
+--------------------+
         ^
         |
         3
         |
         |
+---------------------+
| Id: 0x7f94a185cb60  |
| Type: array         |
| Size: 72 B          |
| Is root: No         |
| Children count: 100 |
+---------------------+
         ^
         |
    second level
         |
         |
+--------------------+
| Id: 0x7f94a185ca20 |
| Type: array        |
| Size: 72 B         |
| Is root: No        |
| Children count: 1  |
+--------------------+
         ^
         |
    first level
         |
         |
+---------------------------+
| Id: 0x7f94a1856260        |
| Type: array               |
| Size: 72 B                |
| Is root: Yes              |
| Execution Frame: <GLOBAL> |
| Symbol Name: myRootArray  |
| Children count: 1         |
+---------------------------+
```

A workflow to find and understand memory leaks using PHP Meminfo
----------------------------------------------------------------

[Hunting down memory leaks](doc/hunting_down_memory_leaks.md)

Other memory debugging tools for PHP
-------------------------------------
 - XDebug (https://xdebug.org/)
With the trace feature and the memory delta option (tool see XDebug documentation), you can trace function memory usage. You can use the provided script to get an aggregated view (TODO link)

 - PHP Memprof (https://github.com/arnaud-lb/php-memory-profiler)
Provides aggregated data about memory usage by functions. Far less resource intensive than a full trace from XDebug.

Troubleshooting
---------------
## "A lot of memory usage is reported by the `memory_usage` entry, but the cumulative size of the items in the summary is far lower than the memory usage"

A lot of memory is used internally by the Zend Engine itself to compile PHP files, to run the virtual machine, to execute the garbage collector, etc... Another part of the memory is usually taken by PHP extensions themselves. And the remaining memory usage comes from the PHP data structures from your program.

In some cases, several hundred megabytes can be used internally by some PHP extensions. Examples are the PDO extension and MySQLi extension.
By default, when executing a SQL query they will buffer all the results inside the PHP memory:
http://php.net/manual/en/mysqlinfo.concepts.buffering.php

In case of very large number of results, this will consume a lot of memory, and this memory usage is not caused by the data you have in your objects or array manipulated by your program, but by the way the extension works.

This is only one example, but the same can happen with image manipulation extensions, that will use a lot of memory to transform images.

All the extensions are using the Zend Memory Manager, so that they will not exceed the maximum memory limit set for the PHP process. So their memory usage is included in the information provided by `memory_get_usage()`.

But PHP Meminfo is only able to get information on memory used by the data structure from the PHP program, not from the extensions themselves.

Hence the difference between those numbers, which can be quite big.

## "Call to undefined function" when calling `meminfo_dump`
This means the extension is not enabled.

Check the PHP Info output and look for the MemInfo data.

To see the PHP Info output, just create a page calling the `phpinfo();` function, and load it from your browser, or call `php -i` from the command line.

## Why most tests are "skipped"?

While doing a `make test`, some tests will need JSON capabilities. But the
compilation system generates a clean env by removing all configuration
directives that load extensions.
So if JSON capabilites are packaged as a separate extension (instead of
being compiled directly in the PHP runtime), the tests will be skipped.

You may run them with the `run-tests.php` generated after the `make test`
command, by providing the `php` executable:

```bash
$ TEST_PHP_EXECUTABLE=$(which php) $(which php) run-tests.php -d extension=$PWD/modules/meminfo.so

```
In this case your tests will run with your local PHP configuration,
including the loading of the JSON extension.

Please note this is not required when working with PHP 8 as the JSON functions are now
usually complied in PHP directly.

Credits
-------
Thanks to Derick Rethans for his inspirational work on the essential XDebug. See http://www.xdebug.org/
