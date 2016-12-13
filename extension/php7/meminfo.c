#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "php_meminfo.h"

#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "zend_extensions.h"
#include "zend_exceptions.h"
#include "Zend/zend_compile.h"

#include "zend.h"
#include "SAPI.h"
#include "zend_API.h"
#include "zend_types.h"


const zend_function_entry meminfo_functions[] = {
    PHP_FE(meminfo_structs_size, NULL)
    PHP_FE(meminfo_objects_list, NULL)
    PHP_FE(meminfo_objects_summary, NULL)
    PHP_FE(meminfo_info_dump, NULL)
    PHP_FE_END
};

/*
    PHP_FE(meminfo_gc_roots_list, NULL)
    {NULL, NULL, NULL}
};*/

zend_module_entry meminfo_module_entry = {
    STANDARD_MODULE_HEADER,
    "meminfo",
    meminfo_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    MEMINFO_VERSION,
    STANDARD_MODULE_PROPERTIES
};

PHP_FUNCTION(meminfo_structs_size)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, zval_stream);

    php_stream_printf(stream, "Simple Zend Type size on this platform\n");
    php_stream_printf(stream, "  Zend Signed Integer (zend_long): %ld bytes.\n", sizeof(zend_long));
    php_stream_printf(stream, "  Zend Unsigned Integer (zend_ulong): %ld bytes.\n", sizeof(zend_ulong));
    php_stream_printf(stream, "  Zend Unsigned Char (zend_uchar): %ld bytes.\n", sizeof(zend_uchar));

    php_stream_printf(stream, "Structs size on this platform:\n");
    php_stream_printf(stream, "  Variable (zval): %ld bytes.\n", sizeof(zval));
    php_stream_printf(stream, "  Class (zend_class_entry): %ld bytes.\n", sizeof(zend_class_entry));
    php_stream_printf(stream, "  Object (zend_object): %ld bytes.\n", sizeof(zend_object));
}

PHP_FUNCTION(meminfo_objects_list)
{
    zval *zval_stream;
    php_stream *stream;
    uint32_t total_objects_buckets;
    uint32_t current_objects = 0;
    zend_string *class_name;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, zval_stream);

    php_stream_printf(stream, "Objects list:\n");

    zend_objects_store *objects = &EG(objects_store);
    total_objects_buckets = objects->top - 1;

    if (objects->top > 1) {
        zend_object **obj_ptr = objects->object_buckets + 1;
        zend_object **end = objects->object_buckets + objects->top;

        do {
            zend_object *obj = *obj_ptr;

            if (IS_OBJ_VALID(obj)) {
                if (!(GC_FLAGS(obj) & IS_OBJ_DESTRUCTOR_CALLED)) {
                    php_stream_printf(stream TSRMLS_CC, "  - Class %s, handle %d, refCount %d\n", ZSTR_VAL(obj->ce->name), obj->handle, GC_REFCOUNT(obj));
                    current_objects++;
                }
            }
            obj_ptr++;
        } while (obj_ptr != end);
    }

    php_stream_printf(stream, "Total object buckets: %d. Current objects: %d.\n", total_objects_buckets, current_objects);
}

PHP_FUNCTION(meminfo_objects_summary)
{
    zval *zval_stream = NULL;
    php_stream *stream = NULL;

    HashTable *classes = NULL;

    zval *p_instances_count;
    zend_string *class_name;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }
    php_stream_from_zval(stream, zval_stream);

    ALLOC_HASHTABLE(classes);
    zend_hash_init(classes, 1000, NULL, NULL, 0);

    zend_objects_store *objects = &EG(objects_store);

    if (objects->top > 1) {
        zend_object **obj_ptr = objects->object_buckets + 1;
        zend_object **end = objects->object_buckets + objects->top;

        do {
            zend_object *obj = *obj_ptr;

            if (IS_OBJ_VALID(obj)) {
                if (!(GC_FLAGS(obj) & IS_OBJ_DESTRUCTOR_CALLED)) {

                    class_name = obj->ce->name;

                    p_instances_count = zend_hash_find(classes, class_name);

                    if (p_instances_count != NULL) {
                        p_instances_count->value.lval++;
                    } else {
                        zval instances_count;

                        p_instances_count = &instances_count;
                        p_instances_count->value.lval = 1;
                        zend_hash_add(classes, class_name, p_instances_count);
                    }
                }
            }
            obj_ptr++;
        } while (obj_ptr != end);
    }

    zend_hash_sort(classes, meminfo_instances_count_compare, 0 TSRMLS_CC);


    uint32_t rank = 1;
    ulong index;

    zend_hash_internal_pointer_reset(classes);

    php_stream_printf(stream TSRMLS_CC, "Instances count by class:\n");

    php_stream_printf(stream TSRMLS_CC, "%-12s %-12s %s\n", "rank", "#instances", "class");
    php_stream_printf(stream TSRMLS_CC, "-----------------------------------------------------------------\n");

    while (zend_hash_has_more_elements(classes) == SUCCESS) {
        p_instances_count = zend_hash_get_current_data(classes);
        zend_hash_get_current_key(classes, &class_name, &index);

        php_stream_printf(stream TSRMLS_CC, "%-12d %-12d %s\n", rank, p_instances_count->value.lval, class_name->val);

        zend_hash_move_forward(classes);
        rank++;
    }

    zend_hash_destroy(classes);
    zend_hash_clean(classes);
    FREE_HASHTABLE(classes);
}


