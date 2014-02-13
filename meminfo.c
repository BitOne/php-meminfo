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
    PHP_FE(meminfo_gc_roots_list, NULL)
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

    stream_printf(stream, "Structs size on this platform:\n");
    stream_printf(stream, "  Class (zend_class_entry): %ld bytes.\n", sizeof(zend_class_entry));
    stream_printf(stream, "  Object (zend_object): %ld bytes.\n", sizeof(zend_object));
    stream_printf(stream, "  Variable (zval): %ld bytes.\n", sizeof(zval));
    stream_printf(stream, "  Variable value (zvalue_value): %ld bytes.\n", sizeof(zvalue_value));
}

PHP_FUNCTION(meminfo_objects_list)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);

    stream_printf(stream, "Objects list:\n");

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
            object =  (zend_object * ) obj->object;

            class_entry = object->ce;
            stream_printf(stream, "  - Class %s, handle %d\n", class_entry->name, i);

            current_objects++;
        }
     }

    stream_printf(stream, "Total object buckets: %d. Current objects: %d.\n", total_objects_buckets, current_objects);

}

PHP_FUNCTION(meminfo_gc_roots_list)
{
    zval *zval_stream;
    php_stream *stream;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &zval_stream) == FAILURE) {
        return;
    }

    php_stream_from_zval(stream, &zval_stream);
    stream_printf(stream, "GC roots list:\n");

    gc_root_buffer *current = GC_G(roots).next;
    while (current != &GC_G(roots)) {
        if (current->handle) {
            stream_printf(stream, "  Handle %d found on GC root\n", current->handle);
        } else {
            zval* z =current->u.pz;

            stream_printf(stream, "  Not object GC root\n");
            stream_printf(stream, "  GC root of type %c\n",Z_TYPE_P(current->u.pz));
        }
        stream_printf(stream, "    Ref count GC %d\n",current->u.pz->refcount__gc);
        current = current->next;

    }
}


