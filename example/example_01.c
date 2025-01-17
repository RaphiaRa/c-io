#include <io.h>

int main(void)
{
    io_Err err = IO_ERR_OK;
    io_Context context;
    if (!io_ok(err = io_Context_init(&context))) {
        printf("Failed to init context: %s\n", io_Err_msg(err));
        return -1;
    }
    io_UnixAcceptor acceptor;
    if (!io_ok(err = io_UnixAcceptor_init(&acceptor, &context, "/tmp/test"))) {
        printf("Failed to init acceptor: %s\n", io_Err_msg(err));
        goto cleanup_context;
    }
    io_UnixSocket accepting;
    if (!io_ok(err = io_UnixSocket_init(&accepting, &context))) {
        printf("Failed to init accepting: %s\n", io_Err_msg(err));
        goto cleanup_acceptor;
    }
    if (!io_ok(err = io_UnixAcceptor_accept(&acceptor, &accepting))) {
        printf("Failed to accept: %s\n", io_Err_msg(err));
        goto cleanup_socket;
    }
    printf("New Connection\n");
    char buffer[512];
    size_t size = sizeof(buffer);
    if (!io_ok(err = io_UnixSocket_read(&accepting, buffer, &size))) {
        printf("Failed to read: %s\n", io_Err_msg(err));
        goto cleanup_socket;
    }
    printf("Read: %s\n", buffer);
cleanup_socket:
    io_UnixSocket_deinit(&accepting);
cleanup_acceptor:
    io_UnixAcceptor_deinit(&acceptor);
cleanup_context:
    io_Context_deinit(&context);
}
