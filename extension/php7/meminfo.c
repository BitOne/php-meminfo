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
    PHP_FE_END
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
    zend_array *p_symbol_table;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, zval_stream);
    php_stream_printf(stream TSRMLS_CC, "{\n");

    php_stream_printf(stream TSRMLS_CC, "\"header\":\n");
    php_stream_printf(stream TSRMLS_CC, meminfo_dump_header(header, sizeof(header)));
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
        p_symbol_table = zend_rebuild_symbol_table();

        // Once we have the symbol table, switch to the prev frame to get the right frame name
        prev_frame = exec_frame->prev_execute_data;

        if (prev_frame) {
            if (prev_frame->prev_execute_data) {
                meminfo_build_frame_label(frame_label, sizeof(frame_label), prev_frame);
            } else {
                snprintf(frame_label, sizeof(frame_label), "<GLOBAL>");
            }
        }

        meminfo_browse_zvals_from_symbol_table(stream, frame_label, p_symbol_table, visited_items, &first_element);

        exec_frame = exec_frame->prev_execute_data;
    }

    php_stream_printf(stream TSRMLS_CC, "\n    }\n");
    php_stream_printf(stream TSRMLS_CC, "}\n}\n");

    zend_hash_destroy(visited_items);
    FREE_HASHTABLE(visited_items);
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
    } else {
        zend_hash_add_new(visited_items, zstr_label, &isset);
    }
    zend_string_release(zstr_label);

    return found;
}

void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element)
{
    zval *zval;

    zend_string *key;
    HashPosition pos;
    zend_ulong num_key;
    int i;

    int first_child = 1;

    php_stream_printf(stream TSRMLS_CC, "        \"children\" : {\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zval = zend_hash_get_current_data_ex(ht, &pos)) {

        if (!first_child) {
            php_stream_printf(stream TSRMLS_CC, ",\n");
        } else {
            first_child = 0;
        }

        switch (zend_hash_get_current_key_ex(ht, &key, &num_key, &pos)) {
            case HASH_KEY_IS_STRING:

                if (is_object) {
                    const char *property_name, *class_name;
                    zend_string * escaped_property_name;

                    zend_unmangle_property_name(key, &class_name, &property_name);

                    escaped_property_name = meminfo_escape_for_json(property_name);

                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", ZSTR_VAL(escaped_property_name), zval);

                    zend_string_release(escaped_property_name);
                } else {
                    zend_string * escaped_key;

                    escaped_key = meminfo_escape_for_json(ZSTR_VAL(key));

                    php_stream_printf(stream TSRMLS_CC, "            \"%s\":\"%p\"", ZSTR_VAL(escaped_key), zval);

                    zend_string_release(escaped_key);
                }

                break;
            case HASH_KEY_IS_LONG:
                php_stream_printf(stream TSRMLS_CC, "            \"%ld\":\"%p\"", num_key, zval);
                break;
        }

        zend_hash_move_forward_ex(ht, &pos);
    }
    php_stream_printf(stream TSRMLS_CC, "\n        }\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while (zval = zend_hash_get_current_data_ex(ht, &pos)) {
        meminfo_zval_dump(stream, NULL, NULL, zval, visited_items, first_element);
        zend_hash_move_forward_ex(ht, &pos);
    }
}

void meminfo_zval_dump(php_stream * stream, char * frame_label, zend_string * symbol_name, zval * zv, HashTable *visited_items, int *first_element)
{
    char zval_id[16];

    if (Z_TYPE_P(zv) == IS_INDIRECT) {
        zv = Z_INDIRECT_P(zv);
    }

    if (Z_ISREF_P(zv)) {
        ZVAL_DEREF(zv);
    }

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
    php_stream_printf(stream TSRMLS_CC, "        \"size\" : \"%ld\",\n", meminfo_get_element_size(zv));

    if (frame_label) {
        zend_string * escaped_frame_label;

        if (symbol_name) {
            zend_string * escaped_symbol_name;

            escaped_symbol_name = meminfo_escape_for_json(ZSTR_VAL(symbol_name));

            php_stream_printf(stream TSRMLS_CC, "        \"symbol_name\" : \"%s\",\n", ZSTR_VAL(escaped_symbol_name));

            zend_string_release(escaped_symbol_name);
        }

        escaped_frame_label = meminfo_escape_for_json(frame_label);

        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : true,\n");
        php_stream_printf(stream TSRMLS_CC, "        \"frame\" : \"%s\"\n", ZSTR_VAL(escaped_frame_label));

        zend_string_release(escaped_frame_label);
    } else {
        php_stream_printf(stream TSRMLS_CC, "        \"is_root\" : false\n");
    }

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        HashTable *properties;
        int is_temp;
        zend_string * escaped_class_name;

        properties = NULL;

        escaped_class_name = meminfo_escape_for_json(ZSTR_VAL(zv->value.obj->ce->name));

        php_stream_printf(stream TSRMLS_CC, ",\n");
        php_stream_printf(stream TSRMLS_CC, "        \"class\" : \"%s\",\n", ZSTR_VAL(escaped_class_name));

        zend_string_release(escaped_class_name);

        php_stream_printf(stream TSRMLS_CC, "        \"object_handle\" : \"%d\",\n", zv->value.obj->handle);

        properties = Z_OBJDEBUG_P(zv, is_temp);

        if (properties != NULL) {
            meminfo_hash_dump(stream, properties, 1, visited_items, first_element);

            if (is_temp) {
                zend_hash_destroy(properties);
                efree(properties);
            }
        }
    } else if (Z_TYPE_P(zv) == IS_ARRAY) {
        php_stream_printf(stream TSRMLS_CC, ",\n");
        meminfo_hash_dump(stream, zv->value.arr, 0, visited_items, first_element);
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
            size += zv->value.str->len;
            break;

        // TODO: add size of the indexes
        case IS_ARRAY:
            size += sizeof(HashTable);
            break;

        // TODO: add size of the properties table, but without property content
        case IS_OBJECT:
            size += sizeof(zend_object);
            break;
    }

    return size;
}

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
 * Escape for JSON encoding
 */
zend_string * meminfo_escape_for_json(const char *s)
{
    int new_str_len, i;
    char unescaped_char[2];
    char escaped_char[7]; // \uxxxx format
    zend_string *s1, *s2, *s3 = NULL;

    s1 = php_str_to_str((char *) s, strlen(s), "\\", 1, "\\\\", 2);
    s2 = php_str_to_str(ZSTR_VAL(s1), ZSTR_LEN(s1), "\"", 1, "\\\"", 2);

    for (i = 0; i <= 0x1f; i++) {
        unescaped_char[0] =  (char) i;
        sprintf(escaped_char, "\\u%04x", i);
        if (s3) {
            s2 = s3;
        }
        s3 = php_str_to_str(ZSTR_VAL(s2), ZSTR_LEN(s2), unescaped_char, 1, escaped_char, 6);
        zend_string_release(s2);
    }

    zend_string_release(s1);

    return s3;
}

/**
 * Generate a JSON header for the meminfo
 *
 */
char * meminfo_dump_header(char * header, int header_len)
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
