#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_meminfo.h"

#include "ext/standard/info.h"

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

//TODO: use gettype directly ? Possible ?
char* get_type_label(zval* z) {
    switch (Z_TYPE_P(z)) {
        case IS_NULL:
            return "NULL";
            break;

        case IS_BOOL:
            return "boolean";
            break;

        case IS_LONG:
            return "integer";
            break;

        case IS_DOUBLE:
            return "double";
            break;

        case IS_STRING:
            return "string";
            break;

        case IS_ARRAY:
            return "array";
            break;

        case IS_OBJECT:
            return "object";
            break;

        case IS_RESOURCE:
            return "resource";
            break;

        default:
            return "Unknown type";
    }
}

/**
 * Return the class associated to the provided object handle
 *
 * @param zend_uint object handle
 *
 * @return char * class name
 */
const char *get_classname(zend_uint handle TSRMLS_DC)
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


PHP_FUNCTION(meminfo_structs_size)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);

    php_stream_printf(stream TSRMLS_CC, "Structs size on this platform:\n");
    php_stream_printf(stream TSRMLS_CC, "  Class (zend_class_entry): %ld bytes.\n", sizeof(zend_class_entry));
    php_stream_printf(stream TSRMLS_CC, "  Object (zend_object): %ld bytes.\n", sizeof(zend_object));
    php_stream_printf(stream TSRMLS_CC, "  Variable (zval): %ld bytes.\n", sizeof(zval));
    php_stream_printf(stream TSRMLS_CC, "  Variable value (zvalue_value): %ld bytes.\n", sizeof(zvalue_value));
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

    for (i = 1; i < objects->top ; i++) {
        if (objects->object_buckets[i].valid) {
            struct _store_object *obj = &objects->object_buckets[i].bucket.obj;

            php_stream_printf(stream TSRMLS_CC, "  - Class %s, handle %d, refCount %d\n", get_classname(i TSRMLS_CC), i, obj->refcount);

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

    ALLOC_HASHTABLE(classes);

    zend_hash_init(classes, 1000, NULL, NULL, 0);

    zend_objects_store *objects = &EG(objects_store);
    zend_uint i;

    for (i = 1; i < objects->top ; i++) {
        if (objects->object_buckets[i].valid && !objects->object_buckets[i].destructor_called) {
            const char *class_name;
            zval **zv_dest;

            class_name = get_classname(i TSRMLS_CC);

            if (zend_hash_find(classes, class_name, strlen(class_name)+1, (void **) &zv_dest) == SUCCESS) {
                Z_LVAL_PP(zv_dest) = Z_LVAL_PP(zv_dest) ++;
            } else {
                zval *zv_instances_count;
                MAKE_STD_ZVAL(zv_instances_count);
                ZVAL_LONG(zv_instances_count, 1);

                zend_hash_update(classes, class_name, strlen(class_name)+1, &zv_instances_count, sizeof(zval *), NULL);
            }
        }
    }

    zend_hash_sort(classes, zend_qsort, instances_count_compare, 0 TSRMLS_CC);

    php_stream_from_zval(stream, &zval_stream);
    php_stream_printf(stream TSRMLS_CC, "Instances count by class:\n");

    php_stream_printf(stream TSRMLS_CC, "%-12s %-12s %s\n", "num", "#instances", "class");
    php_stream_printf(stream TSRMLS_CC, "-----------------------------------------------------------------\n");

    zend_uint num = 1;

    HashPosition position;
    zval **entry = NULL;

    for (zend_hash_internal_pointer_reset_ex(classes, &position);
         zend_hash_get_current_data_ex(classes, (void **) &entry, &position) == SUCCESS;
         zend_hash_move_forward_ex(classes, &position)) {

        char *class_name = NULL;
        uint  class_name_len;
        ulong index;

        zend_hash_get_current_key_ex(classes, &class_name, &class_name_len, &index, 0, &position);
        php_stream_printf(stream TSRMLS_CC, "%-12d %-12ld %s\n", num, Z_LVAL_PP(entry), class_name);

        num++;
    }

    zend_hash_destroy(classes);
    FREE_HASHTABLE(classes);
}

static int instances_count_compare(const void *a, const void *b TSRMLS_DC)
{
    const Bucket *first_bucket;
    const Bucket *second_bucket;

    first_bucket = *((const Bucket **) a);
    second_bucket = *((const Bucket **) b);

    zval *zv_first;
    zval *zv_second;

    zv_first = (zval *) first_bucket->pDataPtr;
    zv_second = (zval *) second_bucket->pDataPtr;


    zend_uint first;
    zend_uint second;

    first = Z_LVAL_P(zv_first);
    second = Z_LVAL_P(zv_second);

    if (first > second) {
        return -1;
    } else if (first == second) {
        return 0;
    } else {
        return 1;
    }
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
                get_classname(current->handle TSRMLS_CC),
                current->handle);
        } else {
            php_stream_printf(stream TSRMLS_CC, "  Type: %s", get_type_label(pz));
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
