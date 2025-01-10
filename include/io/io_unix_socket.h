#ifndef IO_UNIX_SOCKET_H
#define IO_UNIX_SOCKET_H

#include <io/io_socket.h>

typedef struct io_UnixSocket {
    io_Socket base;
} io_UnixSocket;

IO_INLINE(void)
io_UnixSocket_init(io_UnixSocket* socket, io_Context* context)
{
    io_Socket_init(&socket->base, context);
}

DEFINE_SOCKET_WRAPPERS(io_UnixSocket, io_Socket)

#endif
