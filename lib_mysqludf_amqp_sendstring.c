#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "send.h"

#ifdef    __cplusplus
extern "C" {
#endif

    my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid);

#ifdef    __cplusplus
}
#endif

my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {
    return lib_mysqludf_amqp_send_init(initid, args, message);
}

char* lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error) {
    return lib_mysqludf_amqp_send(initid, args, result, length, is_null, error, "text/plain");
}

void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid) {
    lib_mysqludf_amqp_send_deinit(initid);
}