//PHP_FUNCTION(meminfo_gc_roots_list)
//{
//    zval *zval_stream;
//    php_stream *stream;
//    zval* pz;
//
//    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
//        return;
//    }
//
//    php_stream_from_zval(stream, &zval_stream);
//    php_stream_printf(stream TSRMLS_CC, "GC roots list:\n");
//
//    gc_root_buffer *current = GC_G(roots).next;
//
//    while (current != &GC_G(roots)) {
//        pz = current->u.pz;
//        php_stream_printf(stream TSRMLS_CC, "  zval pointer: %p ", (void *) pz);
//        if (current->handle) {
//            php_stream_printf(
//                stream TSRMLS_CC,
//                "  Class %s, handle %d\n",
//                meminfo_get_classname(current->handle),
//                current->handle);
//        } else {
//            php_stream_printf(stream TSRMLS_CC, "  Type: %s",  zend_get_type_by_const(Z_TYPE_P(pz)));
//            php_stream_printf(stream TSRMLS_CC, ", Ref count GC %d\n", pz->refcount__gc);
//
//        }
//        current = current->next;
//
//    }
//}

/**
 * Generate a JSON output of the list of items in memory (objects, arrays, string, etc...)
 * with their sizes and other information
 */
PHP_FUNCTION(meminfo_info_dump)
{
    zval *zval_stream;
    zend_execute_data *exec_frame, *prev_frame;
    zend_execute_data *init_exec_frame;
    HashTable *global_symbol_table;
    HashTable *symbol_table;
    char header[1024];
    char frame_label[500];

    int first_element = 1;
    int i;

    php_stream *stream;
    HashTable *visited_items;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, zval_stream);
    php_stream_printf(stream TSRMLS_CC, "{\n");

    php_stream_printf(stream TSRMLS_CC, "\"header\":\n");
    php_stream_printf(stream TSRMLS_CC, meminfo_info_dump_header(header, sizeof(header)));
    php_stream_printf(stream TSRMLS_CC, ",\n");

    php_stream_printf(stream TSRMLS_CC, "\"items\": {\n");

    ALLOC_HASHTABLE(visited_items);
    zend_hash_init(visited_items, 1000, NULL, NULL, 0);

    exec_frame = EG(current_execute_data);
    init_exec_frame = exec_frame;

    while (exec_frame) {
        int i;
        zend_op_array * op_array;
        // Switch the active frame to the current browsed one and rebuild the symbol table
        // to get it right
        EG(current_execute_data) = exec_frame;
        zend_rebuild_symbol_table();

        // Once we have the symbol table, switch to the prev frame to get the right frame name
        prev_frame = exec_frame->prev_execute_data;

        if (prev_frame) {
            if (prev_frame->prev_execute_data) {
                meminfo_build_frame_label(frame_label, sizeof(frame_label), prev_frame);
            } else {
                snprintf(frame_label, sizeof(frame_label), "<GLOBAL>");
            }
        }

        meminfo_browse_zvals_from_symbol_table(stream, frame_label, &EG(symbol_table), visited_items, &first_element);

        exec_frame = exec_frame->prev_execute_data;
    }

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

    bucket_a = *((Bucket **) a);
    bucket_b = *((Bucket **) b);

    zend_long instances_count_a;
    zend_long instances_count_b;

    instances_count_a = (zend_long) &(bucket_a)->val.value.lval;
    instances_count_b = (zend_long) &(bucket_b)->val.value.lval;

    if (instances_count_a > instances_count_b) {
        return -1;
    } else if (instances_count_a == instances_count_b) {
        return 0;
    } else {
        return 1;
    }
}


