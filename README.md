MEMINFO
=======
PHP Meminfo is a PHP extension that gives you insights on the PHP memory content.

Its main goal is to help you understand memory leaks: by looking at data present in memory, you can better understand your application behaviour.

One of the main source of inspiration for this tool is the Java jmap tool with the -histo option (see `man jmap`).

Compatibility
-------------
PHP 5.6 and PHP 7.1 (may work on PHP 7.0 and 7.2, but not tested yet).

Compilation instructions
------------------------
You will need the `phpize` command. It can be installed on a Debian based system by:
```bash
$ apt-get install php5-dev
```
for PHP 5, or
```bash
$ apt-get install php7.1-dev
```
for PHP 7.1 when using the Ondrej repository from sury.org.

Once you have this command, follow this steps:

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
$ cd analyzers
$ composer install
```

Usage
-----
The extension has one main function: `meminfo_info_dump`.

This function generates a dump of the PHP memory in a JSON format. This dump can be later analyzed by the provided analyzers.

This functions takes a stream handle as a parameter. It allows you to specify a file (ex `fopen('/tmp/file.txt', 'w')`, as well as to use standard output with the `php://stdout` stream.

## Information gathered
Each memory item (string, boolean, objects, array, etc...) is dumped with the following information:
 - in-memory address
 - type (object, array, int, string, ...)
 - class (only for objects)
 - object handle (only for objects)
 - self size (without the size of the linked objects)
 - is_root (tells if the item is directly linked to a declared variable in the PHP program)
 - symbol name (name of the variable name in the PHP program, if the item is linked to a variable)
 - execution frame (name of the method where the variable has been declared)
 - children: list of linked items with the key name in case of array or property name if object, associated to their address in memory

### Dumping memory info
```php
meminfo_info_dump(fopen('/tmp/my_dump_file.json', 'w'));

```
Memory Leak Consequences
------------------------

The main consequences of a memory leak are:
 - increasing memory usage
 - decreasing performances

Decreasing performances is usually the most visible part. As memory leak saturates the garbage collector buffer, it runs far more often, without being able to free any memory. This leads to a high CPU usage of the process, with a lot of time spent in the garbage collector instead of your code (garbage collector doesn't run in parallel with the user code in PHP, it has to interrupt it).

See https://speakerdeck.com/bitone/hunting-down-memory-leaks-with-php-meminfo for a more detailed insight on how memory leak can occur.

Memory Leak Hunting Process
----------------------------
## Overview
 1. dump memory state with `meminfo_info_dump`
 2. use the *summary* command of the analyzer to display the item type that is the most present in memory. It's even better to use the summary to display the evolution of objects in memory in order, as the evolution will show where the memory leak really is
 3. use the *query* command of the analyzer to find one item from the class that is leaking
 4. use the *ref-path* command analyzer to find out the references that still hold this object in memory

## Object Leaks
On object oriented programming, a memory leak usually consists of *objects* leak.

## Memory state dump

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

Without the `-v` option, the output will contains only item memory address and key/property name. Adding the `-v` option will display all the information of the linked items.

```bash
$ bin/analyzer ref-path my_dump_file.json 0x12345678
```

Usage in production
-------------------
PHP Meminfo can be used in production, as it does not have any impact on performances outside of the call to the `meminfo` functions.

Nevertheless, production environment is not where you debug ;)

Other memory debugging tools for PHP
-------------------------------------
 - XDebug (https://xdebug.org/)
With the trace feature and the memory delta option (tool see XDebug documentation), you can trace function memory usage. You can use the provided script to get an aggregated view (TODO link)

 - PHP Memprof (https://github.com/arnaud-lb/php-memory-profiler)
Provides aggregated data about memory usage by functions. Far less resource intensive than a full trace from XDebug.

Troubleshooting
---------------

## "Call to undefined function" when calling `meminfo_info_dump`
It certainly means the extension is not enabled.

Check the PHP Info output and look for the MemInfo data.

To see the PHP Info output, just create a page calling the `phpinfo();` function, and load it from your browser, or call `php -i` from the command line.

Credits
-------
Thanks to Derick Rethans on his inspirational work on the essential XDebug. See http://www.xdebug.org/

Video presentation (in French)
------------------------------
https://www.youtube.com/watch?v=wZjnj1PAJ78
