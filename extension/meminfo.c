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

#if PHP_VERSION_ID >= 80000
ZEND_BEGIN_ARG_INFO_EX(arginfo_meminfo_dump, 0, 0, 1)
    ZEND_ARG_INFO(0, output_stream)
ZEND_END_ARG_INFO()

const zend_function_entry meminfo_functions[] = {
    PHP_FE(meminfo_dump, arginfo_meminfo_dump)
    PHP_FE_END
};
#else
const zend_function_entry meminfo_functions[] = {
    PHP_FE(meminfo_dump, NULL)
    PHP_FE_END
};
#endif

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

    int first_element = 1;

    php_stream *stream;
    HashTable visited_items;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &zval_stream) == FAILURE) {
        return;
    }

    zend_hash_init(&visited_items, 1000, NULL, NULL, 0);

    php_stream_from_zval(stream, zval_stream);
    php_stream_printf(stream, "{\n");

    php_stream_printf(stream, "  \"header\" : {\n");
    php_stream_printf(stream, "    \"memory_usage\" : %zd,\n", zend_memory_usage(0));
    php_stream_printf(stream, "    \"memory_usage_real\" : %zd,\n", zend_memory_usage(1));
    php_stream_printf(stream, "    \"peak_memory_usage\" : %zd,\n", zend_memory_peak_usage(0));
    php_stream_printf(stream, "    \"peak_memory_usage_real\" : %zd\n", zend_memory_peak_usage(1));
    php_stream_printf(stream, "  },\n");

    php_stream_printf(stream, "  \"items\": {\n");
    meminfo_browse_exec_frames(stream, &visited_items, &first_element);
    meminfo_browse_class_static_members(stream, &visited_items, &first_element);

    php_stream_printf(stream, "\n    }\n");
    php_stream_printf(stream, "}\n}\n");

    zend_hash_destroy(&visited_items);
}

/**
 * Go through all exec frames to gather declared variables and follow them to record items in memory
 */
void meminfo_browse_exec_frames(php_stream *stream,  HashTable *visited_items, int *first_element)
{
    zend_execute_data *exec_frame, *prev_frame;
    zend_array *p_symbol_table;

    exec_frame = EG(current_execute_data);

    char frame_label[500];

    // Skipping the frame of the meminfo_dump() function call
    exec_frame = exec_frame->prev_execute_data;

    while (exec_frame) {
        // Switch the active frame to the current browsed one and rebuild the symbol table
        // to get it right
        EG(current_execute_data) = exec_frame;

        // copy variables from ex->func->op_array.vars into the symbol table for the last called *user* function
        // therefore it does necessary returns the symbol table of the current frame 
        p_symbol_table = zend_rebuild_symbol_table();

        if (p_symbol_table != NULL) {

            if (exec_frame->prev_execute_data) {
                meminfo_build_frame_label(frame_label, sizeof(frame_label), exec_frame);
            } else {
                snprintf(frame_label, sizeof(frame_label), "<GLOBAL>");
            }

            meminfo_browse_zvals_from_symbol_table(stream, frame_label, p_symbol_table, visited_items, first_element);

        }
        exec_frame = exec_frame->prev_execute_data;
    }
}

/**
 * Go through static members of classes
 */
