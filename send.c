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
    char *url;
    struct amqp_connection_info parsed_url;
    size_t url_buf_len;
    amqp_rpc_reply_t reply;
    conn_info_t *conn_info;

    if (args->arg_count != 4 || (args->arg_type[0] != STRING_RESULT)        /* url */
                             || (args->arg_type[1] != STRING_RESULT)        /* exchange */
                             || (args->arg_type[2] != STRING_RESULT)        /* routing key */
                             || (args->arg_type[3] != STRING_RESULT)) {     /* message */
        (void) strncpy(message, "lib_mysqludf_amqp_send: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    url_buf_len = args->lengths[0] + 1;
    url = (char *) malloc(url_buf_len);
    if (url == NULL) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: malloc error", MYSQL_ERRMSG_SIZE);
        goto init_error_url;
    }
    memset(url, '\0', url_buf_len);
    memcpy(url, args->args[0], args->lengths[0]);

    amqp_default_connection_info(&parsed_url);
    rc = amqp_parse_url(url, &parsed_url);
    if (rc != AMQP_STATUS_OK) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: bad URL", MYSQL_ERRMSG_SIZE);
        goto init_error_url;
    }

    conn_info = (conn_info_t *) malloc(sizeof(conn_info_t));
    conn_info->conn = amqp_new_connection();
    conn_info->socket = amqp_tcp_socket_new(conn_info->conn);
    if (conn_info->socket == NULL) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: socket error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;
    }

    rc = amqp_socket_open(conn_info->socket, parsed_url.host, parsed_url.port);
    if (rc < 0) {
        (void) strncpy(message, "lib_mysqludf_amqp_send: socket open error", MYSQL_ERRMSG_SIZE);
        goto init_error_destroy;
    }

    reply = amqp_login(conn_info->conn, parsed_url.vhost, 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, parsed_url.user, parsed_url.password);
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

    free(url);

    return 0;

init_error_close:
    (void) amqp_connection_close(conn_info->conn, AMQP_REPLY_SUCCESS);

init_error_destroy:
    (void) amqp_destroy_connection(conn_info->conn);
    free(initid->ptr);
    initid->ptr = NULL;

init_error_url:
    free(url);

    return 1;
}

char*
lib_mysqludf_amqp_send(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error, char *content_type)
{

    int rc;
    amqp_bytes_t exchange = {
        .bytes = args->args[1],
        .len = args->lengths[1]
    };
    amqp_bytes_t routing_key = {
        .bytes = args->args[2],
        .len = args->lengths[2],
    };
    amqp_bytes_t payload = {
        .bytes = args->args[3],
        .len = args->lengths[3],
    };
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

    rc = amqp_basic_publish(conn_info->conn, 1, exchange, routing_key, 0, 0, &props, payload);
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