void meminfo_browse_zvals_from_symbol_table(php_stream *stream, char* frame_label, HashTable *p_symbol_table, HashTable * visited_items, int *first_element)
{
    zval *zval_to_dump;
    HashPosition pos;

    zend_string *key;
    ulong index;
    uint key_len;

    zend_hash_internal_pointer_reset_ex(p_symbol_table, &pos);

    while (zval_to_dump = zend_hash_get_current_data_ex(p_symbol_table, &pos)) {

        zend_hash_get_current_key_ex(p_symbol_table, &key, &index, &pos);

        meminfo_zval_dump(stream, frame_label, key, zval_to_dump, visited_items, first_element);

        zend_hash_move_forward_ex(p_symbol_table, &pos);
    }
}

int meminfo_visit_item(char * item_label, HashTable *visited_items)
{
    int found = 0;
    zval isset;
    zend_string * zstr_label;

    zstr_label = zend_string_init(item_label, strlen(item_label), 0);

    isset.value.lval = 1;

    if (zend_hash_exists(visited_items, zstr_label)) {
        found = 1;
        zend_string_release(zstr_label);
    } else {
        zend_hash_add_new(visited_items, zstr_label, &isset);
    }

    return found;
}

//void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element)
//{
//    zval **zval;
//    char *key;
//
//    HashPosition pos;
//    ulong num_key;
//    uint key_len;
//    int i;
//
//    int first_child = 1;
//
//    php_stream_printf(stream TSRMLS_CC, "        \"children\" : {\n");
//
//    zend_hash_internal_pointer_reset_ex(ht, &pos);
//    while (zend_hash_get_current_data_ex(ht, (void **) &zval, &pos) == SUCCESS) {
//
//        if (!first_child) {
//            php_stream_printf(stream TSRMLS_CC, ",\n");
//        } else {
//            first_child = 0;
//        }
//
//        switch (zend_hash_get_current_key_ex(ht, &key, &key_len, &num_key, 0, &pos)) {
//            case HASH_KEY_IS_STRING:
//
//                if (is_object) {
//                    const char *property_name, *class_name;
//                    int mangled = zend_unmangle_property_name(key, key_len - 1, &class_name, &property_name);
//
//                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", meminfo_escape_for_json(property_name), *zval );
//                } else {
//                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", meminfo_escape_for_json(key), *zval );
//                }
//
//                break;
//            case HASH_KEY_IS_LONG:
//                php_stream_printf(stream TSRMLS_CC, "            \"%ld\":\"%p\"", num_key, *zval );
//                break;
//        }
//
//        zend_hash_move_forward_ex(ht, &pos);
//    }
//    php_stream_printf(stream TSRMLS_CC, "\n        }\n");
//
//    zend_hash_internal_pointer_reset_ex(ht, &pos);
//    while (zend_hash_get_current_data_ex(ht, (void **) &zval, &pos) == SUCCESS) {
//        meminfo_zval_dump(stream, NULL, NULL, *zval, visited_items, first_element);
//        zend_hash_move_forward_ex(ht, &pos);
//    }
//}
//
void meminfo_zval_dump(php_stream * stream, char * frame_label, zend_string * symbol_name, zval * zv, HashTable *visited_items, int *first_element)
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
//    php_stream_printf(stream TSRMLS_CC, "        \"size\" : \"%ld\",\n", meminfo_get_element_size(zv));

    if (frame_label) {
        if (symbol_name) {
            php_stream_printf(stream TSRMLS_CC, "        \"symbol_name\" : \"%s\",\n", meminfo_escape_for_json(ZSTR_VAL(symbol_name)));
        }
        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : true,\n");
        php_stream_printf(stream TSRMLS_CC, "        \"frame\" : \"%s\"\n", meminfo_escape_for_json(frame_label));
    } else {
        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : false\n");
    }

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        HashTable *properties;

        properties = NULL;

        int is_temp;

        php_stream_printf(stream TSRMLS_CC, ",\n");
        php_stream_printf(stream TSRMLS_CC, "        \"class\" : \"%s\",\n", meminfo_escape_for_json(ZSTR_VAL(zv->value.obj->ce->name)));
        php_stream_printf(stream TSRMLS_CC, "        \"object_handle\" : \"%d\",\n", zv->value.obj->handle);

        properties = Z_OBJDEBUG_P(zv, is_temp);

        if (properties != NULL) {
            //meminfo_hash_dump(stream, properties, 1, visited_items, first_element);

            if (is_temp) {
                zend_hash_destroy(properties);
                efree(properties);
            }
        }
    } else if (Z_TYPE_P(zv) == IS_ARRAY) {
        php_stream_printf(stream TSRMLS_CC, ",\n");
        //meminfo_hash_dump(stream, zv->value.arr, 0, visited_items, first_element);
    } else {
        php_stream_printf(stream TSRMLS_CC, "\n");
    }
}
///**
// * Get size of an element
// *
// * @param zval *zv Zval of the element
// *
// * @return zend_ulong
// */
//zend_ulong meminfo_get_element_size(zval *zv)
//{
//    zend_ulong size;
//
//    size = sizeof(zval);
//
//    switch (Z_TYPE_P(zv)) {
//        case IS_STRING:
//            size += zv->value.str.len;
//            break;
//
//        case IS_ARRAY:
//            size += sizeof(HashTable);
//            break;
//
//        case IS_OBJECT:
//            size += sizeof(zend_object);
//            break;
//    }
//
//    return size;
//}
//
/**
 * Build the current frame label based on function name and object class
 * if necessary.
 *
 * Most code comes from the debug_print_backtrace implementation.
 */
