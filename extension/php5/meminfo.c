#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_meminfo.h"

#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "zend_extensions.h"
#include "zend_exceptions.h"
#include "Zend/zend_compile.h"

#include "zend.h"
#include "SAPI.h"
#include "zend_API.h"


const zend_function_entry meminfo_functions[] = {
    PHP_FE(meminfo_dump, NULL)
    {NULL, NULL, NULL}
};

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


/**
 * Generate a JSON output of the list of items in memory (objects, arrays, string, etc...)
 * with their sizes and other information
 */
PHP_FUNCTION(meminfo_dump)
{
    zval *zval_stream;
    zend_execute_data *exec_frame;
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

    php_stream_from_zval(stream, &zval_stream);
    php_stream_printf(stream TSRMLS_CC, "{\n");

    php_stream_printf(stream TSRMLS_CC, "\"header\":\n");
    php_stream_printf(stream TSRMLS_CC, meminfo_dump_header(header, sizeof(header) TSRMLS_CC));
    php_stream_printf(stream TSRMLS_CC, ",\n");

    php_stream_printf(stream TSRMLS_CC, "\"items\": {\n");

    ALLOC_HASHTABLE(visited_items);
    zend_hash_init(visited_items, 1000, NULL, NULL, 0);

    exec_frame = EG(current_execute_data);
    init_exec_frame = exec_frame;
    while (exec_frame && !exec_frame->op_array) {
        exec_frame = exec_frame->prev_execute_data;
    }

    while (exec_frame) {
        int i;
        zend_op_array * op_array;
        // Switch the active frame to the current browsed one and rebuild the symbol table
        // to get it right
        EG(current_execute_data) = exec_frame;
        zend_rebuild_symbol_table(TSRMLS_C);
        symbol_table = EG(active_symbol_table);

        // Once we have the symbol table, switch to the prev frame to get the right frame name
        exec_frame = exec_frame->prev_execute_data;

        if (exec_frame) {
            meminfo_build_frame_label(frame_label, sizeof(frame_label), exec_frame TSRMLS_CC);

            meminfo_browse_zvals_from_symbol_table(stream, frame_label, symbol_table, visited_items, &first_element TSRMLS_CC);

            exec_frame = exec_frame->prev_execute_data;
        }
    }
    EG(current_execute_data) = init_exec_frame;
    zend_rebuild_symbol_table(TSRMLS_C);

    global_symbol_table = &EG(symbol_table);

    strcpy(frame_label, "<GLOBAL>");

    meminfo_browse_zvals_from_symbol_table(stream, frame_label, global_symbol_table, visited_items, &first_element TSRMLS_CC);

    php_stream_printf(stream TSRMLS_CC, "\n    }\n");
    php_stream_printf(stream TSRMLS_CC, "}\n}\n");

    zend_hash_destroy(visited_items);
    FREE_HASHTABLE(visited_items);
}

/**
 * Return the class associated to the provided object handle
 *
 * @param zend_object_handle object handle
 *
 * @return char * class name
 */
const char * meminfo_get_classname(zend_object_handle handle TSRMLS_DC)
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

void meminfo_browse_zvals_from_symbol_table(php_stream *stream, char* frame_label, HashTable *symbol_table, HashTable * visited_items, int *first_element TSRMLS_DC)
{
    zval **zval_to_dump;
    HashPosition pos;

    char *key;
    ulong num_key;
    uint key_len;

    zend_hash_internal_pointer_reset_ex(symbol_table, &pos);
    while (zend_hash_get_current_data_ex(symbol_table, (void **) &zval_to_dump, &pos) == SUCCESS) {

        zend_hash_get_current_key_ex(symbol_table, &key, &key_len, &num_key, 0, &pos);

        meminfo_zval_dump(stream, frame_label, key, *zval_to_dump, visited_items, first_element TSRMLS_CC);

        zend_hash_move_forward_ex(symbol_table, &pos);
    }
}

int meminfo_visit_item(const char * item_label, HashTable *visited_items TSRMLS_DC)
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

void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element TSRMLS_DC)
{
    zval **zval;
    char *key, *char_buf;;

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
                    zend_unmangle_property_name(key, key_len - 1, &class_name, &property_name);
                    char_buf = meminfo_escape_for_json(property_name TSRMLS_CC);
                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", char_buf, *zval );
                    efree(char_buf);
                } else {
                    char_buf = meminfo_escape_for_json(key TSRMLS_CC);
                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", char_buf, *zval );
                    efree(char_buf);
                }

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
        meminfo_zval_dump(stream, NULL, NULL, *zval, visited_items, first_element TSRMLS_CC);
        zend_hash_move_forward_ex(ht, &pos);
    }
}

