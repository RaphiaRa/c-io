#include <io.h>

typedef struct Connection {
    io_UnixSocket socket;
} Connection;

static Connection*
Connection_create(io_Context* context)
{
    Connection* connection = malloc(sizeof(Connection));
    connection->socket = io_UnixSocket_make(context);
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
    if (IO_ERR_HAS(err)) {
        printf("Failed to read bytes");
    } else {
        printf("Wrote %d of bytes\n", (int)size);
    }
    free(self);
}

#define MSG "Hello Example!"

static void
Connection_start(Connection* connection)
{
    io_UnixSocket_async_write(&connection->socket, MSG, sizeof(MSG), Connection_handle_write, connection);
}

typedef struct Server {
    io_Context* context;
    io_UnixAcceptor acceptor;
    Connection* accepting;
} Server;

static Server
Server_make(io_Context* context)
{
    Server server = {
        .context = context,
        .acceptor = {io_Acceptor_make(context)},
        .accepting = NULL,
    };
    return server;
}

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
    if (IO_ERR_HAS(err)) {
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
    io_UnixAcceptor_async_accept(&server->acceptor, &server->accepting->socket,
                                 Server_handle_accept, server);
}

static io_Err
Server_start(Server* server, const char* path)
{
    unlink(path);
    io_Err err = IO_ERR_OK;
    if (IO_ERR_HAS(err = io_UnixAcceptor_bind(&server->acceptor, path))) {
        return err;
    }
    Server_accept(server);
    return err;
}

int main(void)
{
    io_Err err = IO_ERR_OK;
    io_Context context = io_Context_make();
    Server server = Server_make(&context);
    if (IO_ERR_HAS(err = Server_start(&server, "/tmp/test"))) {
        goto on_error;
    }
    io_Context_run(&context);
    goto cleanup;
on_error:
    io_Err_printf(stderr, err);
cleanup:
    Server_deinit(&server);
    io_Context_deinit(&context);
}