void meminfo_browse_class_static_members(php_stream *stream,  HashTable *visited_items, int *first_element)
{
    HashPosition ce_pos;
    HashPosition prop_pos;
    zend_class_entry *class_entry;
    zend_property_info * prop_info;

    char frame_label[500];
    char symbol_name[500];
    const char *prop_name, *class_name;
    zend_string * zstr_symbol_name;
    zval * prop;

    zend_hash_internal_pointer_reset_ex(CG(class_table), &ce_pos);
    while ((class_entry = zend_hash_get_current_data_ptr_ex(CG(class_table), &ce_pos)) != NULL) {

#if PHP_VERSION_ID >= 70400
        if (class_entry->default_static_members_count > 0 && CE_STATIC_MEMBERS(class_entry)) {
#else
        if (class_entry->static_members_table) {
#endif

            HashTable *properties_info = &(class_entry->properties_info);

            zend_hash_internal_pointer_reset_ex(properties_info, &prop_pos);

            while ((prop_info = zend_hash_get_current_data_ptr_ex(properties_info, &prop_pos)) != NULL) {

                if (prop_info->flags & ZEND_ACC_STATIC) {
                    snprintf(frame_label, sizeof(frame_label), "<CLASS_STATIC_MEMBER>");
#if PHP_VERSION_ID >= 70400
                    prop = CE_STATIC_MEMBERS(class_entry) + prop_info->offset;
#else
                    prop = &class_entry->static_members_table[prop_info->offset];
#endif

                    zend_unmangle_property_name(prop_info->name, &class_name, &prop_name);

                    if (class_name) {
                        snprintf(symbol_name, sizeof(frame_label), "%s::%s",  class_name, prop_name);
                    } else {
                        snprintf(symbol_name, sizeof(frame_label), "%s::%s",  ZSTR_VAL(class_entry->name), ZSTR_VAL(prop_info->name));
                    }

                    zstr_symbol_name = zend_string_init(symbol_name, strlen(symbol_name), 0);

                    meminfo_zval_dump(stream, frame_label, zstr_symbol_name, prop, visited_items, first_element);

                    zend_string_release(zstr_symbol_name);
                }

                zend_hash_move_forward_ex(properties_info, &prop_pos);
            }
        }

        zend_hash_move_forward_ex(CG(class_table), &ce_pos);
    }
}

void meminfo_browse_zvals_from_symbol_table(php_stream *stream, char* frame_label, HashTable *p_symbol_table, HashTable * visited_items, int *first_element)
{
    zval *zval_to_dump;
    HashPosition pos;

    zend_string *key;
    zend_long index;

    zend_hash_internal_pointer_reset_ex(p_symbol_table, &pos);

    while ((zval_to_dump = zend_hash_get_current_data_ex(p_symbol_table, &pos)) != NULL) {

        zend_hash_get_current_key_ex(p_symbol_table, &key, &index, &pos);

        meminfo_zval_dump(stream, frame_label, key, zval_to_dump, visited_items, first_element);

        zend_hash_move_forward_ex(p_symbol_table, &pos);
    }
}

int meminfo_visit_item(char * item_identifier, HashTable *visited_items)
{
    int found = 0;
    zval isset;
    zend_string * zstr_item_identifier;

    zstr_item_identifier = zend_string_init(item_identifier, strlen(item_identifier), 0);

    ZVAL_LONG(&isset, 1);

    if (zend_hash_exists(visited_items, zstr_item_identifier)) {
        found = 1;
    } else {
        zend_hash_add(visited_items, zstr_item_identifier, &isset);
    }
    zend_string_release(zstr_item_identifier);

    return found;
}

void meminfo_hash_dump(php_stream *stream, HashTable *ht, zend_bool is_object, HashTable *visited_items, int *first_element)
{
    zval *zval;

    zend_string *key;
    HashPosition pos;
    zend_ulong num_key;

    int first_child = 1;

    php_stream_printf(stream, "        \"children\" : {\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while ((zval = zend_hash_get_current_data_ex(ht, &pos)) != NULL) {
        char zval_id[17];

        if (Z_TYPE_P(zval) == IS_INDIRECT) {
            zval = Z_INDIRECT_P(zval);
        }

        if (Z_ISREF_P(zval)) {
            ZVAL_DEREF(zval);
        }

        if (Z_TYPE_P(zval) == IS_OBJECT) {
            sprintf(zval_id, "%p", Z_OBJ_P(zval));
        } else {
            sprintf(zval_id, "%p", zval);
        }

        if (!first_child) {
            php_stream_printf(stream, ",\n");
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

                    php_stream_printf(stream, "            \"%s\":\"%s\"", ZSTR_VAL(escaped_property_name), zval_id);

                    zend_string_release(escaped_property_name);
                } else {
                    zend_string * escaped_key;

                    escaped_key = meminfo_escape_for_json(ZSTR_VAL(key));

                    php_stream_printf(stream, "            \"%s\":\"%s\"", ZSTR_VAL(escaped_key), zval_id);

                    zend_string_release(escaped_key);
                }

                break;
            case HASH_KEY_IS_LONG:
                php_stream_printf(stream, "            \"%ld\":\"%s\"", num_key, zval_id);
                break;
        }

        zend_hash_move_forward_ex(ht, &pos);
    }
    php_stream_printf(stream, "\n        }\n");

    zend_hash_internal_pointer_reset_ex(ht, &pos);
    while ((zval = zend_hash_get_current_data_ex(ht, &pos)) != NULL) {
        meminfo_zval_dump(stream, NULL, NULL, zval, visited_items, first_element);
        zend_hash_move_forward_ex(ht, &pos);
    }
}