void meminfo_zval_dump(php_stream * stream, char * frame_label, char * symbol_name, zval * zv, HashTable *visited_items, int *first_element TSRMLS_DC)
{
    char zval_id[16];
    char *char_buf;
    sprintf(zval_id, "%p", zv);

    if (meminfo_visit_item(zval_id, visited_items TSRMLS_CC)) {
        return;
    }

    if (! *first_element) {
        php_stream_printf(stream TSRMLS_CC, "\n    },\n");
    } else {
        *first_element = 0;
    }

    php_stream_printf(stream TSRMLS_CC, "    \"%s\" : {\n", zval_id);
    php_stream_printf(stream TSRMLS_CC, "        \"type\" : \"%s\",\n", zend_get_type_by_const(Z_TYPE_P(zv)));
    php_stream_printf(stream TSRMLS_CC, "        \"size\" : \"%ld\",\n", meminfo_get_element_size(zv TSRMLS_CC));

    if (frame_label) {
        if (symbol_name) {
            char_buf = meminfo_escape_for_json(symbol_name TSRMLS_CC);
            php_stream_printf(stream TSRMLS_CC, "        \"symbol_name\" : \"%s\",\n", char_buf);
            efree(char_buf);
        }
        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : true,\n");
        char_buf = meminfo_escape_for_json(frame_label TSRMLS_CC);
        php_stream_printf(stream TSRMLS_CC, "        \"frame\" : \"%s\"\n", char_buf);
        efree(char_buf);
    } else {
        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : false\n");
    }

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        HashTable *properties;

        properties = NULL;

        int is_temp;

        php_stream_printf(stream TSRMLS_CC, ",\n");
        char_buf = meminfo_escape_for_json(meminfo_get_classname(zv->value.obj.handle TSRMLS_CC) TSRMLS_CC);
        php_stream_printf(stream TSRMLS_CC, "        \"class\" : \"%s\",\n", char_buf);
        efree(char_buf);
        php_stream_printf(stream TSRMLS_CC, "        \"object_handle\" : \"%d\",\n", zv->value.obj.handle);

        properties = Z_OBJDEBUG_P(zv, is_temp);

        if (properties != NULL) {
            meminfo_hash_dump(stream, properties, 1, visited_items, first_element TSRMLS_CC);

            if (is_temp) {
                zend_hash_destroy(properties);
                efree(properties);
            }
        }
    } else if (Z_TYPE_P(zv) == IS_ARRAY) {
        php_stream_printf(stream TSRMLS_CC, ",\n");
        meminfo_hash_dump(stream, zv->value.ht, 0, visited_items, first_element TSRMLS_CC);
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
zend_ulong meminfo_get_element_size(zval *zv TSRMLS_DC)
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
 * Build the current frame label based on function name and object class
 * if necessary
 */
void meminfo_build_frame_label(char* frame_label, int frame_label_len, zend_execute_data* frame TSRMLS_DC)
{
    const char* function_name;
    const char *class_name = NULL;
    const char *free_class_name = NULL;
    char *call_type;

    function_name = (frame->function_state.function->common.scope &&
        frame->function_state.function->common.scope->trait_aliases) ?
            zend_resolve_method_name(
                    frame->object ?
                        Z_OBJCE_P(frame->object) :
                        frame->function_state.function->common.scope,
                    frame->function_state.function) :
                frame->function_state.function->common.function_name;

    if (function_name) {
        if (frame->object) {
            if (frame->function_state.function->common.scope) {
                class_name = frame->function_state.function->common.scope->name;
            } else {
                zend_uint class_name_len;
                int dup;

                dup = zend_get_object_classname(frame->object, &class_name, &class_name_len TSRMLS_CC);
                if(!dup) {
                    free_class_name = class_name;
                }
            }

            call_type = "->";
        } else if (frame->function_state.function->common.scope) {
            class_name = frame->function_state.function->common.scope->name;
            call_type = "::";
        } else {
            class_name = NULL;
            call_type = NULL;
        }
    } else {

        if (!frame->opline || frame->opline->opcode != ZEND_INCLUDE_OR_EVAL) {
            /* can happen when calling eval from a custom sapi */
            function_name = "unknown";
        } else
        switch (frame->opline->extended_value) {
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
                /* this can actually happen if you use debug_backtrace() in your error_handler and
                 * you're in the top-scope */
                function_name = "unknown";
                break;
        }

        call_type = NULL;
    }

    if (NULL == class_name) {
        class_name = "";
    }

    if (NULL == call_type) {
        call_type = "";
    }

    snprintf(frame_label, frame_label_len, "%s%s%s()", class_name, call_type, function_name);

    if (free_class_name) {
        efree((char*)free_class_name);
    }
}

/**
 * Escape the \ and " characters for JSON encoding
 */
char * meminfo_escape_for_json(const char *s TSRMLS_DC)
{
    int new_str_len;
    char *s1, *s2;

    s1 = php_str_to_str((char *) s, strlen(s), "\\", 1, "\\\\", 2, &new_str_len);
    s2 = php_str_to_str(s1, strlen(s1), "\"", 1, "\\\"", 2, &new_str_len);

    if (s1) {
        efree(s1);
    }

    return s2;
}

/**
 * Generate a JSON header for the meminfo
 *
 */
char * meminfo_dump_header(char * header, int header_len TSRMLS_DC)
{
    size_t memory_usage;
    size_t memory_usage_real;
    size_t peak_memory_usage;
    size_t peak_memory_usage_real;

    memory_usage = zend_memory_usage(0 TSRMLS_CC);
    memory_usage_real = zend_memory_usage(1 TSRMLS_CC);

    peak_memory_usage = zend_memory_peak_usage(0 TSRMLS_CC);
    peak_memory_usage_real = zend_memory_peak_usage(1 TSRMLS_CC);

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
