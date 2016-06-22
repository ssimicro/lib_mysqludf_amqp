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
    /* TODO should the various functions be split into separate files (I think so) */

    my_bool lib_mysqludf_amqp_info_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_info(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    void lib_mysqludf_amqp_info_deinit(UDF_INIT *initid);

    my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
    char *lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
    void lib_mysqludf_amqp_sendstring_deinit(UDF_INIT *initid);

#ifdef    __cplusplus
}
#endif

/*
 * lib_mysqludf_amqp_info
 */

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


/*
 * lib_mysqludf_amqp_sendstring
 */

typedef struct knapsack {   /* TODO come up with a better name for this */
    amqp_socket_t *socket;
    amqp_connection_state_t conn;
} knapsack_t;

my_bool lib_mysqludf_amqp_sendstring_init(UDF_INIT *initid, UDF_ARGS *args, char *message) {

    int rc;
    amqp_rpc_reply_t reply;
    knapsack_t *knapsack;

    /* TODO determine max argument length (is it 255?) and document it */
    if (args->arg_count != 5 || (args->arg_type[0] != STRING_RESULT) /* host */
                             || (args->arg_type[1] != INT_RESULT)    /* port */
                             || (args->arg_type[2] != STRING_RESULT) /* exchange */
                             || (args->arg_type[3] != STRING_RESULT) /* routing key */
                             || (args->arg_type[4] != STRING_RESULT)) { /* message */
        strncpy(message, "lib_mysqludf_amqp_sendstring: invalid arguments", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    knapsack = (knapsack_t *) malloc(sizeof(knapsack_t));
    knapsack->conn = amqp_new_connection();
    knapsack->socket = amqp_tcp_socket_new(knapsack->conn);
    if (knapsack->socket == NULL) {                             /* TODO need a clean-up function to avoid these long IF blocks */
        amqp_destroy_connection(knapsack->conn);
        free(initid->ptr);
        initid->ptr = NULL;
        strncpy(message, "lib_mysqludf_amqp_sendstring: socket error", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    /* TODO verify that the cast is correct (it works, but is it the right thing to do here) */
    rc = amqp_socket_open(knapsack->socket, args->args[0], (int)(*(long long *)args->args[1]) );
    if (rc < 0) {
        amqp_destroy_connection(knapsack->conn);
        free(initid->ptr);
        initid->ptr = NULL;
        strncpy(message, "lib_mysqludf_amqp_sendstring: socket open error", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    /* TODO make login credentials configurable - do we have access to session variables? */
    reply = amqp_login(knapsack->conn, "/", 0, 131072, 0, AMQP_SASL_METHOD_PLAIN, "guest", "guest");
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        amqp_connection_close(knapsack->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(knapsack->conn);
        free(initid->ptr);
        initid->ptr = NULL;
        strncpy(message, "lib_mysqludf_amqp_sendstring: login error", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    amqp_channel_open(knapsack->conn, 1);
    reply = amqp_get_rpc_reply(knapsack->conn);
    if (reply.reply_type != AMQP_RESPONSE_NORMAL) {
        amqp_connection_close(knapsack->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(knapsack->conn);
        free(initid->ptr);
        initid->ptr = NULL;
        strncpy(message, "lib_mysqludf_amqp_sendstring: channel error", MYSQL_ERRMSG_SIZE);
        return 1;
    }

    initid->ptr = (char *) knapsack;
    initid->maybe_null = 1; // returns NULL
    initid->const_item = 0; // may return something different if called again

    return 0;
}

char* lib_mysqludf_amqp_sendstring(UDF_INIT *initid, UDF_ARGS *args, char* result, unsigned long* length, char *is_null, char *error) {

    int rc;
    knapsack_t *knapsack = (knapsack_t *) initid->ptr;

    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG | AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain"); /* TODO would be nice to support JSON */
    props.delivery_mode = 2;

    rc = amqp_basic_publish(knapsack->conn, 1, amqp_cstring_bytes(args->args[2]), amqp_cstring_bytes(args->args[3]), 0, 0, &props, amqp_cstring_bytes(args->args[4]));
    if (rc < 0) {
        amqp_channel_close(knapsack->conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(knapsack->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(knapsack->conn);
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
        knapsack_t *knapsack = (knapsack_t *) initid->ptr;
        amqp_channel_close(knapsack->conn, 1, AMQP_REPLY_SUCCESS);
        amqp_connection_close(knapsack->conn, AMQP_REPLY_SUCCESS);
        amqp_destroy_connection(knapsack->conn);
        free(initid->ptr);
        initid->ptr = NULL;
    }

    return;
}
