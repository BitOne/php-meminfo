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
PHP_FUNCTION(meminfo_size_info);

static int instances_count_compare(const void *a, const void *b TSRMLS_DC);

const char * meminfo_get_classname(zend_object_handle handle);
char* meminfo_get_type_label(zval* z);
zend_ulong meminfo_get_element_size(zval* z);

void meminfo_browse_zval_with_size(php_stream * stream, zval * zv, HashTable *visited_items, int *first_element);
void meminfo_browse_hash_with_size(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element);
void meminfo_browse_zvals_from_symbol_table(php_stream *stream, HashTable *symbol_table, HashTable * visited_items, int *first_element);
int meminfo_visit_item(const char * item_label, HashTable *visited_items);

char * meminfo_escape_for_json(const char *s);
char * meminfo_size_info_generate_header(char *header);

extern zend_module_entry meminfo_entry;
#define phpext_meminfo_ptr &meminfo_module_entry

#endif
