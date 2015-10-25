#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_meminfo.h"

#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "zend_extensions.h"
#include "zend_exceptions.h"

#include "zend.h"
#include "SAPI.h"
#include "zend_API.h"

const zend_function_entry meminfo_functions[] = {
    PHP_FE(meminfo_structs_size, NULL)
    PHP_FE(meminfo_objects_list, NULL)
    PHP_FE(meminfo_objects_summary, NULL)
    PHP_FE(meminfo_gc_roots_list, NULL)
    PHP_FE(meminfo_symbol_table, NULL)
    PHP_FE(meminfo_size_info, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry meminfo_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "meminfo",
    meminfo_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    MEMINFO_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_MEMINFO
ZEND_GET_MODULE(meminfo)
#endif



PHP_FUNCTION(meminfo_structs_size)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);

    php_stream_printf(stream TSRMLS_CC, "Simple Zend Type size on this platform\n");
    php_stream_printf(stream TSRMLS_CC, "  Zend Unsigned Int (zend_uint): %ld bytes.\n", sizeof(zend_uint));
    php_stream_printf(stream TSRMLS_CC, "  Zend Unsigned Char (zend_uchar): %ld bytes.\n", sizeof(zend_uchar));

    php_stream_printf(stream TSRMLS_CC, "Structs size on this platform:\n");
    php_stream_printf(stream TSRMLS_CC, "  Variable value (zvalue_value): %ld bytes.\n", sizeof(zvalue_value));
    php_stream_printf(stream TSRMLS_CC, "  Variable (zval): %ld bytes.\n", sizeof(zval));
    php_stream_printf(stream TSRMLS_CC, "  Class (zend_class_entry): %ld bytes.\n", sizeof(zend_class_entry));
    php_stream_printf(stream TSRMLS_CC, "  Object (zend_object): %ld bytes.\n", sizeof(zend_object));
}

PHP_FUNCTION(meminfo_objects_list)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);

    php_stream_printf(stream TSRMLS_CC, "Objects list:\n");

// TODO: check if object_buckets exists ? See gc_collect_roots from zend_gc.c
    zend_objects_store *objects = &EG(objects_store);
    zend_uint i;
    zend_uint total_objects_buckets = objects->top - 1;
    zend_uint current_objects = 0;
    zend_object * object;
    zend_class_entry * class_entry;

    for (i = 1; i < objects->top ; i++) {
        if (objects->object_buckets[i].valid) {
            struct _store_object *obj = &objects->object_buckets[i].bucket.obj;

            php_stream_printf(stream TSRMLS_CC, "  - Class %s, handle %d, refCount %d\n", meminfo_get_classname(i), i, obj->refcount);

            current_objects++;
        }
     }

    php_stream_printf(stream TSRMLS_CC, "Total object buckets: %d. Current objects: %d.\n", total_objects_buckets, current_objects);
}

