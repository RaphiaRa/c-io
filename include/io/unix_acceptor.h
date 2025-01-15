/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_UNIX_ACCEPTOR_H
#define IO_UNIX_ACCEPTOR_H

#include <io/config.h>

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <io/acceptor.h>
#include <io/context.h>
#include <io/err.h>
#include <io/system_err.h>
#include <io/unix_socket.h>

typedef struct io_UnixAcceptor {
    io_Acceptor base;
} io_UnixAcceptor;

DEFINE_DESCRIPTOR_WRAPPERS(io_UnixAcceptor, io_Acceptor)
DEFINE_ACCEPT_WRAPPERS(io_UnixAcceptor, io_UnixSocket)

IO_INLINE(void)
io_UnixAcceptor_init(io_UnixAcceptor* acceptor, io_Context* ctx)
{
    io_Acceptor_init(&acceptor->base, ctx);
}

IO_INLINE(io_UnixAcceptor)
io_UnixAcceptor_make(io_Context* ctx)
{
    io_UnixAcceptor acceptor = {0};
    io_UnixAcceptor_init(&acceptor, ctx);
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
    // Set the socket to non-blocking mode
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK) < 0) {
        err = io_SystemErr_make(errno);
        goto cleanup_socket;
    }
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
io_UnixAcceptor_deinit(io_UnixAcceptor* acceptor)
{
    io_Acceptor_deinit(&acceptor->base);
}

#endif
