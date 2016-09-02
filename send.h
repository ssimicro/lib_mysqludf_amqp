#ifndef __LIB_MYSQLUDF_AMQP_SEND_H
#define __LIB_MYSQLUDF_AMQP_SEND_H

#include <mysql.h>

#ifdef    __cplusplus
extern "C" {
#endif

    my_bool lib_mysqludf_amqp_send_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_send(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error, char *content_type);
    void lib_mysqludf_amqp_send_deinit(UDF_INIT *initid);

#ifdef    __cplusplus
}
#endif

#endif
