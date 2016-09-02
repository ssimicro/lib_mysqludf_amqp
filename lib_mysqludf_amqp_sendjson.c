#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "lib_mysqludf_amqp.h"
#include "send.h"

my_bool
lib_mysqludf_amqp_sendjson_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    return lib_mysqludf_amqp_send_init(initid, args, message);
}

char*
lib_mysqludf_amqp_sendjson(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error)
{
    return lib_mysqludf_amqp_send(initid, args, result, length, is_null, error, "application/json");
}

void
lib_mysqludf_amqp_sendjson_deinit(UDF_INIT *initid)
{
    lib_mysqludf_amqp_send_deinit(initid);
}
