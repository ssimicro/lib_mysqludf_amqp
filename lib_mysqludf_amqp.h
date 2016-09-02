#ifndef __LIB_MYSQLUDF_AMQP_H
#define __LIB_MYSQLUDF_AMQP_H

/* Public User Defined Functions for MySQL */

#ifdef    __cplusplus
extern "C" {
#endif

    extern my_bool lib_mysqludf_amqp_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    extern char *lib_mysqludf_amqp_info(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    extern void lib_mysqludf_amqp_info_deinit(UDF_INIT *initid);

    extern my_bool lib_mysqludf_amqp_sendjson_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    extern char *lib_mysqludf_amqp_sendjson(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    extern void lib_mysqludf_amqp_sendjson_deinit(UDF_INIT *initid);

    extern my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    extern char *lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    extern void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid);

#ifdef    __cplusplus
}
#endif

#endif /* __LIB_MYSQLUDF_AMQP_H */
