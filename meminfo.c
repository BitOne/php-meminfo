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
    zend_printf("==============================\n");
    zend_printf("Structs size on this platform:\n");
    zend_printf("------------------------------\n");
    zend_printf("  Class: %ld bytes.\n", sizeof( zend_class_entry));
    zend_printf("  Object: %ld bytes.\n", sizeof( zend_object));
    zend_printf("  Variable: %ld bytes.\n", sizeof( zval));
    zend_printf("  Variable value: %ld bytes.\n", sizeof( zvalue_value));
    zend_printf("===============================\n\n");
}

PHP_FUNCTION(meminfo_objects_list)
{
    zend_printf("===============================\n");
    zend_printf("Dumping objects list:\n");
    zend_printf("-------------------------------\n");

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
            zend_printf("  Object handle %d, of type %s.\n", i, class_entry->name);

            current_objects++;
        }
     }

    zend_printf("Objects buckets: %d. Current objects: %d.\n", total_objects_buckets, current_objects);
    zend_printf("===============================\n\n");
}


