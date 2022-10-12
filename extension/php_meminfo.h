#ifndef PHP_MEMINFO_H
#define PHP_MEMINFO_H 1

#include "php.h"

extern zend_module_entry meminfo_module_entry;
#define phpext_meminfo_ptr &meminfo_module_entry

#define MEMINFO_NAME "PHP Meminfo"
#define MEMINFO_VERSION "2.0.0-beta1"
#define MEMINFO_AUTHOR "Benoit Jacquemont"
#define MEMINFO_COPYRIGHT "Copyright (c) 2010-2021 by Benoit Jacquemont & contributors"
#define MEMINFO_COPYRIGHT_SHORT "Copyright (c) 2010-2021"

ZEND_BEGIN_MODULE_GLOBALS(meminfo)
    zend_bool dump_on_limit;
ZEND_END_MODULE_GLOBALS(meminfo)

static ZEND_DECLARE_MODULE_GLOBALS(meminfo)
#define MEMINFO_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(meminfo, v)

PHP_FUNCTION(meminfo_dump);
PHP_MSHUTDOWN_FUNCTION(meminfo);
PHP_MINIT_FUNCTION(meminfo);
PHP_MINFO_FUNCTION(meminfo);
PHP_GINIT_FUNCTION(meminfo);

PHP_INI_BEGIN()
STD_PHP_INI_ENTRY("meminfo.dump_on_limit", "Off", PHP_INI_ALL, OnUpdateBool, dump_on_limit, zend_meminfo_globals, meminfo_globals)
PHP_INI_ENTRY("meminfo.dump_dir", "/tmp", PHP_INI_ALL, NULL)
PHP_INI_END()

zend_ulong   meminfo_get_element_size(zval* z);

// Functions to browse memory parts to record item
void perform_dump(php_stream* stream);
void meminfo_browse_exec_frames(php_stream *stream,  HashTable *visited_items, int *first_element);
void meminfo_browse_class_static_members(php_stream *stream,  HashTable *visited_items, int *first_element);

void meminfo_zval_dump(php_stream * stream, char * frame_label, zend_string * symbol_name, zval * zv, HashTable *visited_items, int *first_element);
void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element);
void meminfo_browse_zvals_from_symbol_table(php_stream *stream, char * frame_label, HashTable *symbol_table, HashTable * visited_items, int *first_element);
void meminfo_browse_zvals_from_op_array(php_stream *stream, char * frame_label, zend_op_array *op_array, zend_execute_data *exec_frame, HashTable * visited_items, int *first_element);
int meminfo_visit_item(char *item_identifier, HashTable *visited_items);

void meminfo_build_frame_label(char * frame_label, int frame_label_len, zend_execute_data* frame);

zend_string * meminfo_escape_for_json(const char *s);

static zend_bool should_autodump(int error_type, const char* message);

// Function pointer to original error handler
// See https://www.phpinternalsbook.com/php7/extensions_design/hooks.html
#if   PHP_VERSION_ID < 70200 /* PHP 7.1 */
static void (*original_zend_error_cb)(int type, const char* error_filename, const uint error_lineno, const char* format, va_list args);
#define MEMINFO_ZEND_ERROR_CB_ARGS_PASSTHRU type, error_filename, error_lineno, format, args
#elif PHP_VERSION_ID < 80000 /* PHP 7.2 - 7.4 */
static void (*original_zend_error_cb)(int type, const char* error_filename, const uint32_t error_lineno, const char* format, va_list args);
#define MEMINFO_ZEND_ERROR_CB_ARGS_PASSTHRU type, error_filename, error_lineno, format, args
#elif PHP_VERSION_ID < 80100 /* PHP 8.0 */
static void (*original_zend_error_cb)(int type, const char* error_filename, const uint32_t error_lineno, zend_string* message);
#define MEMINFO_ZEND_ERROR_CB_ARGS_PASSTHRU type, error_filename, error_lineno, message
#else                        /* PHP 8.1 */
static void (*original_zend_error_cb)(int type, zend_string* error_filename, const uint32_t error_lineno, zend_string* message);
#define MEMINFO_ZEND_ERROR_CB_ARGS_PASSTHRU type, error_filename, error_lineno, message
#endif

extern zend_module_entry meminfo_entry;

#endif
