#include <io.h>

int main(void)
{
    io_Err err = IO_ERR_OK;
    io_Context context = io_Context_make();
    io_UnixAcceptor acceptor = io_UnixAcceptor_make(&context);
    if (!io_ok(err = io_UnixAcceptor_bind(&acceptor, "/tmp/test"))) {
        goto on_error;
    }
    io_UnixSocket socket = io_UnixSocket_make(&context);
    if (!io_ok(err = io_UnixAcceptor_accept(&acceptor, &socket))) {
        goto on_error;
    }
    char buffer[512];
    size_t size = sizeof(buffer);
    if (!io_ok(err = io_UnixSocket_read(&socket, buffer, &size))) {
        goto on_error;
    }
    buffer[size] = '\0';
    printf("Msg: %s\n", buffer);
    goto cleanup;
on_error:
    io_Err_printf(stderr, err);
cleanup:
    return -1;
}
