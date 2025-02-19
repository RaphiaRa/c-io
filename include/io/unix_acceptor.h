/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_UNIX_ACCEPTOR_H
#define IO_UNIX_ACCEPTOR_H
#if IO_OS_POSIX

#include <io/config.h>

#include <io/acceptor.h>
#include <io/context.h>
#include <io/err.h>
#include <io/system_call.h>
#include <io/system_err.h>
#include <io/unix_socket.h>

#include <sys/un.h>

typedef struct io_UnixAcceptor {
    io_Acceptor base;
} io_UnixAcceptor;

DEFINE_DESCRIPTOR_WRAPPERS(io_UnixAcceptor, io_Acceptor)
DEFINE_ACCEPT_WRAPPERS(io_UnixAcceptor, io_UnixSocket)

IO_INLINE(io_Err)
io_UnixAcceptor_init(io_UnixAcceptor* acceptor, io_Context* ctx, const char* path)
{
    io_Acceptor_init(&acceptor->base, ctx);
    int fd = io_socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return io_SystemErr(errno);
    }
    io_Err err = IO_ERR_OK;
    // Set the socket to non-blocking mode
    if (io_fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        err = io_SystemErr(errno);
        goto cleanup_socket;
    }
    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);
    if (io_bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        err = io_SystemErr(errno);
        goto cleanup_socket;
    }
    if (io_listen(fd, SOMAXCONN) == -1) {
        err = io_SystemErr(errno);
        goto cleanup_socket;
    }
    io_Acceptor_set_fd(&acceptor->base, fd);
    return err;
cleanup_socket:
    io_close(fd);
    return err;
}

IO_INLINE(void)
io_UnixAcceptor_deinit(io_UnixAcceptor* acceptor)
{
    io_Acceptor_deinit(&acceptor->base);
}

#endif
#endif
