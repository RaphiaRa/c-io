/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_TCP_SOCKET_H
#define IO_TCP_SOCKET_H

#include <io/config.h>

#include <io/gai_err.h>
#include <io/socket.h>
#include <io/system_call.h>

#if IO_OS_POSIX
#include <netdb.h>
#include <netinet/in.h>
#elif IO_OS_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

typedef struct io_TcpSocket {
    io_Socket base;
} io_TcpSocket;

DEFINE_DESCRIPTOR_WRAPPERS(io_TcpSocket, io_Socket)

IO_INLINE(io_Err)
io_TcpSocket_connect(io_TcpSocket* socket, const char* path, uint16_t port);

IO_INLINE(io_Err)
io_TcpSocket_init(io_TcpSocket* socket, io_Context* ctx, const char* path, uint16_t port)
{
    io_Socket_init(&socket->base, ctx);
    if (path) {
        return io_TcpSocket_connect(socket, path, port);
    }
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_TcpSocket_connect(io_TcpSocket* socket, const char* path, uint16_t port)
{
    struct addrinfo hints = {0};
    struct addrinfo *servinfo, *p;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    int ret = 0;
    if ((ret = getaddrinfo(path, port_str, &hints, &servinfo)) != 0) {
        return io_GaiErr(ret);
    }

    int fd = 0;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((fd = io_socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        if (io_connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            io_close(fd);
            continue;
        }
        break;
    }
    if (p == NULL) {
        return io_OtherErr(IO_OTHER_ERRC_NO_ENDPOINT);
    }
    freeaddrinfo(servinfo);
    io_Socket_set_fd(&socket->base, fd);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_TcpSocket_deinit(io_TcpSocket* socket)
{
    io_Socket_deinit(&socket->base);
}

DEFINE_SOCKET_WRAPPERS(io_TcpSocket, io_Socket)

#endif
