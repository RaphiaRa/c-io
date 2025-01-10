#ifndef IO_UNIX_ACCEPTOR_H
#define IO_UNIX_ACCEPTOR_H

#include <io/io_config.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <io/io_acceptor.h>
#include <io/io_context.h>
#include <io/io_err.h>
#include <io/io_system_err.h>
#include <io/io_unix_socket.h>

typedef struct io_UnixAcceptor {
    io_Acceptor base;
} io_UnixAcceptor;

IO_INLINE(io_UnixAcceptor)
io_UnixAcceptor_make(io_Context* ctx)
{
    io_UnixAcceptor acceptor = {
        .base = io_Acceptor_make(ctx),
    };
    return acceptor;
}

IO_INLINE(io_Err)
io_UnixAcceptor_bind(io_UnixAcceptor* acceptor, const char* path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return io_SystemErr_make(errno);
    }
    io_Err err = IO_ERR_OK;
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        err = io_SystemErr_make(errno);
        goto cleanup_socket;
    }
    if (listen(fd, SOMAXCONN) == -1) {
        err = io_SystemErr_make(errno);
        goto cleanup_socket;
    }
    io_Acceptor_set_fd(&acceptor->base, fd);
    return err;
cleanup_socket:
    close(fd);
    return err;
}

IO_INLINE(void)
io_UnixAcceptor_destroy(io_UnixAcceptor* acceptor)
{
    io_Acceptor_destroy(&acceptor->base);
}

DEFINE_DESCRIPTOR_WRAPPERS(io_UnixAcceptor, io_Acceptor)
DEFINE_ACCEPT_WRAPPERS(io_UnixAcceptor, io_UnixSocket)

#endif
