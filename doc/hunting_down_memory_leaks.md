Hunting Down Memory Leaks
=========================

Why getting rid of Memory Leaks ?
---------------------------------

The main consequences of a memory leak are:
 - increasing memory usage
 - decreasing performances

Decreasing performances is usually the most visible part. As memory leak
saturates the garbage collector buffer, it runs far more often, without
being able to free any memory.

This leads to a high CPU usage of the process, with a lot of time spent
in the garbage collector instead of your code (garbage collector doesn't
run in parallel with the user code in PHP, so the garbage collector has to pause the user code).

See https://speakerdeck.com/bitone/hunting-down-memory-leaks-with-php-meminfo
for a more detailed insight on how memory is released in PHP and how memory
leaks can occur.

How to hunt Memory Leak Down
----------------------------
## Process Overview
 1. Dump memory state.
 2. Display a *summary* of items in memory.
 3. Find an item that shouldn't be in memory anymore, but is part of the dump.
 4. Track down the reference that are still holding the object in memory.
 5. Fix the code.
 6. Profit!

## 1. Dumping Memory State
From anywhere in your PHP code, the following line will dump the current state
of the PHP memory, including all the items alive in memory:
```php
meminfo_dump(fopen('/tmp/php_mem_dump.json', 'w'));

```

## 2. Memory state overview with the summary analyzer
From the analyzer directory:
```bash
$ bin/analyzer summary /tmp/php_mem_dump.json
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

The summary gives you an aggregated view of items by type or class.
It's the best tool to know where to look at, as most of the time a memory
leak will generate objects of the same class.

So a huge volume of instances of a particular class is a good indicator of the
direction to take to track down the memory leak.

Even better is to follow the evolution of the summary, so the leak will be even
more apparent as the number of items leaking will be the one increasing over time.

Once we have a good idea of the kind of items that are leaking, let's go deeper
and try to find why there are still in memory.
## 3. Finding objects with the most children
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
Perhaps some array accumulates logs or caches something. This command helps you to find items with many children.
But be careful this command does not summarize multi-dimensional arrays.
For resolve ID filed see the next paragraph - "Finding references to the leaked object".

## 4. Finding an instance of a leaked object
So now, we know of what class are the object leaking. It can be multiple classes in the case of
objects composed with multiple other objects.

But only one class is needed for now. Let's take our `MyClassA` example.

We will use the analyzer's `query` command to find a single instance of this class
that is still in memory, and that is not directly attached to a variable declared in a
living scope (`is_root` at `0`).

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
|                | Object Handle: 3  |                              |
|                | Size: 72 B        |                              |
|                | Is root: No       |                              |
+----------------+-------------------+------------------------------+
| 0x7f94a1877068 | Type: object      | myObjectName: 0x7f94a185d060 |
|                | Class: MyClassA   |                              |
|                | Object Handle: 4  |                              |
|                | Size: 72 B        |                              |
|                | Is root: No       |                              |
+----------------+-------------------+------------------------------+
| 0x7f94a1877088 | Type: object      | myObjectName: 0x7f94a185d1a0 |
|                | Class: MyClassA   |                              |
...
```

We have a list of ten items matching our query (use the `-l` option to get more).

So let's take one of them, for example `0x7f94a1877068`, and find out what are
the references that are still alive and pointing to it.

## 5. Finding references to the leaked object

We know this object is still in memory, and we know he is not linked directly
to a variable (not root). So it means another item (like an array or an object)
has reference to our object. Maybe our object is in an array or maybe it's a member
of another object.
And maybe the *container* of our object is attached directly to a variable. It's a *root*.

But maybe, it's not. Maybe the container itself is part of an array or another object.
And it can goes very far until you find the object or array that is the root of that.

To automatically find the path from our leaked object to the root that makes it stay
in memory, we will use the `ref-path` command.

```bash
$ bin/analyzer -v ref-path 0x7f94a1877068 /tmp/php_mem_dump.json
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
So the reason why our object is still in memory is because it's in an array,
that is itself in another array that is in an final array. And the final array
is directly linked to the variable declared in the `<GLOBAL>` frame and called `myRootArray`.

In a shorter PHP-ish notation, this could be written like that:
```php
$myRootArray['first level']['second level'][3] = 0x7f94a1877068;
```

So now, we know we are leaking this object because we still have a reference
going down to this declared `$myRootArray` variable.

This tremendously helps understand the reason of the memory leak. Now that we
have the leaking objects and the item that keep reference to them, we have
all the information needed to debug the code.

It's your turn to play ;)

Video presentation in English
-----------------------------
https://www.youtube.com/watch?v=NjIlKlFImlo

A French version:
https://www.youtube.com/watch?v=wZjnj1PAJ78


