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

long meminfo_get_var_size(zval *var);

long meminfo_get_hash_size(HashTable *ht)
{
    zval **var;
    char *string_key;
    HashPosition iterator;
    ulong num_key;
    uint str_len;

    long hash_size = 0;

    zend_hash_internal_pointer_reset_ex(ht, &iterator);

    while (zend_hash_get_current_data_ex(ht, (void **) &var, &iterator) == SUCCESS) {
        switch (zend_hash_get_current_key_ex(ht, &string_key, &str_len, &num_key, 0, &iterator)) {
            case HASH_KEY_IS_STRING:
                zend_printf("Hash key string");
                break;
            case HASH_KEY_IS_LONG:
                {
                    char key[25];
                    snprintf(key, sizeof(key), "%ld", num_key);
                }
                break;
        }

        hash_size += meminfo_get_var_size(*var);

        zend_hash_move_forward_ex(ht, &iterator);
    }
    return hash_size;
    
}

long meminfo_get_var_size(zval *var)
{
    long var_size = 0;

    switch (var->type) {
        case IS_ARRAY:
/*
            if (++var->value.ht->nApplyCount>1) {
                var->value.ht->nApplyCount--;
                return var_size;
            }
            var_size += meminfo_get_hash_size(var->value.ht);
            var->value.ht->nApplyCount--;
*/
            break;
        case IS_OBJECT:
            // Object's content not managed here : the object will be part 
            // of the objects buckets list anyway, so avoiding duplicate
            break;
        case IS_STRING:
            var_size = var->value.str.len;
            break;
        default:
            var_size = sizeof(zval);
            break;
    }

    return var_size;
  
//  zend_printf("\nZval:[");
  //zend_print_flat_zval_r(*tmp TSRMLS_CC);
//  zend_print_zval_r(var, 0 TSRMLS_CC);
//  zend_printf("]\n");
  // Go through recursively on each part of the property
  // If the value is not an object:
    // get the size of the value
    // Add values to the total
 
  // TODO : add the size of the zval metadata (for example, pointers used by
  // the structure and data like string length

  // TODO : same thing for the object. In both case, check how the memory is really
  // calculated by zend

  // TODO : check if the zval has already be counted (case where a variable is
  // shared by reference between several objects
}

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
        object_size = 0;
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


