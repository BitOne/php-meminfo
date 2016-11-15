/**
 * Meminfo common lib
 *
 * @author    Benoit Jacquemont <benoit@akeneo.com>
 * @license   MIT
 */

#include "php.h"
#include "meminfo_lib.h"

/**
 * Generate a JSON header for the info_dump output
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