PHP_FUNCTION(meminfo_objects_summary)
{
    zval *zval_stream = NULL;
    php_stream *stream = NULL;
    HashTable *classes = NULL;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }
    php_stream_from_zval(stream, &zval_stream);

    ALLOC_HASHTABLE(classes);

    zend_hash_init(classes, 1000, NULL, NULL, 0);

    zend_objects_store *objects = &EG(objects_store);
    zend_uint i;
    zend_object *object;

    for (i = 1; i < objects->top ; i++) {
        if (objects->object_buckets[i].valid && !objects->object_buckets[i].destructor_called) {

            const char *class_name;
            zend_uint *p_instances_count;

            class_name = meminfo_get_classname(i);

            if (zend_hash_find(classes, class_name, strlen(class_name)+1, (void **) &p_instances_count) == SUCCESS) {
                (*p_instances_count)++;
            } else {
                zend_uint instances_count;
                instances_count = 1;
                p_instances_count = &instances_count;

                zend_hash_update(classes, class_name, strlen(class_name)+1, p_instances_count, sizeof(zend_uint *), NULL);
            }
        }
    }

    zend_hash_sort(classes, zend_qsort, meminfo_instances_count_compare, 0 TSRMLS_CC);

    php_stream_printf(stream TSRMLS_CC, "Instances count by class:\n");

    php_stream_printf(stream TSRMLS_CC, "%-12s %-12s %s\n", "rank", "#instances", "class");
    php_stream_printf(stream TSRMLS_CC, "-----------------------------------------------------------------\n");

    zend_uint rank = 1;

    HashPosition position;

    zend_uint *p_instances_count;

    for (zend_hash_internal_pointer_reset_ex(classes, &position);
         zend_hash_get_current_data_ex(classes, (void **) &p_instances_count, &position) == SUCCESS;
         zend_hash_move_forward_ex(classes, &position)) {

        char *class_name = NULL;
        uint  class_name_len;
        ulong index;

        zend_hash_get_current_key_ex(classes, &class_name, &class_name_len, &index, 0, &position);
        php_stream_printf(stream TSRMLS_CC, "%-12d %-12d %s\n", rank, *p_instances_count, class_name);

        rank++;
    }

    zend_hash_destroy(classes);
    FREE_HASHTABLE(classes);
}


PHP_FUNCTION(meminfo_gc_roots_list)
{
    zval *zval_stream;
    php_stream *stream;
    zval* pz;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);
    php_stream_printf(stream TSRMLS_CC, "GC roots list:\n");

    gc_root_buffer *current = GC_G(roots).next;

    while (current != &GC_G(roots)) {
        pz = current->u.pz;
        php_stream_printf(stream TSRMLS_CC, "  zval pointer: %p ", (void *) pz);
        if (current->handle) {
            php_stream_printf(
                stream TSRMLS_CC,
                "  Class %s, handle %d\n",
                meminfo_get_classname(current->handle),
                current->handle);
        } else {
            php_stream_printf(stream TSRMLS_CC, "  Type: %s",  zend_get_type_by_const(Z_TYPE_P(pz)));
            php_stream_printf(stream TSRMLS_CC, ", Ref count GC %d\n", pz->refcount__gc);

        }
        current = current->next;

    }
}

PHP_FUNCTION(meminfo_symbol_table)
{
    zval *zval_stream;
    php_stream *stream;
    HashTable main_symbol_table;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);

    main_symbol_table = EG(symbol_table);

    php_stream_printf(stream TSRMLS_CC, "Nb elements in Symbol Table: %d\n",main_symbol_table.nNumOfElements);
}

/**
 * Generate a JSON output of the list of items in memory (objects, arrays, string, etc...)
 * with their sizes
 */
PHP_FUNCTION(meminfo_size_info)
{
    zval *zval_stream;
    zend_execute_data *exec_frame;
    HashTable *frame_symbol_table, *global_symbol_table;
    char header[1024];

    int first_element;

    php_stream *stream;
    HashTable *visited_items;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);
    php_stream_printf(stream TSRMLS_CC, "{\n");

    php_stream_printf(stream TSRMLS_CC, "\"header\":\n");
    php_stream_printf(stream TSRMLS_CC, meminfo_size_info_generate_header(header));
    php_stream_printf(stream TSRMLS_CC, ",\n");

    php_stream_printf(stream TSRMLS_CC, "\"items\": {\n");

    ALLOC_HASHTABLE(visited_items);
    zend_hash_init(visited_items, 1000, NULL, NULL, 0);

    first_element = 1;

    exec_frame = EG(current_execute_data);

    while (exec_frame) {
        frame_symbol_table = exec_frame->symbol_table;


        /* TODO Check why it is NULL sometimes
         * See some example where the symbol table
         * seems to be "regenerated"
         */
        if (frame_symbol_table != NULL) {
            meminfo_browse_zvals_from_symbol_table(stream, frame_symbol_table, visited_items, &first_element);
        }

        exec_frame = exec_frame->prev_execute_data;
    }

    global_symbol_table = &EG(symbol_table);

    meminfo_browse_zvals_from_symbol_table(stream, global_symbol_table, visited_items, &first_element);

    php_stream_printf(stream TSRMLS_CC, "\n    }\n");
    php_stream_printf(stream TSRMLS_CC, "}\n}\n");

    zend_hash_destroy(visited_items);
    FREE_HASHTABLE(visited_items);
}

