#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib_mysqludf_amqp.h"

my_bool
lib_mysqludf_amqp_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{

    if (args->arg_count != 0) {
        (void) strncpy(message, "lib_mysqludf_amqp_info: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    initid->max_length = (unsigned long) strlen(PACKAGE_STRING) + 1;
    return 0;
}

char*
lib_mysqludf_amqp_info(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error)
{

    (void) strncpy(result, PACKAGE_STRING, strlen(PACKAGE_STRING) + 1);
    *length = (unsigned long) strlen(result);

    *is_null = 0;

    return result;
}

void
lib_mysqludf_amqp_info_deinit(UDF_INIT *initid)
{
    /* nothing to do here */
    return;
}
