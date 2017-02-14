#include "config.h"

#include <ctype.h>
#include <mysql.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "send.h"
#include "uuid.h"

typedef struct conn_info {
    amqp_socket_t *socket;
    amqp_connection_state_t conn;
} conn_info_t;

my_bool
lib_mysqludf_amqp_send_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{

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
        (void) strncpy(message, "lib_mysqludf_amqp_send: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    conn_info = (conn_info_t *) malloc(sizeof(conn_info_t));
    conn_info->conn = amqp_new_connection();
    conn_info->socket = amqp_tcp_socket_new(conn_info->conn);
    if (conn_info->socket == NULL) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: socket error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;;
    }

    rc = amqp_socket_open(conn_info->socket, args->args[0], (int)(*((long long *) args->args[1])));
    if (rc < 0) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: socket open error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;
    }

    reply = amqp_login(conn_info->conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, args->args[2], args->args[3]);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: login error", MYSQL_ERRMSG_SIZE);
        goto init_error_close;
    }

    amqp_channel_open(conn_info->conn, 1);
    reply = amqp_get_rpc_reply(conn_info->conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: channel error", MYSQL_ERRMSG_SIZE);
        goto init_error_close;
    }

    initid->ptr = (char *) conn_info;
    initid->maybe_null = 1; /* returns NULL */
    initid->const_item = 0; /* may return something different if called again */

    return 0;

init_error_close:
    (void) amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);

init_error_destroy:
    (void) amqp_destroy_connection(conn_info->conn);
    free(initid->ptr);
    initid->ptr = NULL;

    return 1;
}

char*
lib_mysqludf_amqp_send(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error, char *content_type)
{

    int rc;
    conn_info_t *conn_info = (conn_info_t *) initid->ptr;
    amqp_table_entry_t headers[1];
    amqp_basic_properties_t props;
    props._flags = 0;

    /* contentType */
    props.content_type = amqp_cstring_bytes(content_type);
    props._flags |= AMQP_BASIC_CONTENT_TYPE_FLAG;

    /* deliveryMode */
    props.delivery_mode = 2;
    props._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;

    /* timestamp */
    props.timestamp = time(NULL);
    props._flags |= AMQP_BASIC_TIMESTAMP_FLAG;

    /* headers */
    headers[0].key = amqp_cstring_bytes("User-Agent");
    headers[0].value.kind = AMQP_FIELD_KIND_UTF8;
    headers[0].value.value.bytes = amqp_cstring_bytes(PACKAGE_NAME "/" PACKAGE_VERSION);
    props.headers.entries = headers;
    props.headers.num_entries = sizeof(headers) / sizeof(headers[0]);
    props._flags |= AMQP_BASIC_HEADERS_FLAG;

    /* appId */
    props.app_id = amqp_cstring_bytes(PACKAGE_NAME);
    props._flags |= AMQP_BASIC_APP_ID_FLAG;

    /* messageId */
    ssiuuidgen(result);
    props.message_id = amqp_cstring_bytes(result);
    props._flags |= AMQP_BASIC_MESSAGE_ID_FLAG;

    rc = amqp_basic_publish(conn_info->conn, 1, amqp_cstring_bytes(args->args[4]), amqp_cstring_bytes(args->args[5]), 0, 0, &props, amqp_cstring_bytes(args->args[6]));
    if (rc < 0) {
        (void) amqp_channel_close(conn_info->conn, 1, AMQP_REPLY_SUCCESS);
        (void) amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);
        (void) amqp_destroy_connection(conn_info->conn);
        free(initid->ptr);
        initid->ptr = NULL;

        *is_null = 1;
        *error = 1;

        return NULL;
    }

    *is_null = 0;
    *error = 0;
    *length = (unsigned long) strlen(result);

    return result;
}

void
lib_mysqludf_amqp_send_deinit(UDF_INIT *initid)
{
    if (initid->ptr != NULL) {
        conn_info_t *conn_info = (conn_info_t *) initid->ptr;
        (void) amqp_channel_close(conn_info->conn, 1, AMQP_REPLY_SUCCESS);
        (void) amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);
        (void) amqp_destroy_connection(conn_info->conn);
        free(initid->ptr);
        initid->ptr = NULL;
    }

    return;
}
