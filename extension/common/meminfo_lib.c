/**
 * Meminfo common lib
 *
 * @author    Benoit Jacquemont <benoit@akeneo.com>
 * @license   MIT
 */

#include "php.h"
#include "meminfo_lib.h"

/**
 * Escape the \ and " characters for JSON encoding
 */
char * meminfo_escape_for_json(const char *s)
{
    int new_str_len;
    char *s1, *s2;
    s1 = php_str_to_str((char*)s, strlen(s), "\\", 1, "\\\\", 2, &new_str_len);
    s2 = php_str_to_str(s1, strlen(s1), "\"", 1, "\\\"", 2, &new_str_len);

    efree(s1);

    return s2;
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
