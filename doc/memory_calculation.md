Memory calculation
==================

# Dependencies
The dependencies are defined in term of parent/child relationships. Relationships can only be carried by arrays and objects.
When an object has an attribute (or a value in case of array)  pointing to another item, this object or array is considered as
a parent of the item.

# How memory size is computed

In this interface, you will find two memory size informations: *self size* and *full size*.

## Self size
This information is the size of the item itself, not including the size of objects linked.

## Full size
The full size is the sum of the sizes of all descendants of the objects.

## How multi-links are accounted in memory size
Look at the following code:

```php
    $parentObject1 = new SplObject();
    $parentObject2 = new SplObject();

    $childObject = new SplObject();

    $parentObject1->objectA = $childObject;

    $parentObject2->objectA = $childObject;
```

When suming the full size of `$parentObject1` and `$parentObject2,
the naive way would be to sum the size of all descendants.
But this would mean accounting`$childObject`twice.

So for each item, we flag it as *accounted* the first time we take it
into account when computing a full size object, and *non-accounted* the
following times. And as an *non-accounted* itme, it's not included in the
full-size of the object, thus avoiding counted objects twice.

For the previous code example, the final result would be:
  - parentObject1
    - size: (self size of parentObject1 + self size of childObject)
    - accounted children:
      - childObject
    - non-accounted children:
      - *empty*
  - parentObject2
    - size: (self size of parentObject2)
    - accounted children:
      - *empty*
    - non-accounted children:
      - childObject

### Warning
"When" a child is accounted can be quite random, as the program cannot know
how what is the more "meaningful" relationship that should account the child
instead of a less "meaningful" relationship.

More sophisticated strategy could be implemented in the future to produce a
more expected result.

# Limitations
## Memory calculations
The following items are not yet taken into account in memory calculation:
 - classes memory usage
 - array keys memory usage
 - objects attribute name memory usage

