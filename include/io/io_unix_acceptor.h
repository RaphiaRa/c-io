#ifndef IO_UNIX_ACCEPTOR_H
#define IO_UNIX_ACCEPTOR_H

#include <io/io_config.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <io/io_acceptor.h>
#include <io/io_basic_acceptor.h>
#include <io/io_context.h>
#include <io/io_err.h>
#include <io/io_system_err.h>

typedef struct io_UnixAcceptor {
    io_BasicAcceptor base;
} io_UnixAcceptor;

IO_INLINE(void)
io_UnixAcceptor_init(io_UnixAcceptor* acceptor, io_Context* ctx)
{
    io_BasicAcceptor_init(&acceptor->base, ctx);
}

IO_INLINE(io_Err)
io_UnixAcceptor_open(io_UnixAcceptor* acceptor, const char* path)
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
    io_BasicAcceptor_set_fd(&acceptor->base, fd);
    return err;
cleanup_socket:
    close(fd);
    return err;
}

#endif
