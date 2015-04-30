#ifndef PHP_MEMINFO_H
#define PHP_MEMINFO_H 1

#define MEMINFO_NAME "PHP Meminfo"
#define MEMINFO_VERSION "0.2"
#define MEMINFO_AUTHOR "Benoit Jacquemont"
#define MEMINFO_COPYRIGHT  "Copyright (c) 2010-2015 by Benoit Jacquemont"
#define MEMINFO_COPYRIGHT_SHORT "Copyright (c) 2011-2015"

PHP_FUNCTION(meminfo_structs_size);
PHP_FUNCTION(meminfo_objects_list);
PHP_FUNCTION(meminfo_objects_summary);
PHP_FUNCTION(meminfo_gc_roots_list);
PHP_FUNCTION(meminfo_symbol_table);

static int instances_count_compare(const void *a, const void *b TSRMLS_DC);

extern zend_module_entry meminfo_entry;
#define phpext_meminfo_ptr &meminfo_module_entry

#endif
