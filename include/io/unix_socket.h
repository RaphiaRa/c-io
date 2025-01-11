#ifndef IO_UNIX_SOCKET_H
#define IO_UNIX_SOCKET_H

#include <io/socket.h>

typedef struct io_UnixSocket {
    io_Socket base;
} io_UnixSocket;

IO_INLINE(io_UnixSocket)
io_UnixSocket_make(io_Context* context)
{
    io_UnixSocket unix_socket = {
        .base = io_Socket_make(context),
    };
    return unix_socket;
}

IO_INLINE(void)
io_UnixSocket_deinit(io_UnixSocket* socket)
{
    io_Socket_deinit(&socket->base);
}

DEFINE_SOCKET_WRAPPERS(io_UnixSocket, io_Socket)

#endif
