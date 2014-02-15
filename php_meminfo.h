#ifndef PHP_MEMINFO_H
#define PHP_MEMINFO_H 1

#define MEMINFO_NAME "MemInfo"
#define MEMINFO_VERSION "0.1-dev"
#define MEMINFO_AUTHOR "Benoit Jacquemont"
#define XDEBUG_COPYRIGHT  "Copyright (c) 2010-2014 by Benoit Jacquemont"
#define XDEBUG_COPYRIGHT_SHORT "Copyright (c) 2010-2014"

PHP_FUNCTION(meminfo_structs_size);
PHP_FUNCTION(meminfo_objects_list);
PHP_FUNCTION(meminfo_gc_roots_list);
PHP_FUNCTION(meminfo_symbol_table);

extern zend_module_entry meminfo_entry;
#define phpext_meminfo_ptr &meminfo_module_entry

#endif
