#ifndef PHP_MEMINFO_H
#define PHP_MEMINFO_H 1

extern zend_module_entry meminfo_module_entry;
#define phpext_meminfo_ptr &meminfo_module_entry

#define MEMINFO_NAME "PHP Meminfo"
#define MEMINFO_VERSION "2.0.0-beta1"
#define MEMINFO_AUTHOR "Benoit Jacquemont"
#define MEMINFO_COPYRIGHT  "Copyright (c) 2010-2021 by Benoit Jacquemont & contributors"
#define MEMINFO_COPYRIGHT_SHORT "Copyright (c) 2010-2021"

PHP_FUNCTION(meminfo_dump);

zend_ulong   meminfo_get_element_size(zval* z);

// Functions to browse memory parts to record item
void meminfo_browse_exec_frames(php_stream *stream,  HashTable *visited_items, int *first_element);
void meminfo_browse_class_static_members(php_stream *stream,  HashTable *visited_items, int *first_element);

void meminfo_zval_dump(php_stream * stream, char * frame_label, zend_string * symbol_name, zval * zv, HashTable *visited_items, int *first_element);
void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element);
void meminfo_browse_zvals_from_symbol_table(php_stream *stream, char * frame_label, HashTable *symbol_table, HashTable * visited_items, int *first_element);
void meminfo_browse_zvals_from_op_array(php_stream *stream, char * frame_label, zend_op_array *op_array, zend_execute_data *exec_frame, HashTable * visited_items, int *first_element);
int meminfo_visit_item(char *item_identifier, HashTable *visited_items);

void meminfo_build_frame_label(char * frame_label, int frame_label_len, zend_execute_data* frame);

zend_string * meminfo_escape_for_json(const char *s);

extern zend_module_entry meminfo_entry;

#endif
