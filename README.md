MEMINFO
=======
PHP Meminfo is a PHP extension that gives you insights on the PHP memory content.

Its main goal is to help you understand memory leaks: by looking at data present in memory, you can better understand your application behaviour.

One of the main source of inspiration for this tool is the Java jmap tool with the -histo option (see `man jmap`).

![Travis CI Results](https://travis-ci.org/BitOne/php-meminfo.svg?branch=master)

Compatibility
-------------
PHP 5.5, 5.6, 7.0, 7.1, and 7.2.

May compiles and works with PHP 5.3 and 5.4 but hasn't been tested with these versions.

Installation
------------
On macOS you can install or upgrade the extension using [Homebrew](https://brew.sh)

For exmaple, if you want to install PHP Meminfo for PHP 7.1, run:

```bash
$ brew install php71-meminfo
```

Compilation instructions
------------------------
## Compilation
From the root of the `extension/php5` for PHP 5 or `extension/php7` for PHP 7 directory:

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

This functions takes a stream handle as a parameter. It allows you to specify a file (ex `fopen('/tmp/file.txt', 'w')`, as well as to use standard output with the `php://stdout` stream.

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
The reference path is the path between a specific item in memory (identified by it's
pointer address) and all the intermediary items up to the one item that is attached
to a variable still alive in the program.

This path shows who are the items responsible for the memory leak of the specific item
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
$ bin/analyzer ref-path 0x7f94a1877068 /tmp/php_mem_dump.json
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

A worflow to find and understand memory leak by using PHP Meminfo
-----------------------------------------------------------------

[Hunting down memory leaks](doc/hunting_down_memory_leaks.md)

Other memory debugging tools for PHP
-------------------------------------
 - XDebug (https://xdebug.org/)
With the trace feature and the memory delta option (tool see XDebug documentation), you can trace function memory usage. You can use the provided script to get an aggregated view (TODO link)

 - PHP Memprof (https://github.com/arnaud-lb/php-memory-profiler)
Provides aggregated data about memory usage by functions. Far less resource intensive than a full trace from XDebug.

Troubleshooting
---------------

## "Call to undefined function" when calling `meminfo_dump`
It certainly means the extension is not enabled.

Check the PHP Info output and look for the MemInfo data.

To see the PHP Info output, just create a page calling the `phpinfo();` function, and load it from your browser, or call `php -i` from the command line.

Credits
-------
Thanks to Derick Rethans for his inspirational work on the essential XDebug. See http://www.xdebug.org/