/**
   Compare two hashtable buckets  by extracting their
   int value and return the comparision result.
*/
static int meminfo_instances_count_compare(const void *a, const void *b TSRMLS_DC)
{
    const Bucket *bucket_a;
    const Bucket *bucket_b;

    bucket_a = *((const Bucket **) a);
    bucket_b = *((const Bucket **) b);

    zend_uint instances_count_a;
    zend_uint instances_count_b;

    instances_count_a = (zend_uint) bucket_a->pDataPtr;
    instances_count_b = (zend_uint) bucket_b->pDataPtr;

    if (instances_count_a > instances_count_b) {
        return -1;
    } else if (instances_count_a == instances_count_b) {
        return 0;
    } else {
        return 1;
    }
}

/**
 * Return the class associated to the provided object handle
 *
 * @param zend_object_handle object handle
 *
 * @return char * class name
 */
const char * meminfo_get_classname(zend_object_handle handle)
{
    zend_objects_store *objects = &EG(objects_store);
    zend_object *object;
    zend_class_entry *class_entry;
    const char* class_name;

    class_name = "";

    if (objects->object_buckets[handle].valid) {
        struct _store_object *obj = &objects->object_buckets[handle].bucket.obj;
        object =  (zend_object * ) obj->object;

        class_entry = object->ce;
        class_name = class_entry->name;
    }

    return class_name;
}

void meminfo_browse_zvals_from_symbol_table(php_stream *stream, HashTable *symbol_table, HashTable * visited_items, int *first_element)
{
    zval **zval;

    HashPosition pos;

    zend_hash_internal_pointer_reset_ex(symbol_table, &pos);
    while (zend_hash_get_current_data_ex(symbol_table, (void **) &zval, &pos) == SUCCESS) {

        meminfo_browse_zval_with_size(stream, *zval, visited_items, first_element);

        zend_hash_move_forward_ex(symbol_table, &pos);
    }
}

int meminfo_visit_item(const char * item_label, HashTable *visited_items)
{
    int found = 0;
    int isset = 1;


    if (zend_hash_exists(visited_items, item_label, strlen(item_label))) {
        found = 1;
    } else {
        zend_hash_update(visited_items, item_label, strlen(item_label), &isset, sizeof(int), NULL);
    }

    return found;
}

void meminfo_browse_hash_with_size(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element)
{
    zval **zval;
    char *key;

    HashPosition pos;
    ulong num_key;
    uint key_len;
    int i;

    int first_child = 1;

    php_stream_printf(stream TSRMLS_CC, "        \"children\" : {\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zend_hash_get_current_data_ex(ht, (void **) &zval, &pos) == SUCCESS) {

        if (!first_child) {
            php_stream_printf(stream TSRMLS_CC, ",\n");
        } else {
            first_child = 0;
        }

        switch (zend_hash_get_current_key_ex(ht, &key, &key_len, &num_key, 0, &pos)) {
            case HASH_KEY_IS_STRING:

                if (is_object) {
                    const char *property_name, *class_name;
                    int mangled = zend_unmangle_property_name(key, key_len - 1, &class_name, &property_name);

                    strcpy(key, property_name);
                }
                php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", meminfo_escape_for_json(key), *zval );
                break;
            case HASH_KEY_IS_LONG:
                php_stream_printf(stream TSRMLS_CC, "            \"%ld\":\"%p\"", num_key, *zval );
                break;
        }

        zend_hash_move_forward_ex(ht, &pos);
    }
    php_stream_printf(stream TSRMLS_CC, "\n        }\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zend_hash_get_current_data_ex(ht, (void **) &zval, &pos) == SUCCESS) {
        meminfo_browse_zval_with_size(stream, *zval, visited_items, first_element);
        zend_hash_move_forward_ex(ht, &pos);
    }
}

