/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_UNIX_SOCKET_H
#define IO_UNIX_SOCKET_H

#include <io/socket.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

typedef struct io_UnixSocket {
    io_Socket base;
} io_UnixSocket;

DEFINE_DESCRIPTOR_WRAPPERS(io_UnixSocket, io_Socket)

IO_INLINE(io_Err)
io_UnixSocket_connect(io_UnixSocket* socket, const char* path);

IO_INLINE(io_Err)
io_UnixSocket_init(io_UnixSocket* socket, io_Context* ctx, const char* path)
{
    io_Socket_init(&socket->base, ctx);
    if (path) {
        return io_UnixSocket_connect(socket, path);
    }
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_UnixSocket_connect(io_UnixSocket* socket_, const char* path)
{
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1) {
        return io_SystemErr(errno);
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(fd);
        return io_SystemErr(errno);
    }
    io_Socket_set_fd(&socket_->base, fd);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_UnixSocket_deinit(io_UnixSocket* socket)
{
    io_Socket_deinit(&socket->base);
}

DEFINE_SOCKET_WRAPPERS(io_UnixSocket, io_Socket)

#endif
