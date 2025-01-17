/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_UNIX_SOCKET_H
#define IO_UNIX_SOCKET_H

#include <io/socket.h>

typedef struct io_UnixSocket {
    io_Socket base;
} io_UnixSocket;

DEFINE_DESCRIPTOR_WRAPPERS(io_UnixSocket, io_Socket)

IO_INLINE(io_Err)
io_UnixSocket_init(io_UnixSocket* socket, io_Context* ctx)
{
    io_Socket_init(&socket->base, ctx);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_UnixSocket_deinit(io_UnixSocket* socket)
{
    io_Socket_deinit(&socket->base);
}

DEFINE_SOCKET_WRAPPERS(io_UnixSocket, io_Socket)

#endif