void meminfo_browse_zval_with_size(php_stream * stream, zval * zv, HashTable *visited_items, int *first_element)
{
    char zval_id[16];
    sprintf(zval_id, "%p", zv);

    if (meminfo_visit_item(zval_id, visited_items)) {
        return;
    }

    if (! *first_element) {
        php_stream_printf(stream TSRMLS_CC, "\n    },\n");
    } else {
        *first_element = 0;
    }

    php_stream_printf(stream TSRMLS_CC, "    \"%s\" : {\n", zval_id);
    php_stream_printf(stream TSRMLS_CC, "        \"type\" : \"%s\",\n", zend_get_type_by_const(Z_TYPE_P(zv)));
    php_stream_printf(stream TSRMLS_CC, "        \"size\" : \"%ld\"", meminfo_get_element_size(zv));

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        HashTable *properties;

        properties = NULL;

        int is_temp;

        php_stream_printf(stream TSRMLS_CC, ",\n");
        php_stream_printf(stream TSRMLS_CC, "        \"class\" : \"%s\",\n", meminfo_escape_for_json(meminfo_get_classname(zv->value.obj.handle)));

        properties = Z_OBJDEBUG_P(zv, is_temp);

        if (properties != NULL) {
            meminfo_browse_hash_with_size(stream, properties, 1, visited_items, first_element);

            if (is_temp) {
                zend_hash_destroy(properties);
                efree(properties);
            }
        }
    } else if (Z_TYPE_P(zv) == IS_ARRAY) {
        php_stream_printf(stream TSRMLS_CC, ",\n");
        meminfo_browse_hash_with_size(stream, zv->value.ht, 0, visited_items, first_element);
    } else {
        php_stream_printf(stream TSRMLS_CC, "\n");
    }

}
/**
 * Get size of an element
 *
 * @param zval *zv Zval of the element
 *
 * @return zend_ulong
 */
zend_ulong meminfo_get_element_size(zval *zv)
{
    zend_ulong size;

    size = sizeof(zval);

    switch (Z_TYPE_P(zv)) {
        case IS_STRING:
            size += zv->value.str.len;
            break;

        case IS_ARRAY:
            size += sizeof(HashTable);
            break;

        case IS_OBJECT:
            size += sizeof(zend_object);
            break;
    }

    return size;
}

/**
 * Escape the \ and " characters for JSON encoding
 */
char * meminfo_escape_for_json(const char *s)
{
    int new_str_len;
    char *s1;

    s1 = php_str_to_str(s, strlen(s), "\\", 1, "\\\\", 2, &new_str_len);

    return  php_str_to_str(s1, strlen(s1), "\"", 1, "\\\"", 2, &new_str_len);
}

/**
 * Generate a JSON header for the meminfo
 *
 */
char * meminfo_size_info_generate_header(char * header)
{
    size_t memory_usage;
    size_t memory_usage_real;
    size_t peak_memory_usage;
    size_t peak_memory_usage_real;

    memory_usage = zend_memory_usage(0);
    memory_usage_real = zend_memory_usage(1);

    peak_memory_usage = zend_memory_peak_usage(0);
    peak_memory_usage_real = zend_memory_peak_usage(1);

    snprintf(
        header,
        1024,
        "{\n\
            \"memory_usage\":%d,\n\
            \"memory_usage_real\":%d,\n\
            \"peak_memory_usage\":%d,\n\
            \"peak_memory_usage_real\":%d\n\
        }",
        memory_usage,
        memory_usage_real,
        peak_memory_usage,
        peak_memory_usage_real
    );

    return header;
}