void meminfo_build_frame_label(char* frame_label, int frame_label_len, zend_execute_data* frame)
{
    zend_function *func;
    const char *function_name;
    char * call_type;
    zend_string *class_name = NULL;
    zend_object *object;
    zend_execute_data *ptr;

    object = Z_OBJ(frame->This);
    ptr = frame->prev_execute_data;

    if (frame->func) {
        func = frame->func;
        function_name = (func->common.scope &&
                         func->common.scope->trait_aliases) ?
            ZSTR_VAL(zend_resolve_method_name(
                (object ? object->ce : func->common.scope), func)) :
            (func->common.function_name ?
                ZSTR_VAL(func->common.function_name) : NULL);
    } else {
        func = NULL;
        function_name = NULL;
    }

    if (function_name) {
        if (object) {
            if (func->common.scope) {
                class_name = func->common.scope->name;
            } else {
                class_name = object->ce->name;
            }

            call_type = "->";
        } else if (func->common.scope) {
            class_name = func->common.scope->name;
            call_type = "::";
        } else {
            class_name = NULL;
            call_type = NULL;
        }
    } else {

        if (!ptr->func || !ZEND_USER_CODE(ptr->func->common.type) || ptr->opline->opcode != ZEND_INCLUDE_OR_EVAL) {
            /* can happen when calling eval from a custom sapi */
            function_name = "unknown";
        } else
        switch (ptr->opline->extended_value) {
            case ZEND_EVAL:
                function_name = "eval";
                break;
            case ZEND_INCLUDE:
                function_name = "include";
                break;
            case ZEND_REQUIRE:
                function_name = "require";
                break;
            case ZEND_INCLUDE_ONCE:
                function_name = "include_once";
                break;
            case ZEND_REQUIRE_ONCE:
                function_name = "require_once";
                break;
            default:
                /* this can actually happen if you're in your error_handler and
                 * you're in the top-scope */
                function_name = "unknown";
                break;
        }
    }
    if (class_name) {
        snprintf(frame_label, frame_label_len, "%s%s%s()", ZSTR_VAL(class_name), call_type, function_name);
    } else {
        snprintf(frame_label, frame_label_len, "%s()", function_name);
    }
}

/**
 * Escape the \ and " characters for JSON encoding
 */
char * meminfo_escape_for_json(const char *s)
{
    int new_str_len;
    zend_string *s1, *s2;
    s1 = php_str_to_str((char *) s, strlen(s), "\\", 1, "\\\\", 2);
    s2 = php_str_to_str(ZSTR_VAL(s1), ZSTR_LEN(s1), "\"", 1, "\\\"", 2);

    if (s1) {
        zend_string_release(s1);
    }

    return ZSTR_VAL(s2);
}

/**
 * Generate a JSON header for the meminfo
 *
 */
char * meminfo_info_dump_header(char * header, int header_len)
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
        header_len,
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

#ifdef COMPILE_DL_MEMINFO
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(meminfo)
#endif
