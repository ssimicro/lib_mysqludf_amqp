#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef    __cplusplus
extern "C" {
#endif
    my_bool lib_mysqludf_amqp_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_info(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    void lib_mysqludf_amqp_info_deinit(UDF_INIT *initid);
#ifdef    __cplusplus
}
#endif

my_bool lib_mysqludf_amqp_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {

    if (args->arg_count != 0) {
        strncpy(message, "lib_mysqludf_amqp_info: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    initid->max_length = strlen(PACKAGE_STRING) + 1;
    initid->maybe_null = 0;
    initid->const_item = 1;

    return 0;
}

char* lib_mysqludf_amqp_info(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error) {

    strncpy(result, PACKAGE_STRING, *length);

    *length = strlen(result);
    *is_null = 0;
    *error = 0;

    return result;
}

void lib_mysqludf_amqp_info_deinit(UDF_INIT *initid) {
    return;
}
