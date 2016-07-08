#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#ifdef    __cplusplus
extern "C" {
#endif

    my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid);

#ifdef    __cplusplus
}
#endif

typedef struct conn_info {
    amqp_socket_t *socket;
    amqp_connection_state_t conn;
} conn_info_t;

my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {

    int rc;
    amqp_rpc_reply_t reply;
    conn_info_t *conn_info;

    if (args->arg_count != 7 || (args->arg_type[0] != STRING_RESULT)        /* host */
                             || (args->arg_type[1] != INT_RESULT)           /* port */
                             || (args->arg_type[2] != STRING_RESULT)        /* username */
                             || (args->arg_type[3] != STRING_RESULT)        /* password */
                             || (args->arg_type[4] != STRING_RESULT)        /* exchange */
                             || (args->arg_type[5] != STRING_RESULT)        /* routing key */
                             || (args->arg_type[6] != STRING_RESULT)) {     /* message */
        strncpy(message, "lib_mysqludf_amqp_sendstring: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    conn_info = (conn_info_t *) malloc(sizeof(conn_info_t));
    conn_info->conn = amqp_new_connection();
    conn_info->socket = amqp_tcp_socket_new(conn_info->conn);
    if (conn_info->socket == NULL) {
        strncpy(message, "lib_mysqludf_amqp_sendstring: socket error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;;
    }

    rc = amqp_socket_open(conn_info->socket, args->args[0], (int)(*((long long *) args->args[1])));
    if (rc < 0) {
        strncpy(message, "lib_mysqludf_amqp_sendstring: socket open error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;
    }

    reply = amqp_login(conn_info->conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, args->args[2], args->args[3]);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        strncpy(message, "lib_mysqludf_amqp_sendstring: login error", MYSQL_ERRMSG_SIZE);
        goto init_error_close;
    }

    amqp_channel_open(conn_info->conn, 1);
    reply = amqp_get_rpc_reply(conn_info->conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        strncpy(message, "lib_mysqludf_amqp_sendstring: channel error", MYSQL_ERRMSG_SIZE);
        goto init_error_close;
    }

    initid->ptr = (char *) conn_info;
    initid->maybe_null = 1; // returns NULL
    initid->const_item = 0; // may return something different if called again

    return 0;

init_error_close:
    amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);

init_error_destroy:
    amqp_destroy_connection(conn_info->conn);
    free(initid->ptr);
    initid->ptr = NULL;

    return 1;
}

char* lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error) {

    int rc;
    conn_info_t *conn_info = (conn_info_t *) initid->ptr;

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain"); /* TODO would be nice to support JSON */
    props.delivery_mode = 2;

    rc = amqp_basic_publish(conn_info->conn, 1, amqp_cstring_bytes(args->args[4]), amqp_cstring_bytes(args->args[5]), 0, 0, &props, amqp_cstring_bytes(args->args[6]));
    if (rc < 0) {
        amqp_channel_close(conn_info->conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn_info->conn);
        free(initid->ptr);
        initid->ptr = NULL;
        *is_null = 1;
        *error = 1;
        return NULL;
    }

    *is_null = 1;
    *error = 0;

    /* TODO should this return something besides NULL? SUCCESS/FAIL? Some type of correlationId? */

    return NULL;
}

void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid) {
    if (initid->ptr != NULL) {
        conn_info_t *conn_info = (conn_info_t *) initid->ptr;
        amqp_channel_close(conn_info->conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(conn_info->conn);
        free(initid->ptr);
        initid->ptr = NULL;
    }

    return;
}
