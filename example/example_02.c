/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#include <io.h>

typedef struct Connection {
    io_UnixSocket socket;
} Connection;

static Connection*
Connection_create(io_Context* context)
{
    Connection* connection = malloc(sizeof(Connection));
    io_UnixSocket_init(&connection->socket, context);
    return connection;
}

static void
Connection_destroy(Connection* connection)
{
    io_UnixSocket_deinit(&connection->socket);
    free(connection);
}

static void
Connection_handle_write(void* self, size_t size, io_Err err)
{
    if (!io_ok(err)) {
        printf("Failed to read bytes");
    } else {
        printf("Wrote %d of bytes\n", (int)size);
    }
    Connection_destroy(self);
}

#define MSG "Hello Example!"

static void
Connection_start(Connection* connection)
{
    io_Err err = IO_ERR_OK;
    if (!io_ok(err = io_UnixSocket_async_write(&connection->socket, MSG, sizeof(MSG),
                                               Connection_handle_write, connection))) {
        printf("Failed to initiate write: %s, abort\n", io_Err_msg(err));
        Connection_destroy(connection);
    }
}

typedef struct Server {
    io_Context* context;
    io_UnixAcceptor acceptor;
    Connection* accepting;
} Server;

static void
Server_deinit(Server* server)
{
    io_UnixAcceptor_deinit(&server->acceptor);
    Connection_destroy(server->accepting);
}

static void
Server_accept(Server* server);

static void
Server_handle_accept(void* self, io_Err err)
{
    Server* server = self;
    if (!io_ok(err)) {
        printf("Failed to accept connection: %s\n", io_Err_msg(err));
        return;
    }
    printf("New Connection\n");
    Connection_start(IO_MOVE_PTR(server->accepting));
    Server_accept(server);
}

static void
Server_accept(Server* server)
{
    server->accepting = Connection_create(server->context);
    io_Err err = IO_ERR_OK;
    if (!io_ok(err = io_UnixAcceptor_async_accept(&server->acceptor, &server->accepting->socket,
                                                  Server_handle_accept, server))) {
        printf("Failed to initiate accept: %s, abort\n", io_Err_msg(err));
        Connection_destroy(server->accepting);
    }
}

static io_Err
Server_init(Server* server, io_Context* ctx, const char* path)
{
    server->context = ctx;
    unlink(path);
    io_Err err = IO_ERR_OK;
    if (!io_ok(err = io_UnixAcceptor_init(&server->acceptor, server->context, path))) {
        return err;
    }
    Server_accept(server);
    return err;
}

int main(void)
{
    io_Err err = IO_ERR_OK;
    io_Context context;
    if (!io_ok(err = io_Context_init(&context))) {
        printf("Failed to init context: %s\n", io_Err_msg(err));
        return -1;
    }
    Server server;
    if (!io_ok(err = Server_init(&server, &context, "/tmp/test"))) {
        printf("Failed to init server: %s\n", io_Err_msg(err));
        goto cleanup_context;
    }
    io_Context_run(&context);
    Server_deinit(&server);
cleanup_context:
    io_Context_deinit(&context);
    return (io_ok(err)) ? 0 : -1;
}