void meminfo_zval_dump(php_stream * stream, char * frame_label, zend_string * symbol_name, zval * zv, HashTable *visited_items, int *first_element)
{
    char zval_identifier[17];

    if (Z_TYPE_P(zv) == IS_INDIRECT) {
        zv = Z_INDIRECT_P(zv);
    }

    if (Z_ISREF_P(zv)) {
        ZVAL_DEREF(zv);
    }

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        sprintf(zval_identifier, "%p", Z_OBJ_P(zv));
    } else {
        sprintf(zval_identifier, "%p", zv);
    }

    if (meminfo_visit_item(zval_identifier, visited_items)) {
        return;
    }

    if (! *first_element) {
        php_stream_printf(stream, "\n    },\n");
    } else {
        *first_element = 0;
    }

    php_stream_printf(stream, "    \"%s\" : {\n", zval_identifier);
    php_stream_printf(stream, "        \"type\" : \"%s\",\n", zend_get_type_by_const(Z_TYPE_P(zv)));
    php_stream_printf(stream, "        \"size\" : \"%ld\",\n", meminfo_get_element_size(zv));

    if (frame_label) {
        zend_string * escaped_frame_label;

        if (symbol_name) {
            zend_string * escaped_symbol_name;

            escaped_symbol_name = meminfo_escape_for_json(ZSTR_VAL(symbol_name));

            php_stream_printf(stream, "        \"symbol_name\" : \"%s\",\n", ZSTR_VAL(escaped_symbol_name));

            zend_string_release(escaped_symbol_name);
        }

        escaped_frame_label = meminfo_escape_for_json(frame_label);

        php_stream_printf(stream, "        \"is_root\" : true,\n");
        php_stream_printf(stream, "        \"frame\" : \"%s\"\n", ZSTR_VAL(escaped_frame_label));

        zend_string_release(escaped_frame_label);
    } else {
        php_stream_printf(stream, "        \"is_root\" : false\n");
    }

    if (Z_TYPE_P(zv) == IS_OBJECT) {
        HashTable *properties;
        zend_string * escaped_class_name;

        properties = NULL;

        escaped_class_name = meminfo_escape_for_json(ZSTR_VAL(Z_OBJCE_P(zv)->name));

        php_stream_printf(stream, ",\n");
        php_stream_printf(stream, "        \"class\" : \"%s\",\n", ZSTR_VAL(escaped_class_name));

        zend_string_release(escaped_class_name);

        php_stream_printf(stream, "        \"object_handle\" : \"%d\",\n", Z_OBJ_HANDLE_P(zv));

#if PHP_VERSION_ID >= 70400
        properties = zend_get_properties_for(zv, ZEND_PROP_PURPOSE_DEBUG);
#else
        int is_temp;
        properties = Z_OBJDEBUG_P(zv, is_temp);
#endif

        if (properties != NULL) {
            meminfo_hash_dump(stream, properties, 1, visited_items, first_element);

#if PHP_VERSION_ID >= 70400
            zend_release_properties(properties);
#else
            if (is_temp) {
                zend_hash_destroy(properties);
                efree(properties);
            }
#endif
        }
    } else if (Z_TYPE_P(zv) == IS_ARRAY) {
        php_stream_printf(stream, ",\n");
        meminfo_hash_dump(stream, Z_ARRVAL_P(zv), 0, visited_items, first_element);
    } else {
        php_stream_printf(stream, "\n");
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
            size += Z_STRLEN_P(zv);
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

#if PHP_VERSION_ID >= 80000
        if (func->common.function_name) {
            function_name = ZSTR_VAL(func->common.function_name);
        } else {
            function_name = NULL;
        }
#else
        function_name = (func->common.scope &&
                         func->common.scope->trait_aliases) ?
            ZSTR_VAL(zend_resolve_method_name(
                (object ? object->ce : func->common.scope), func)) :
            (func->common.function_name ?
                ZSTR_VAL(func->common.function_name) : NULL);
#endif
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
    int i;
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

#ifdef COMPILE_DL_MEMINFO
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(meminfo)
#endif
