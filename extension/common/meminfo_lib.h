/**
 * Meminfo common lib header
 *
 * @author    Benoit Jacquemont <benoit@akeneo.com>
 * @license   MIT
 */

#include "php.h"

char * meminfo_escape_for_json(const char *s);
char * meminfo_info_dump_header(char *header, int header_len);
